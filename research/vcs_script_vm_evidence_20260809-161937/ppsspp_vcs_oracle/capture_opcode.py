#!/usr/bin/env python3
"""Capture one naturally-executed VCS PSP opcode through PPSSPP's debugger API.

The tool never patches the game or injects bytecode.  It temporarily adds CPU
breakpoints at the dispatcher, selected handler and handler return site, then
restores the user's original CPU breakpoint state.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List, Mapping, Optional

from capture_thread_diff import memory_diff, thread_diff, thread_snapshot, u16, u32
from oracle import (
    DebuggerClient,
    OracleError,
    discover_load_base,
    hex32,
    load_json,
    parse_int,
)


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = Path(__file__).with_name("breakpoints.json")


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")


def register_value(registers: Mapping[str, Any], *names: str) -> int:
    lowered = {str(key).lower().lstrip("$"): value for key, value in registers.items()}
    for name in names:
        value = lowered.get(name.lower().lstrip("$"))
        if value is not None:
            return parse_int(value)
    raise OracleError(f"debugger register set does not contain any of: {', '.join(names)}")


def signed(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def breakpoint_payload(row: Mapping[str, Any]) -> Dict[str, Any]:
    payload: Dict[str, Any] = {"address": parse_int(row["address"])}
    for key in ("enabled", "log", "condition", "logFormat"):
        if key in row and row[key] is not None:
            payload[key] = row[key]
    return payload


def memory_breakpoint_payload(row: Mapping[str, Any]) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "address": parse_int(row["address"]),
        "size": parse_int(row["size"]),
    }
    for key in ("enabled", "log", "read", "write", "change", "condition", "logFormat"):
        if key in row and row[key] is not None:
            payload[key] = row[key]
    return payload


class CpuBreakpointLease:
    """Use addresses briefly while preserving any breakpoints already there."""

    def __init__(self, client: DebuggerClient) -> None:
        self.client = client
        self.original: Dict[int, List[Dict[str, Any]]] = {}
        self.added: set[int] = set()

    def _list(self) -> List[Dict[str, Any]]:
        result = self.client.call("cpu.breakpoint.list")
        breakpoints = result.get("breakpoints") if isinstance(result, dict) else None
        if not isinstance(breakpoints, list):
            raise OracleError("cpu.breakpoint.list returned an unexpected response")
        return [dict(row) for row in breakpoints if isinstance(row, dict)]

    def add(self, address: int, condition: Optional[str] = None) -> None:
        address &= 0xFFFFFFFF
        if address not in self.original:
            self.original[address] = [
                row for row in self._list() if parse_int(row.get("address", -1)) == address
            ]
        params: Dict[str, Any] = {"address": address, "enabled": True, "log": False}
        if condition is not None:
            params["condition"] = condition
        if self.original[address]:
            # PPSSPP's API replaces an exact-address breakpoint on add.  Update
            # it temporarily instead, then restore all user-visible settings.
            self.client.call("cpu.breakpoint.update", **params)
        else:
            self.client.call("cpu.breakpoint.add", **params)
        self.added.add(address)

    def remove_temporary(self, address: int) -> None:
        address &= 0xFFFFFFFF
        if address not in self.added:
            return
        self.client.call("cpu.breakpoint.remove", address=address)
        self.added.discard(address)

    def restore(self) -> None:
        for address in list(self.added):
            try:
                rows = self.original.get(address, [])
                if rows:
                    self.client.call("cpu.breakpoint.update", **breakpoint_payload(rows[0]))
                else:
                    self.client.call("cpu.breakpoint.remove", address=address)
            except Exception:
                pass
            self.added.discard(address)


class MemoryWriteTracepointLease:
    """Temporary non-pausing PPSSPP write tracepoints for one ScriptThread."""

    FIELD_SIZES = {
        "ip": 4,
        "saved_ip": 4,
        "call_frame_data": 4,
        "wake_time": 4,
        "call_stack_depth": 2,
        "condition_state": 2,
        "condition_result": 1,
        "wait_flag": 1,
        "not_flag": 1,
    }

    def __init__(self, client: DebuggerClient) -> None:
        self.client = client
        self.original: Dict[tuple[int, int], List[Dict[str, Any]]] = {}
        self.added: Dict[tuple[int, int], Dict[str, Any]] = {}
        self.unavailable: List[Dict[str, Any]] = []

    def _list(self) -> List[Dict[str, Any]]:
        result = self.client.call("memory.breakpoint.list")
        breakpoints = result.get("breakpoints") if isinstance(result, dict) else None
        if not isinstance(breakpoints, list):
            raise OracleError("memory.breakpoint.list returned an unexpected response")
        return [dict(row) for row in breakpoints if isinstance(row, dict)]

    def add_field(self, field: str, address: int, size: int) -> None:
        key = (address & 0xFFFFFFFF, size)
        if key in self.added or any(row.get("field") == field for row in self.unavailable):
            return
        if key not in self.original:
            self.original[key] = [
                row
                for row in self._list()
                if parse_int(row.get("address", -1)) == key[0]
                and parse_int(row.get("size", -1)) == key[1]
            ]
        if self.original[key]:
            # Restoring a replaced memory check would lose its historical hit
            # counter, so preserve user tracepoints exactly and record why this
            # oracle field was not instrumented.
            self.unavailable.append(
                {
                    "field": field,
                    "address": hex32(key[0]),
                    "size": size,
                    "reason": "existing_user_memory_breakpoint_preserved",
                }
            )
            return
        self.client.call(
            "memory.breakpoint.add",
            address=key[0],
            size=size,
            enabled=False,
            log=False,
            read=False,
            write=True,
            change=False,
        )
        self.added[key] = {"field": field, "address": key[0], "size": size}

    def arm_thread_fields(
        self, field_offsets: Mapping[str, Any], thread_address: int, fields: Iterable[str]
    ) -> None:
        for field in dict.fromkeys(fields):
            if field not in field_offsets or field not in self.FIELD_SIZES:
                raise OracleError(f"tracepoint field is not configured: {field}")
            self.add_field(
                field,
                (thread_address + parse_int(field_offsets[field])) & 0xFFFFFFFF,
                self.FIELD_SIZES[field],
            )

    def observations(self) -> List[Dict[str, Any]]:
        current = {
            (parse_int(row["address"]), parse_int(row["size"])): row for row in self._list()
        }
        output: List[Dict[str, Any]] = []
        for key, item in self.added.items():
            row = current.get(key, {})
            hits = parse_int(row.get("hits", 0))
            output.append(
                {
                    "field": item["field"],
                    "address": hex32(item["address"]),
                    "size": item["size"],
                    "kind": "PPSSPP memory.breakpoint.add(write=true, enabled=false, log=false)",
                    "write_hits": hits,
                    "write_observed": hits > 0,
                }
            )
        return output + self.unavailable

    def restore(self) -> None:
        for address, size in list(self.added):
            try:
                self.client.call("memory.breakpoint.remove", address=address, size=size)
            except Exception:
                pass
            self.added.pop((address, size), None)


def resume(client: DebuggerClient) -> None:
    # PPSSPP sends cpu.stepping asynchronously after a resume, so there is no
    # ticketed response to wait for here.
    client.send_event("cpu.resume")


def resume_if_stepping(client: DebuggerClient) -> None:
    status = client.call("cpu.status")
    if status.get("stepping"):
        resume(client)


def wait_for_pc(client: DebuggerClient, target_pc: int, timeout: float) -> Dict[str, Any]:
    deadline = time.monotonic() + timeout
    target_pc &= 0xFFFFFFFF
    while time.monotonic() < deadline:
        event = client.next_event(max(0.05, min(1.0, deadline - time.monotonic())))
        if event is None:
            continue
        if event.get("event") != "cpu.stepping":
            continue
        registers = client.registers()
        pc = register_value(registers, "pc", "r32")
        if pc == target_pc:
            return registers
        resume(client)
    raise OracleError(f"timed out waiting for PC {hex32(target_pc)}")


def case_for_opcode(config: Mapping[str, Any], opcode: int) -> Dict[str, Any]:
    for case in config.get("proof_cases", []):
        if parse_int(case["opcode"]) == opcode:
            return dict(case)
    known = ", ".join(str(case.get("opcode")) for case in config.get("proof_cases", []))
    raise OracleError(f"opcode 0x{opcode:04X} is not a configured proof case ({known})")


def read_watch_regions(
    client: DebuggerClient, regions: Iterable[Mapping[str, Any]], thread_address: int
) -> Dict[str, Dict[str, Any]]:
    result: Dict[str, Dict[str, Any]] = {}
    for index, region in enumerate(regions):
        name = str(region.get("name", f"region_{index}"))
        address = parse_int(region["address"])
        if str(region.get("address_base", "absolute")) == "thread":
            address += thread_address
        size = parse_int(region["size"])
        result[name] = {"address": hex32(address), "bytes": client.read(address, size)}
    return result


def watch_diffs(before: Mapping[str, Any], after: Mapping[str, Any]) -> List[Dict[str, Any]]:
    output: List[Dict[str, Any]] = []
    for name in before:
        first, second = before[name], after[name]
        output.append(
            {
                "name": name,
                "address": first["address"],
                "changes": memory_diff(
                    parse_int(first["address"]), first["bytes"], second["bytes"]
                ),
            }
        )
    return output


def capture_once(
    client: DebuggerClient,
    config: Mapping[str, Any],
    opcode: int,
    load_base: int,
    timeout: float,
    max_dispatches: int,
) -> Dict[str, Any]:
    dispatcher = config["dispatcher"]
    capture = config["capture"]
    case = case_for_opcode(config, opcode)
    dispatcher_pc = load_base + parse_int(dispatcher["rva"])
    opcode_ready_pc = dispatcher_pc + parse_int(dispatcher["opcode_ready_rva_offset"])
    opcode_register = str(dispatcher["opcode_register"])
    opcode_condition = f"({opcode_register} & 0x7fff) == 0x{opcode:04x}"
    table = load_base + parse_int(dispatcher["handler_table_rva"])
    table_stride = parse_int(dispatcher["handler_record_size"])
    snapshot_size = parse_int(config["thread"]["snapshot_size"])
    opcode_window = parse_int(capture["opcode_window_bytes"])
    gp_offset = parse_int(dispatcher["script_space_pointer_gp_offset"])
    leases = CpuBreakpointLease(client)
    tracepoints = MemoryWriteTracepointLease(client)

    try:
        # At this instruction the dispatcher has fetched the 16-bit opcode
        # into a1, but has not yet advanced ScriptThread::IP.  The PPSSPP
        # expression condition therefore filters in the emulator and avoids
        # pausing once for every unrelated script command.
        leases.add(opcode_ready_pc, condition=opcode_condition)
        # A normal game is already executing.  Sending cpu.resume in that
        # state causes PPSSPP to emit an avoidable asynchronous error.
        resume_if_stepping(client)

        for dispatch_count in range(1, 2):
            entry_regs = wait_for_pc(client, opcode_ready_pc, timeout)
            thread_address = register_value(entry_regs, "a0", "r4")
            gp = register_value(entry_regs, "gp", "r28")
            before_raw = client.read(thread_address, snapshot_size)
            ip_before = u32(before_raw, 0x10)
            script_base = client.read_u32((gp + gp_offset) & 0xFFFFFFFF)
            byte_window = client.read((script_base + ip_before) & 0xFFFFFFFF, opcode_window)
            raw_opcode = u16(byte_window, 0)
            normalized_opcode = raw_opcode & 0x7FFF

            if normalized_opcode != opcode:
                raise OracleError(
                    f"filtered breakpoint delivered opcode 0x{normalized_opcode:04X}, "
                    f"expected 0x{opcode:04X}"
                )

            dispatch_record = client.read(table + opcode * table_stride, table_stride)
            descriptor = u32(dispatch_record, 0)
            handler = u32(dispatch_record, 4)
            expected_handler = load_base + parse_int(case["static_handler_rva"])
            if handler != expected_handler:
                raise OracleError(
                    f"dispatcher table handler {hex32(handler)} does not match configured "
                    f"static target {hex32(expected_handler)}"
                )

            before_watches = read_watch_regions(
                client, capture.get("watch_regions", []), thread_address
            )
            if capture.get("memory_write_tracepoints", False):
                tracepoints.arm_thread_fields(
                    config["thread"]["known_fields"],
                    thread_address,
                    ["ip", "not_flag"] + list(case.get("static_expected_thread_fields", [])),
                )
            leases.add(handler)
            resume(client)

            handler_regs = wait_for_pc(client, handler, timeout)
            after_dispatch_raw = client.read(thread_address, snapshot_size)
            return_pc = register_value(handler_regs, "ra", "r31")
            if return_pc in (0, handler):
                raise OracleError(f"invalid handler return address {hex32(return_pc)}")
            leases.add(return_pc)
            resume(client)

            return_regs = wait_for_pc(client, return_pc, timeout)
            after_raw = client.read(thread_address, snapshot_size)
            after_watches = read_watch_regions(
                client, capture.get("watch_regions", []), thread_address
            )
            dispatch_status = register_value(return_regs, "v0", "r2")
            write_tracepoints = tracepoints.observations() if capture.get(
                "memory_write_tracepoints", False
            ) else []

            record = {
                "schema": "ppsspp-vcs-oracle/capture-v2",
                "captured_at": utc_now(),
                "source": {
                    "transport": "PPSSPP debugger WebSocket",
                    "execution": "natural game execution; no game memory patching or script injection",
                },
                "proof_case": {
                    "name": case["name"],
                    "opcode": f"0x{opcode:04X}",
                    "expected_vm_dispatch_status": parse_int(case["expected_vm_dispatch_status"]),
                    "expected_script_flow": case["script_flow"],
                    "script_contract": case.get("script_contract", {}),
                    "static_expected_thread_fields": case.get("static_expected_thread_fields", []),
                },
                "runtime": {
                    "load_base": hex32(load_base),
                    "dispatcher": hex32(dispatcher_pc),
                    "opcode_ready_breakpoint": {
                        "address": hex32(opcode_ready_pc),
                        "condition": opcode_condition,
                        "opcode_register": opcode_register,
                    },
                    "script_thread": hex32(thread_address),
                    "script_bytecode_base": hex32(script_base),
                    "ip_before": hex32(ip_before),
                    "ip_after": hex32(u32(after_raw, 0x10)),
                    "opcode_bytes": byte_window[:2].hex(),
                    "raw_opcode": f"0x{raw_opcode:04X}",
                    "opcode": f"0x{normalized_opcode:04X}",
                    "not_bit_set": bool(raw_opcode & 0x8000),
                    "input_parameter_bytes": byte_window[2:].hex(),
                    "dispatch_record": {
                        "address": hex32(table + opcode * table_stride),
                        "descriptor": hex32(descriptor),
                        "handler": hex32(handler),
                        "handler_rva": hex32(handler - load_base),
                    },
                    "handler_return_site": hex32(return_pc),
                    "native_handler_status": {
                        "register": "v0",
                        "value": dispatch_status,
                        "process_action": "CONTINUE" if dispatch_status == 0 else "STOP_CURRENT_PROCESS_PASS",
                        "meaning": "Interpreter ABI status; this is not a script-command output.",
                    },
                    "dispatcher_to_handler_thread_diff": thread_diff(
                        thread_address, before_raw, after_dispatch_raw
                    ),
                    "script_thread_diff": thread_diff(thread_address, before_raw, after_raw),
                    "script_thread_write_tracepoints": write_tracepoints,
                    "relevant_memory_changes": watch_diffs(before_watches, after_watches),
                    "dispatches_skipped_before_hit": dispatch_count - 1,
                },
                "assertions": {
                    "opcode_match": normalized_opcode == opcode,
                    "handler_match": handler == expected_handler,
                    "vm_dispatch_status_match": dispatch_status == parse_int(case["expected_vm_dispatch_status"]),
                    "dispatcher_ip_write_observed": any(
                        item.get("field") == "ip" and item.get("write_observed")
                        for item in write_tracepoints
                    ) if write_tracepoints else True,
                },
            }
            return record

        raise OracleError(f"opcode 0x{opcode:04X} did not reach its filtered breakpoint")
    finally:
        tracepoints.restore()
        leases.restore()
        if capture.get("resume_after_capture", True):
            try:
                resume(client)
            except Exception:
                pass


def append_jsonl(path: Path, record: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8", newline="\n") as handle:
        handle.write(json.dumps(record, ensure_ascii=False, sort_keys=True) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("opcode", help="Configured normalized opcode, for example 0x0001")
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    parser.add_argument("--endpoint", default="ws://127.0.0.1:16482/debugger")
    parser.add_argument("--load-base", help="Runtime ELF load base; omit to discover by signature")
    parser.add_argument("--timeout", type=float, default=15.0, help="Seconds per breakpoint wait")
    parser.add_argument("--max-dispatches", type=int, help="Override configuration dispatch scan cap")
    parser.add_argument("--telemetry", type=Path, help="Append JSONL here")
    args = parser.parse_args()

    opcode = parse_int(args.opcode)
    config = load_json(args.config)
    case = case_for_opcode(config, opcode)
    max_dispatches = args.max_dispatches or parse_int(config["capture"]["max_dispatches"])
    default_telemetry = Path(__file__).with_name("telemetry") / f"runtime_{opcode:04X}.jsonl"
    telemetry = args.telemetry or default_telemetry

    try:
        with DebuggerClient(args.endpoint) as client:
            version = client.call("version")
            if not isinstance(version, Mapping):
                raise OracleError("PPSSPP did not return its debugger version")
            status = client.call("game.status")
            if not isinstance(status, Mapping) or not status.get("game"):
                raise OracleError("no running game is exposed by PPSSPP's debugger")
            if args.load_base:
                load_base = parse_int(args.load_base)
            else:
                elf = (args.config.parent / str(config["elf"])).resolve()
                load_base = discover_load_base(
                    client, elf, parse_int(config["dispatcher"]["rva"])
                )
            record = capture_once(
                client, config, opcode, load_base, args.timeout, max_dispatches
            )
            record["ppsspp"] = {"version": version, "game": status}
            append_jsonl(telemetry, record)
    except (OSError, OracleError, ValueError, KeyError, struct.error) as exc:
        print(f"capture failed: {exc}", file=sys.stderr)
        return 2

    verdict = "PASS" if all(record["assertions"].values()) else "FAIL"
    print(
        f"{verdict}: {case['name']} {record['runtime']['opcode']} -> "
        f"{record['runtime']['dispatch_record']['handler']} vm_status="
        f"{record['runtime']['native_handler_status']['value']} "
        f"({record['runtime']['native_handler_status']['process_action']})\ntelemetry: {telemetry}"
    )
    return 0 if verdict == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
