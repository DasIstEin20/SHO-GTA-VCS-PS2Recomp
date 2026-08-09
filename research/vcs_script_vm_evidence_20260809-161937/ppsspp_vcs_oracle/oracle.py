"""Stock-PPSSPP WebSocket client and PSP-runtime-to-PS2-static evidence bridge.

It implements PPSSPP's documented debugger WebSocket framing directly with the
Python standard library, so the oracle has no package dependency and does not
patch the emulator.  It deliberately records observations; it does not invent
source-level command names.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import os
import socket
import struct
import sys
import time
from collections import defaultdict
from pathlib import Path
from typing import Any, Callable
from urllib.parse import urlparse


class OracleError(RuntimeError):
    pass


INITIAL_PROOF_OPCODES = ("0x0000", "0x0001", "0x0002", "0x037A", "0x037B", "0x055A")
CAPTURE_SCHEMAS = {"ppsspp-vcs-oracle/capture-v1", "ppsspp-vcs-oracle/capture-v2"}


def parse_int(value: str | int) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


def hex32(value: int) -> str:
    return f"0x{value & 0xFFFFFFFF:08X}"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def read_jsonl(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    return [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]


class DebuggerClient:
    """Minimal ticketed PPSSPP `debugger.ppsspp.org` WebSocket client."""

    def __init__(self, endpoint: str, timeout: float = 10.0):
        parsed = urlparse(endpoint)
        if parsed.scheme != "ws" or not parsed.hostname or not parsed.port:
            raise OracleError("Endpoint must be ws://host:port/debugger")
        self.endpoint, self.host, self.port, self.path, self.timeout = endpoint, parsed.hostname, parsed.port, parsed.path or "/debugger", timeout
        self.sock: socket.socket | None = None
        self.ticket = 0
        self.events: list[dict[str, Any]] = []

    def connect(self) -> None:
        self.sock = socket.create_connection((self.host, self.port), timeout=self.timeout)
        self.sock.settimeout(self.timeout)
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        request = (
            f"GET {self.path} HTTP/1.1\r\nHost: {self.host}:{self.port}\r\n"
            "Upgrade: websocket\r\nConnection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n"
            "Sec-WebSocket-Protocol: debugger.ppsspp.org\r\n\r\n"
        ).encode("ascii")
        self.sock.sendall(request)
        response = self._read_http_response()
        if b" 101 " not in response.split(b"\r\n", 1)[0]:
            raise OracleError(f"PPSSPP did not upgrade debugger WebSocket: {response[:300]!r}")

    def close(self) -> None:
        if self.sock:
            try:
                self._send_frame(0x8, b"")
            except OSError:
                pass
            self.sock.close()
            self.sock = None

    def __enter__(self) -> "DebuggerClient":
        self.connect()
        return self

    def __exit__(self, *_: object) -> None:
        self.close()

    def _read_exact(self, count: int) -> bytes:
        if not self.sock:
            raise OracleError("WebSocket is not connected")
        parts: list[bytes] = []
        while count:
            part = self.sock.recv(count)
            if not part:
                raise OracleError("PPSSPP debugger WebSocket closed")
            parts.append(part)
            count -= len(part)
        return b"".join(parts)

    def _read_http_response(self) -> bytes:
        if not self.sock:
            raise OracleError("WebSocket is not connected")
        data = bytearray()
        while b"\r\n\r\n" not in data:
            part = self.sock.recv(4096)
            if not part:
                raise OracleError("PPSSPP closed during WebSocket handshake")
            data.extend(part)
            if len(data) > 64 * 1024:
                raise OracleError("Oversized WebSocket handshake")
        return bytes(data)

    def _send_frame(self, opcode: int, payload: bytes) -> None:
        if not self.sock:
            raise OracleError("WebSocket is not connected")
        mask = os.urandom(4)
        length = len(payload)
        first = bytes([0x80 | opcode])
        if length < 126:
            header = first + bytes([0x80 | length])
        elif length <= 0xFFFF:
            header = first + bytes([0x80 | 126]) + struct.pack("!H", length)
        else:
            header = first + bytes([0x80 | 127]) + struct.pack("!Q", length)
        masked = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        self.sock.sendall(header + mask + masked)

    def _recv_frame(self) -> tuple[bool, int, bytes]:
        first, second = self._read_exact(2)
        fin, opcode, masked, length = bool(first & 0x80), first & 0x0F, bool(second & 0x80), second & 0x7F
        if length == 126:
            length = struct.unpack("!H", self._read_exact(2))[0]
        elif length == 127:
            length = struct.unpack("!Q", self._read_exact(8))[0]
        mask = self._read_exact(4) if masked else b""
        payload = self._read_exact(length)
        if masked:
            payload = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        return fin, opcode, payload

    def recv_event(self, timeout: float | None = None) -> dict[str, Any]:
        if not self.sock:
            raise OracleError("WebSocket is not connected")
        old_timeout = self.sock.gettimeout()
        if timeout is not None:
            self.sock.settimeout(timeout)
        try:
            chunks: list[bytes] = []
            text_opcode: int | None = None
            while True:
                fin, opcode, payload = self._recv_frame()
                if opcode == 0x9:  # ping
                    self._send_frame(0xA, payload)
                    continue
                if opcode == 0x8:
                    raise OracleError("PPSSPP debugger WebSocket closed")
                if opcode in (0x1, 0x0):
                    if opcode == 0x1:
                        text_opcode = opcode
                    chunks.append(payload)
                    if fin:
                        if text_opcode != 0x1:
                            raise OracleError("Unexpected binary debugger response")
                        return json.loads(b"".join(chunks).decode("utf-8"))
        except socket.timeout as error:
            raise TimeoutError("Timed out waiting for PPSSPP debugger event") from error
        finally:
            self.sock.settimeout(old_timeout)

    def send_event(self, event: str, **params: Any) -> None:
        payload = {"event": event, **params}
        self._send_frame(0x1, json.dumps(payload, separators=(",", ":")).encode("utf-8"))

    def call(self, event: str, timeout: float | None = None, **params: Any) -> dict[str, Any]:
        self.ticket += 1
        ticket = f"oracle-{self.ticket}"
        self.send_event(event, ticket=ticket, **params)
        deadline = time.monotonic() + (timeout if timeout is not None else self.timeout)
        while True:
            message = self.recv_event(max(0.05, deadline - time.monotonic()))
            if message.get("ticket") == ticket:
                if message.get("event") == "error":
                    raise OracleError(f"PPSSPP {event} failed: {message.get('message', message)}")
                if message.get("event") != event:
                    raise OracleError(f"PPSSPP response event mismatch: requested {event}, got {message.get('event')}")
                return message
            self.events.append(message)

    def wait_event(self, predicate: Callable[[dict[str, Any]], bool], timeout: float) -> dict[str, Any]:
        deadline = time.monotonic() + timeout
        while True:
            for index, message in enumerate(self.events):
                if predicate(message):
                    return self.events.pop(index)
            message = self.recv_event(max(0.05, deadline - time.monotonic()))
            if predicate(message):
                return message
            self.events.append(message)

    def next_event(self, timeout: float) -> dict[str, Any] | None:
        """Return the next queued or incoming asynchronous debugger event."""
        if self.events:
            return self.events.pop(0)
        try:
            return self.recv_event(timeout)
        except TimeoutError:
            return None

    def version(self) -> dict[str, Any]:
        return self.call("version", name="vcs-ppsspp-oracle", version="1.0")

    def registers(self) -> dict[str, int]:
        payload = self.call("cpu.getAllRegs")
        result: dict[str, int] = {}
        for category in payload.get("categories", []):
            for name, value in zip(category.get("registerNames", []), category.get("uintValues", [])):
                result[str(name).lower()] = int(value)
        return result

    def read(self, address: int, size: int) -> bytes:
        payload = self.call("memory.read", address=address & 0xFFFFFFFF, size=size, replacements=False, timeout=max(self.timeout, 30.0))
        return base64.b64decode(payload["base64"])

    def read_u32(self, address: int) -> int:
        return int.from_bytes(self.read(address, 4), "little")


def elf_bytes_at_rva(path: Path, rva: int, size: int) -> bytes:
    data = path.read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        raise OracleError(f"Expected a 32-bit little-endian ELF: {path}")
    _, _, _, _, _, phoff, _, _, _, phentsize, phnum, _, _, _ = struct.unpack_from("<16sHHIIIIIHHHHHH", data, 0)
    for index in range(phnum):
        p_type, p_offset, p_vaddr, _, p_filesz, _, _, _ = struct.unpack_from("<IIIIIIII", data, phoff + index * phentsize)
        if p_type == 1 and p_vaddr <= rva and rva + size <= p_vaddr + p_filesz:
            start = p_offset + rva - p_vaddr
            return data[start : start + size]
    raise OracleError(f"RVA {hex(rva)} does not map to an ELF load segment")


def discover_load_base(client: DebuggerClient, static_elf: Path, dispatcher_rva: int, user_start: int = 0x08800000, user_end: int = 0x0A000000) -> int:
    """Find the relocated VCS image by scanning for the dispatcher instruction prefix."""
    signature = elf_bytes_at_rva(static_elf, dispatcher_rva, 16)
    chunk = 0x40000
    matches: list[int] = []
    for address in range(user_start, user_end, chunk):
        data = client.read(address, min(chunk, user_end - address))
        offset = data.find(signature)
        if offset >= 0:
            candidate = address + offset - dispatcher_rva
            if candidate not in matches:
                matches.append(candidate)
    if len(matches) != 1:
        rendered = ", ".join(hex32(value) for value in matches) or "none"
        raise OracleError(f"Unable to uniquely locate VCS dispatcher signature in PSP user RAM; matches: {rendered}")
    return matches[0]


def collect_runtime_records(telemetry: Path) -> dict[str, dict[str, Any]]:
    values: dict[str, dict[str, Any]] = {}
    for path in sorted(telemetry.glob("*.jsonl")):
        for row in read_jsonl(path):
            if row.get("schema") not in CAPTURE_SCHEMAS:
                continue
            assertions = row.get("assertions", {})
            runtime = row.get("runtime", {})
            if runtime.get("opcode") and assertions and all(assertions.values()):
                values[str(runtime["opcode"])] = row
    return values


def proof_cases(config: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        f"0x{parse_int(row['opcode']):04X}": row
        for row in config.get("proof_cases", [])
    }


def vm_dispatch_interpretation(observed: dict[str, Any] | None, case: dict[str, Any] | None) -> dict[str, Any] | None:
    """Normalize v1 historic records and v2 status-first captures.

    A handler's v0 here belongs to the interpreter ABI.  It must never be
    presented as an SCM command output.
    """
    if not observed:
        return None
    runtime = observed.get("runtime", {})
    raw = runtime.get("native_handler_status")
    if isinstance(raw, dict) and "value" in raw:
        status = parse_int(raw["value"])
    elif "return_value_v0" in runtime:  # capture-v1 provenance
        status = parse_int(runtime["return_value_v0"])
    else:
        return None
    process_action = "CONTINUE" if status == 0 else "STOP_CURRENT_PROCESS_PASS"
    return {
        "register": "v0",
        "value": status,
        "process_action": process_action,
        "script_flow": case.get("script_flow") if case else None,
        "definition": "Interpreter ABI status, not a script-command output.",
        "source_capture_schema": observed.get("schema"),
    }


def compare_static(args: argparse.Namespace) -> int:
    runtime = collect_runtime_records(args.telemetry)
    config = load_json(args.config)
    cases = proof_cases(config)
    proof = load_json(args.manual_proof)
    ps2 = load_json(args.ps2_table)
    ps2_slots = {int(row["opcode"]): row for row in ps2["slots"] if row.get("present")}
    rows: list[dict[str, Any]] = []
    for item in proof.get("slots", []):
        opcode = parse_int(item["opcode"])
        key = f"0x{opcode:04X}"
        observed = runtime.get(key)
        case = cases.get(key)
        ps2_slot = ps2_slots.get(opcode)
        rows.append({
            "opcode": key,
            "name": item.get("name", "COMMAND_" + key[2:]),
            "psp_runtime": observed,
            "psp_vm_dispatch_status": vm_dispatch_interpretation(observed, case),
            "ps2_static_handler": ps2_slot.get("target") if ps2_slot else None,
            "ps2_static_manual_finding": item.get("finding"),
            "status": "RUNTIME_CAPTURED_AND_STATIC_CONTEXT_PRESENT" if observed and ps2_slot else ("AWAITING_RUNTIME_CAPTURE" if ps2_slot else "PS2_STATIC_SLOT_ABSENT"),
        })
    output = {
        "schema": 2,
        "purpose": "PSP runtime oracle observations paired with existing VCS PS2 static evidence; native handler status is interpreter control flow, not an SCM output; no automatic equivalence assertion.",
        "runtime_capture_summary": {
            "initial_proof_opcodes": list(INITIAL_PROOF_OPCODES),
            "captured_opcodes": sorted(runtime, key=parse_int),
            "awaiting_natural_execution": [
                opcode for opcode in INITIAL_PROOF_OPCODES if opcode not in runtime
            ],
            "successful_capture_count": len(runtime),
            "status": "COMPLETE" if all(opcode in runtime for opcode in INITIAL_PROOF_OPCODES) else "PARTIAL",
        },
        "runtime_telemetry": str(args.telemetry),
        "ps2_table": str(args.ps2_table),
        "manual_proof": str(args.manual_proof),
        "canonical_semantics": str(args.canonical_semantics),
        "rows": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(output, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote PSP-runtime-to-PS2-static bridge to {args.output}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    status = sub.add_parser("status", help="verify stock PPSSPP debugger connectivity")
    status.add_argument("--endpoint", default="ws://127.0.0.1:16482/debugger")
    bridge = sub.add_parser("compare-static", help="pair captured PSP runtime facts with PS2 static evidence")
    bridge.add_argument("--telemetry", type=Path, default=Path(__file__).parent / "telemetry")
    bridge.add_argument("--ps2-table", type=Path, default=Path(__file__).parents[2] / "VCS_PS2_HANDLER_DB.json")
    bridge.add_argument("--manual-proof", type=Path, default=Path(__file__).parents[2] / "VCS_PSP_PS2_MANUAL_SEMANTIC_PROOF.json")
    bridge.add_argument("--config", type=Path, default=Path(__file__).with_name("breakpoints.json"))
    bridge.add_argument("--canonical-semantics", type=Path, default=Path(__file__).parents[2] / "VCS_PSP_INITIAL_OPCODE_CANONICAL_SEMANTICS.json")
    bridge.add_argument("--output", type=Path, default=Path(__file__).parent / "telemetry" / "psp_runtime_vs_ps2_static.json")
    args = parser.parse_args()
    if args.command == "status":
        try:
            with DebuggerClient(args.endpoint) as client:
                print(json.dumps({"version": client.version(), "game": client.call("game.status"), "memory": client.call("memory.mapping")}, indent=2))
            return 0
        except (OSError, OracleError, ValueError) as exc:
            print(f"PPSSPP debugger status unavailable: {exc}", file=sys.stderr)
            return 2
    return compare_static(args)


if __name__ == "__main__":
    raise SystemExit(main())
