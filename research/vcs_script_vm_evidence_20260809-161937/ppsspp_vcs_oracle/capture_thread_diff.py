"""Thread/memory snapshots and deterministic diffs for the PPSSPP VCS oracle."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any


THREAD_FIELDS: tuple[tuple[int, int, str], ...] = (
    (0x010, 4, "script_ip"),
    (0x014, 4, "saved_script_ip"),
    (0x1FC, 4, "call_frame_data"),
    (0x200, 4, "wake_time"),
    (0x204, 2, "call_stack_depth"),
    (0x206, 2, "condition_state"),
    (0x209, 1, "condition_result"),
    (0x20A, 1, "thread_flag_20a"),
    (0x20B, 1, "wait_flag"),
    (0x20C, 1, "opcode_not_flag"),
    (0x217, 1, "thread_flag_217"),
)


def u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little")


def u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little")


def _ranges(before: bytes, after: bytes, address: int) -> list[dict[str, Any]]:
    """Coalesce byte differences; keeping raw values makes the proof replayable."""
    result: list[dict[str, Any]] = []
    start: int | None = None
    for index, (left, right) in enumerate(zip(before, after)):
        if left != right and start is None:
            start = index
        if left == right and start is not None:
            result.append({
                "address": f"0x{address + start:08X}",
                "offset": f"0x{start:X}",
                "before": before[start:index].hex(),
                "after": after[start:index].hex(),
            })
            start = None
    if start is not None:
        result.append({
            "address": f"0x{address + start:08X}",
            "offset": f"0x{start:X}",
            "before": before[start:].hex(),
            "after": after[start:].hex(),
        })
    return result


def thread_snapshot(address: int, data: bytes) -> dict[str, Any]:
    fields: dict[str, Any] = {}
    for offset, size, name in THREAD_FIELDS:
        if offset + size > len(data):
            continue
        value = int.from_bytes(data[offset : offset + size], "little")
        fields[name] = {"offset": f"0x{offset:X}", "size": size, "value": f"0x{value:0{size * 2}X}"}
    return {"address": f"0x{address:08X}", "size": len(data), "bytes": data.hex(), "fields": fields}


def thread_diff(address: int, before: bytes, after: bytes) -> dict[str, Any]:
    if len(before) != len(after):
        raise ValueError("Thread snapshots must have equal sizes")
    fields: list[dict[str, Any]] = []
    changed_offsets: set[int] = set()
    for offset, size, name in THREAD_FIELDS:
        if offset + size > len(before) or before[offset : offset + size] == after[offset : offset + size]:
            continue
        changed_offsets.add(offset)
        old, new = int.from_bytes(before[offset : offset + size], "little"), int.from_bytes(after[offset : offset + size], "little")
        fields.append({"name": name, "offset": f"0x{offset:X}", "address": f"0x{address + offset:08X}", "size": size, "before": f"0x{old:0{size * 2}X}", "after": f"0x{new:0{size * 2}X}"})
    return {"changed": before != after, "changed_field_offsets": [f"0x{value:X}" for value in sorted(changed_offsets)], "fields": fields, "ranges": _ranges(before, after, address)}


def memory_diff(address: int, before: bytes, after: bytes, name: str) -> dict[str, Any]:
    if len(before) != len(after):
        raise ValueError(f"Memory snapshot sizes differ for {name}")
    return {"name": name, "address": f"0x{address:08X}", "size": len(before), "changed": before != after, "ranges": _ranges(before, after, address)}
