#!/usr/bin/env python3
"""Summarize a read-only vcs_vm_main_scm_prefix_harness JSONL route trace.

The result is an observed portable-VM route over original SCM bytes. It is not
a PPSSPP trace and not a whole-program reachability proof. Transfer edges are
reported only for opcodes whose IP-changing VM role has already been proved.
"""

from __future__ import annotations

import argparse
import collections
import json
from pathlib import Path
from typing import Any


CONTROL_TRANSFER_OPCODES = {
    0x0002: "GOTO",
    0x0022: "GOTO_IF_FALSE",
    0x0025: "GOSUB",
    0x0026: "RETURN",
    0x005E: "RETURN_TRUE",
    0x005F: "RETURN_FALSE",
    0x01BA: "GOSUB_FILE",
    0x037A: "CALL_NOT",
    0x037B: "CALL",
}


def load(path: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip():
            continue
        value = json.loads(line)
        if not isinstance(value, dict):
            raise ValueError(f"Trace row {number} is not an object")
        rows.append(value)
    if not rows:
        raise ValueError("Trace contains no execution rows")
    return rows


def opcode_int(row: dict[str, Any]) -> int:
    return int(str(row["opcode"]), 0)


def build(rows: list[dict[str, Any]], trace: Path) -> dict[str, Any]:
    counts = collections.Counter(opcode_int(row) for row in rows)
    transfers = []
    for row in rows:
        opcode = opcode_int(row)
        if opcode in CONTROL_TRANSFER_OPCODES:
            transfers.append({
                "index": row["index"],
                "role": CONTROL_TRANSFER_OPCODES[opcode],
                "opcode": f"0x{opcode:04X}",
                "ip_before": row["ip_before"],
                "ip_after": row["ip_after"],
                "status": row["status"],
                "fault": row["fault"],
            })
    terminal = rows[-1]
    return {
        "schema": "vcs-main-scm-observed-route/v1",
        "scope": "Read-only portable-VM execution route over original SCM bytes. It records only the one selected entry route; it is neither PPSSPP telemetry nor whole-program reachability.",
        "input_trace": str(trace),
        "execution": {
            "commands": len(rows),
            "entry_ip": rows[0]["ip_before"],
            "terminal": terminal,
            "unique_opcodes": len(counts),
            "opcode_frequency": [
                {"opcode": f"0x{opcode:04X}", "count": count}
                for opcode, count in sorted(counts.items(), key=lambda item: (-item[1], item[0]))
            ],
        },
        "observed_proven_control_transfers": transfers,
        "guardrails": [
            "Only opcode roles with an existing VM control-flow proof are rendered as transfer edges.",
            "A transfer in this route does not establish that every static caller or target is reachable in the game.",
            "The terminal host-subsystem fault documents an integration boundary; it is not classified as an unknown opcode.",
        ],
    }


def markdown(payload: dict[str, Any]) -> str:
    execution = payload["execution"]
    terminal = execution["terminal"]
    lines = [
        "# VCS MAIN.SCM observed portable-VM route",
        "",
        "This is a read-only route through original SCM bytes in `vcs_vm_core`, not PPSSPP runtime telemetry and not a whole-program CFG claim.",
        "",
        f"- Entry IP: `{execution['entry_ip']}`",
        f"- Commands before terminal result: `{execution['commands']}`",
        f"- Distinct opcode IDs: `{execution['unique_opcodes']}`",
        f"- Terminal: index `{terminal['index']}`, opcode `{terminal['opcode']}`, IP `{terminal['ip_before']} → {terminal['ip_after']}`, `{terminal['fault']}`",
        "",
        "## Observed VM-proven control transfers",
        "",
        "| Index | Role | Opcode | IP before | IP after |",
        "| ---: | --- | --- | --- | --- |",
    ]
    for item in payload["observed_proven_control_transfers"]:
        lines.append(f"| {item['index']} | {item['role']} | `{item['opcode']}` | `{item['ip_before']}` | `{item['ip_after']}` |")
    lines += [
        "",
        "## Opcode frequency on this route",
        "",
        "| Opcode | Count |",
        "| --- | ---: |",
    ]
    for item in execution["opcode_frequency"]:
        lines.append(f"| `{item['opcode']}` | {item['count']} |")
    lines += [
        "",
        "The terminal `0289` is a direct PSP/PS2-proved mission-streaming subsystem boundary. Its selector is decoded before the explicit fault; no generic mission-launch adapter is substituted.",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()
    payload = build(load(args.trace), args.trace)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    args.markdown.write_text(markdown(payload), encoding="utf-8")
    print(f"Wrote {payload['execution']['commands']} command route to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
