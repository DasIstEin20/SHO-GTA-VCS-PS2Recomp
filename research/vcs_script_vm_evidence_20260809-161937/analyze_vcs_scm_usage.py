"""Produce evidence-bounded opcode usage from a VCS MAIN.SCM main segment.

This is intentionally a *linear* bytecode inventory, not an execution trace or
a claim that every discovered command is reachable. It knows only the VCS PSP
parameter widths directly recovered from the VM plus the separately proved
CALL/CALL_NOT layout. It stops at the first undecodable command instead of
trying to resynchronise through data as if it were code.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any


CALL_OPCODES = {0x037A, 0x037B}
HEADER_BYTES = 8
# The public VCS library expresses START_NEW_SCRIPT as `label, arguments` and
# therefore reports two syntactic parameters. Direct PSP 002B7D84 and PS2
# 003B2098 evidence proves the native VM decodes one script index followed by
# 0x60 local-value encodings for the newly created thread.
RECOVERED_VM_PARAMETER_COUNTS = {0x0024: 0x61}
MISSION_STREAM_OPCODE = 0x0289


def u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "little")


def u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 4], "little")


def library_commands(path: Path) -> dict[int, dict[str, Any]]:
    raw = json.loads(path.read_text(encoding="utf-8"))
    default = next(item for item in raw["extensions"] if item["name"] == "default")
    return {int(item["id"], 16): item for item in default["commands"]}


def parameter_end(data: bytes, offset: int, end: int) -> int:
    """Return cursor after one confirmed SCM parameter encoding.

    Tag `0x0A` is the null-terminated ScriptSpace string form visible in the
    plural PSP collector. All other widths follow the recovered decoder and
    variable resolver. This scanner only needs the byte extent, not a value.
    """
    if offset >= end:
        raise ValueError("missing parameter tag")
    tag = data[offset]
    if tag in (0x00, 0x01, 0x02):
        return offset + 1
    if tag in (0x03, 0x07):
        return offset + 2
    if tag in (0x04, 0x08):
        return offset + 3
    if tag == 0x05:
        return offset + 4
    if tag in (0x06, 0x09):
        return offset + 5
    if tag == 0x0A:
        cursor = offset + 1
        while cursor < end and data[cursor] != 0:
            cursor += 1
        if cursor >= end:
            raise ValueError("unterminated tag-0x0A string")
        return cursor + 1
    if 0x0B <= tag <= 0x6C:
        return offset + 1
    if 0x6D <= tag <= 0xCC:
        return offset + 3
    if 0xCD <= tag <= 0xE5:
        return offset + 2
    return offset + 4  # 0xE6..0xFF global array


def decode_call(data: bytes, offset: int, end: int) -> tuple[int, list[int]]:
    if offset + 3 > end:
        raise ValueError("truncated CALL header")
    inputs, outputs, _locals_offset = data[offset:offset + 3]
    cursor = parameter_end(data, offset + 3, end)  # target
    tags = [data[offset + 3]]
    for _ in range(inputs + outputs):
        if cursor >= end:
            raise ValueError("truncated CALL argument/destination")
        tags.append(data[cursor])
        cursor = parameter_end(data, cursor, end)
    return cursor, tags


def decode_mission_selector_literal(data: bytes, offset: int, end: int) -> dict[str, Any]:
    """Describe only a directly encoded 0289 selector literal.

    Opcode 0289's one collected raw parameter is VM-proven. This static
    scanner must not resolve locals/globals or promote other parameter forms
    to a concrete selector, so variable and float-like encodings remain
    explicitly unresolved.
    """
    if offset >= end:
        raise ValueError("truncated 0289 selector")
    tag = data[offset]
    row: dict[str, Any] = {"parameter_tag": f"0x{tag:02X}", "literal": None}
    if tag in (0x00, 0x01):
        row["literal"] = 0
    elif tag == 0x07 and offset + 2 <= end:
        row["literal"] = int.from_bytes(data[offset + 1:offset + 2], "little", signed=True)
    elif tag == 0x08 and offset + 3 <= end:
        row["literal"] = int.from_bytes(data[offset + 1:offset + 3], "little", signed=True)
    elif tag == 0x06 and offset + 5 <= end:
        row["literal"] = int.from_bytes(data[offset + 1:offset + 5], "little", signed=True)
    row["classification"] = "direct_integer_literal" if row["literal"] is not None else "nonliteral_or_unresolved"
    return row


def scan(data: bytes, start: int, end: int, commands: dict[int, dict[str, Any]]) -> dict[str, Any]:
    cursor = start
    uses: Counter[int] = Counter()
    tag_uses: Counter[int] = Counter()
    failures: list[dict[str, Any]] = []
    mission_selector_sites: list[dict[str, Any]] = []
    calls = 0
    while cursor < end:
        instruction = cursor
        if cursor + 2 > end:
            failures.append({"offset": f"0x{instruction:06X}", "reason": "truncated opcode"})
            break
        raw_opcode = u16(data, cursor)
        opcode = raw_opcode & 0x7FFF
        cursor += 2
        command = commands.get(opcode)
        if command is None:
            failures.append({
                "offset": f"0x{instruction:06X}",
                "raw_opcode": f"0x{raw_opcode:04X}",
                "normalized_opcode": f"0x{opcode:04X}",
                "reason": "opcode is absent from the VCS public command library",
            })
            break
        try:
            if opcode == MISSION_STREAM_OPCODE:
                selector = decode_mission_selector_literal(data, cursor, end)
                selector.update({
                    "file_offset": f"0x{instruction:06X}",
                    "raw_opcode": f"0x{raw_opcode:04X}",
                })
                mission_selector_sites.append(selector)
            if opcode in CALL_OPCODES:
                cursor, tags = decode_call(data, cursor, end)
                calls += 1
                tag_uses.update(tags)
            else:
                count = RECOVERED_VM_PARAMETER_COUNTS.get(opcode, int(command.get("num_params", 0)))
                for _ in range(count):
                    if cursor >= end:
                        raise ValueError("truncated parameter")
                    tag_uses[data[cursor]] += 1
                    cursor = parameter_end(data, cursor, end)
            if cursor > end:
                raise ValueError("parameter extends beyond main segment")
        except ValueError as exc:
            failures.append({
                "offset": f"0x{instruction:06X}",
                "raw_opcode": f"0x{raw_opcode:04X}",
                "normalized_opcode": f"0x{opcode:04X}",
                "name": command.get("name", "COMMAND_UNKNOWN"),
                "reason": str(exc),
            })
            break
        uses[opcode] += 1

    rows = []
    for opcode, count in uses.most_common():
        command = commands[opcode]
        rows.append({
            "opcode": f"0x{opcode:04X}",
            "name": command.get("name", "COMMAND_UNKNOWN"),
            "count": count,
            "public_num_params": command.get("num_params", 0),
            "recovered_vm_parameter_count": RECOVERED_VM_PARAMETER_COUNTS.get(opcode),
            "is_condition": bool(command.get("attrs", {}).get("is_condition", False)),
        })
    return {
        "decoded_instruction_count": sum(uses.values()),
        "cursor_after_last_decoded_instruction": f"0x{cursor:06X}",
        "fully_decoded": not failures and cursor == end,
        "failures": failures,
        "call_or_call_not_count": calls,
        "mission_stream_selector_sites": mission_selector_sites,
        "parameter_tag_counts": [{"tag": f"0x{tag:02X}", "count": count} for tag, count in tag_uses.most_common()],
        "opcodes_by_frequency": rows,
    }


def markdown(payload: dict[str, Any]) -> str:
    scan_info = payload["scan"]
    lines = [
        "# VCS PSP MAIN.SCM opcode usage",
        "",
        "This is a static, linear inventory of the header-delimited main segment. It is not runtime hotness or a reachability claim.",
        "",
        f"- Decoded commands: `{scan_info['decoded_instruction_count']}`",
        f"- Fully decoded main segment: `{scan_info['fully_decoded']}`",
        f"- CALL/CALL_NOT sites: `{scan_info['call_or_call_not_count']}`",
        f"- Static `0289` sites: `{len(scan_info['mission_stream_selector_sites'])}`",
        f"- Final cursor: `{scan_info['cursor_after_last_decoded_instruction']}`",
        "",
    ]
    if scan_info["failures"]:
        lines += ["## Stop condition", "", "```json", json.dumps(scan_info["failures"][0], indent=2), "```", ""]
    if scan_info["mission_stream_selector_sites"]:
        lines += ["## Static `0289` selector encodings", "",
                  "File offsets below are static scan coordinates, not execution reachability or native mission-mode values.", "",
                  "| File offset | Raw opcode | Tag | Selector | Classification |",
                  "| ---: | --- | --- | ---: | --- |"]
        for row in scan_info["mission_stream_selector_sites"]:
            literal = "" if row["literal"] is None else str(row["literal"])
            lines.append(f"| `{row['file_offset']}` | `{row['raw_opcode']}` | `{row['parameter_tag']}` | {literal} | `{row['classification']}` |")
        lines.append("")
    lines += ["## Most frequent decoded opcodes", "", "| Opcode | Name | Static occurrences | Condition |", "| --- | --- | ---: | --- |"]
    for row in scan_info["opcodes_by_frequency"][:100]:
        lines.append(f"| `{row['opcode']}` | `{row['name']}` | {row['count']} | {row['is_condition']} |")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scm", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()

    data = args.scm.read_bytes()
    if len(data) < HEADER_BYTES:
        raise ValueError("MAIN.SCM is shorter than its 8-byte header")
    main_segment_end = u32(data, 0)
    if not HEADER_BYTES <= main_segment_end <= len(data):
        raise ValueError(f"invalid main segment end 0x{main_segment_end:X} for {len(data)}-byte SCM")
    # This build has a script descriptor before the actual MAIN command stream.
    # The unique `SCRIPT_NAME "MAIN"` bytecode marker is a safer executable
    # entry than either header dword or the descriptor itself.
    main_marker = b"\x38\x02\x0A\x4D\x41\x49\x4E\x00"
    entry_point = data.find(main_marker, HEADER_BYTES, main_segment_end)
    if entry_point < HEADER_BYTES:
        raise ValueError("could not find the unique SCRIPT_NAME MAIN entry marker")
    if data.find(main_marker, entry_point + 1, main_segment_end) != -1:
        raise ValueError("SCRIPT_NAME MAIN entry marker is not unique")
    payload = {
        "schema": "vcs-main-scm-opcode-usage/v1",
        "method": "linear main-segment decode from recovered parameter widths; unknown data aborts rather than resynchronizing",
        "input": {
            "path": str(args.scm),
            "sha256": hashlib.sha256(data).hexdigest(),
            "bytes": len(data),
            "header": {
                "main_segment_end": f"0x{main_segment_end:06X}",
                "secondary_header_u32": f"0x{u32(data, 4):08X}",
                "non_code_header_bytes": f"0x{HEADER_BYTES:06X}",
                "main_script_name_marker": "SCRIPT_NAME \"MAIN\"",
                "linear_entry_point": f"0x{entry_point:06X}",
            },
        },
        "evidence_boundary": [
            "Counts are static bytecode occurrences, not execution frequency or reachability.",
            "Command names/parameter counts come from the public VCS Sanny library and are used only to delimit bytecode, except for individually proved VM-width overrides.",
            "Opcode 0024 START_NEW_SCRIPT is the first override: native PSP/PS2 decode one script index plus 0x60 child-local parameters, not the library's label/arguments pseudo-arity.",
            "CALL/CALL_NOT use the separately proved VCS header/input/output layout.",
            "VCS PSP CollectParameters maps tag 0x00 to a one-byte zero-value case; it is not imported as the reLCS ARGUMENT_END convention.",
            "The scanner stops on the first undecodable command and never treats data as resynchronised code.",
        ],
        "scan": scan(data, entry_point, main_segment_end, library_commands(args.library)),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.write_text(markdown(payload), encoding="utf-8")
    print(f"Decoded {payload['scan']['decoded_instruction_count']} VCS main-segment commands; fully_decoded={payload['scan']['fully_decoded']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
