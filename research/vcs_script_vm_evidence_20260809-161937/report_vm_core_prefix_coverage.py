#!/usr/bin/env python3
"""Report evidence-gated core coverage over the bounded VCS MAIN.SCM scan.

The MAIN.SCM inventory is deliberately a static linear prefix, not runtime
reachability. This tool preserves that boundary while answering the narrower
question: among the commands successfully decoded before its documented stop,
which opcodes are fully reconstructed, adapter-gated, or still explicit VM
faults?
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import defaultdict
from pathlib import Path
from typing import Any


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def category(status: str) -> str:
    # A recovered coupled host subsystem (currently 0289) is intentionally a
    # loud boundary, not an adapter-gated implementation.  It must remain in
    # the fault column even though its evidence status contains the word HOST.
    if "BOUNDARY_UNIMPLEMENTED" in status:
        return "UNIMPLEMENTED"
    if "ADAPTER" in status or "HOST_PROBE_REQUIRED" in status:
        return "ADAPTER_GATED"
    if status.startswith("IMPLEMENTED"):
        return "FULLY_IMPLEMENTED"
    return "UNIMPLEMENTED"


def markdown(payload: dict[str, Any]) -> str:
    totals = payload["command_totals"]
    scan = payload["scan_boundary"]
    lines = [
        "# VCS MAIN.SCM bounded-prefix core coverage",
        "",
        "This is coverage over the static linear inventory only — not a runtime ",
        "hotness, reachability, or whole-file-execution claim.",
        "",
        "## Boundary",
        "",
        f"- Successfully decoded commands: **{scan['decoded_instruction_count']}**",
        f"- Cursor after last decoded command: `{scan['cursor_after_last_decoded_instruction']}`",
        f"- Fully decoded: **{scan['fully_decoded']}**",
        f"- First stop: `{scan['first_failure']['raw_opcode']}` at `{scan['first_failure']['offset']}` — {scan['first_failure']['reason']}",
        "",
        "## Command coverage",
        "",
        "| Category | Commands | Distinct opcodes | Meaning |",
        "| --- | ---: | ---: | --- |",
        f"| Fully implemented | {totals['FULLY_IMPLEMENTED']['commands']} | {totals['FULLY_IMPLEMENTED']['opcodes']} | Portable core has a direct evidenced contract. |",
        f"| Adapter gated | {totals['ADAPTER_GATED']['commands']} | {totals['ADAPTER_GATED']['opcodes']} | VM contract is implemented; an explicit host dependency remains. |",
        f"| Explicit fault | {totals['UNIMPLEMENTED']['commands']} | {totals['UNIMPLEMENTED']['opcodes']} | No executable core handler, or a deliberate coupled host boundary; VM faults rather than pretending success. |",
        "",
        "## Highest-frequency still-unimplemented opcodes",
        "",
        "| Opcode | Public navigation label | Static count |",
        "| --- | --- | ---: |",
    ]
    for row in payload["highest_frequency_unimplemented"]:
        lines.append(f"| `{row['opcode']}` | `{row['name']}` | {row['count']} |")
    lines += [
        "",
        "The public labels above aid navigation only. They are not promoted to symbol-map names without direct per-handler proof.",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--coverage", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()

    inventory, coverage = load(args.inventory), load(args.coverage)
    status_by_opcode = {entry["opcode"]: entry["status"] for entry in coverage["handlers"]}
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in inventory["scan"]["opcodes_by_frequency"]:
        status = status_by_opcode.get(row["opcode"], "UNIMPLEMENTED")
        grouped[category(status)].append({**row, "core_status": status})

    totals: dict[str, dict[str, int]] = {}
    for label in ("FULLY_IMPLEMENTED", "ADAPTER_GATED", "UNIMPLEMENTED"):
        rows = grouped[label]
        totals[label] = {"commands": sum(int(row["count"]) for row in rows), "opcodes": len(rows)}

    failures = inventory["scan"]["failures"]
    payload = {
        "schema": "vcs-main-scm-core-prefix-coverage/v1",
        "purpose": "Evidence-gated portable VCS VM coverage over the bounded static linear MAIN.SCM inventory.",
        "inputs": {
            "inventory": {"path": str(args.inventory), "sha256": digest(args.inventory)},
            "core_coverage": {"path": str(args.coverage), "sha256": digest(args.coverage)},
        },
        "scan_boundary": {
            "decoded_instruction_count": inventory["scan"]["decoded_instruction_count"],
            "cursor_after_last_decoded_instruction": inventory["scan"]["cursor_after_last_decoded_instruction"],
            "fully_decoded": inventory["scan"]["fully_decoded"],
            "first_failure": failures[0] if failures else None,
        },
        "category_policy": {
            "FULLY_IMPLEMENTED": "Evidence coverage status begins with IMPLEMENTED and has no host/adapter boundary.",
            "ADAPTER_GATED": "Evidence coverage status records a required explicit host adapter/probe.",
            "UNIMPLEMENTED": "No executable evidence-gated core handler, or a proven coupled host-subsystem boundary; execution faults explicitly.",
        },
        "command_totals": totals,
        "rows": {key: sorted(value, key=lambda row: (-int(row["count"]), row["opcode"])) for key, value in grouped.items()},
        "highest_frequency_unimplemented": sorted(grouped["UNIMPLEMENTED"], key=lambda row: (-int(row["count"]), row["opcode"]))[:20],
    }
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    args.markdown.write_text(markdown(payload), encoding="utf-8")
    print(f"Wrote bounded-prefix coverage for {inventory['scan']['decoded_instruction_count']} decoded commands")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
