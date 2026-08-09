"""Evidence-gated VCS PSP ↔ PS2 Script-VM opcode-table comparison.

Table membership is deliberately separated from handler semantics.  A handler
at the same opcode on both targets is `COMMON_SLOT`, not a semantic name or a
claim of binary equivalence.  Only a supplied manual proof may promote a slot
to `DIFFERENT_HANDLER_SEMANTICS`.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any


PSP_LAST = 0x055A
PS2_LAST = 0x0564


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def slot_map(table: dict[str, Any], expected_last: int, label: str) -> dict[int, dict[str, Any]]:
    first = int(table.get("opcode_first", 0))
    last = int(table.get("opcode_last", int(table.get("opcode_count_scanned", 0)) - 1))
    count = int(table.get("opcode_count_scanned", 0))
    if first != 0 or last != expected_last or count != expected_last + 1:
        raise ValueError(f"{label} range must be 0x0000..0x{expected_last:04X} ({expected_last + 1} slots), got first={first}, last={last}, count={count}")
    slots = table.get("slots")
    if not isinstance(slots, list) or len(slots) != count:
        raise ValueError(f"{label} table must include exactly {count} full slots; re-export with ExportOpcodeDispatchTable schema 2")
    values = {int(row["opcode"]): row for row in slots}
    if set(values) != set(range(count)):
        raise ValueError(f"{label} slots are not a complete contiguous range")
    return values


def proof_map(proof: dict[str, Any]) -> dict[int, dict[str, Any]]:
    values: dict[int, dict[str, Any]] = {}
    for row in proof.get("slots", []):
        opcode = int(str(row["opcode"]), 0) if isinstance(row["opcode"], str) else int(row["opcode"])
        values[opcode] = row
    return values


def markdown(payload: dict[str, Any], proof: dict[int, dict[str, Any]]) -> str:
    counts = payload["classification_counts"]
    lines = [
        "# VCS PSP ↔ PS2 opcode table comparison",
        "",
        "PSP range is `0x0000..0x055A` (1,371 slots); PS2 range is `0x0000..0x0564` (1,381 slots). `COMMON_SLOT` proves table membership on both platforms only. It is not a command-name or ABI claim.",
        "",
        "| Classification | Handler slots |",
        "| --- | ---: |",
    ]
    for key in ("COMMON_SLOT", "PSP_ONLY", "PS2_ONLY", "DIFFERENT_HANDLER_SEMANTICS"):
        lines.append(f"| `{key}` | {counts[key]} |")
    lines += ["", "## Manual semantic review started", "", "| Opcode | Disposition | Review status | Finding |", "| --- | --- | --- | --- |"]
    for opcode in (0x0000, 0x0001, 0x0002, 0x0004, 0x0005, 0x0006, 0x0007,
                   0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
                   0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014,
                   0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A,
                   0x001B, 0x001C, 0x001D, 0x001E, 0x0022, 0x0024, 0x0025,
                   0x0026, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E,
                   0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
                   0x0036, 0x0037, 0x0038, 0x0039, 0x003A,
                   0x003B, 0x0044, 0x0045, 0x0049, 0x0059, 0x005E, 0x005F, 0x0078, 0x0079, 0x007F, 0x00C8, 0x00C9,
                   0x009F, 0x00A5, 0x00CA, 0x0107, 0x010E, 0x0113, 0x01BA, 0x0203, 0x0238, 0x0268, 0x0289, 0x0377,
                   0x02DB, 0x02E2, 0x037A, 0x037B, 0x0401, 0x04E5, 0x04F2, 0x055A):
        row = next(item for item in payload["rows"] if item["opcode_decimal"] == opcode)
        item = proof[opcode]
        lines.append(f"| `{row['opcode']}` | `{row['classification']}` | `{item['status']}` | {item['finding']} |")
    tail = [proof[opcode] for opcode in range(0x055B, 0x0565)]
    lines += ["", "## `0x055B..0x0564`: PS2-only", "", "All ten slots have no PSP table record; each was manually inspected from the listed PS2 instruction windows. Their original command names remain intentionally withheld.", "", "| Opcode | Behavior proved from PS2 instruction window |", "| --- | --- |"]
    for item in tail:
        lines.append(f"| `{item['opcode']}` | {item['finding']} |")
    lines += ["", "`DIFFERENT_HANDLER_SEMANTICS` remains zero: no such classification is emitted from target-address or descriptor differences alone. It requires a manual proof of divergent behavior.", ""]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--psp", type=Path, required=True)
    parser.add_argument("--ps2", type=Path, required=True)
    parser.add_argument("--manual-proof", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--markdown", type=Path)
    args = parser.parse_args()

    psp_table, ps2_table, proof = load(args.psp), load(args.ps2), load(args.manual_proof)
    psp, ps2 = slot_map(psp_table, PSP_LAST, "PSP"), slot_map(ps2_table, PS2_LAST, "PS2")
    manual = proof_map(proof)
    rows: list[dict[str, Any]] = []
    totals: Counter[str] = Counter({
        "COMMON_SLOT": 0,
        "PSP_ONLY": 0,
        "PS2_ONLY": 0,
        "DIFFERENT_HANDLER_SEMANTICS": 0,
    })
    for opcode in range(PS2_LAST + 1):
        psp_row, ps2_row = psp.get(opcode), ps2[opcode]
        psp_present = bool(psp_row and psp_row.get("present"))
        ps2_present = bool(ps2_row.get("present"))
        # Empty records are not handler slots and therefore do not receive one
        # of the four handler classifications requested by the research plan.
        if not psp_present and not ps2_present:
            continue
        semantic = manual.get(opcode)
        if psp_present and ps2_present:
            classification = "DIFFERENT_HANDLER_SEMANTICS" if semantic and semantic.get("semantic_relation") == "different" else "COMMON_SLOT"
        elif psp_present:
            classification = "PSP_ONLY"
        else:
            classification = "PS2_ONLY"
        totals[classification] += 1
        rows.append({
            "opcode": f"0x{opcode:04X}",
            "opcode_decimal": opcode,
            "classification": classification,
            "psp": psp_row if psp_present else None,
            "ps2": ps2_row if ps2_present else None,
            "semantic_proof": semantic,
            "semantic_status": semantic.get("status", "UNREVIEWED") if semantic else "UNREVIEWED",
        })
    payload = {
        "schema": 1,
        "purpose": "PSP/PS2 VCS opcode handler table comparison; membership is not semantic equivalence.",
        "ranges": {"psp": "0x0000..0x055A", "ps2": "0x0000..0x0564"},
        "inputs": {
            "psp_table": {"path": str(args.psp), "sha256": sha256(args.psp), "dispatcher": psp_table.get("dispatcher"), "table": psp_table.get("table")},
            "ps2_handler_db": {"path": str(args.ps2), "sha256": sha256(args.ps2), "dispatcher": ps2_table.get("dispatcher"), "table": ps2_table.get("table")},
            "manual_proof": {"path": str(args.manual_proof), "sha256": sha256(args.manual_proof)},
        },
        "classification_policy": {
            "COMMON_SLOT": "both tables contain a handler record; semantics remains unproven unless semantic_proof says otherwise",
            "PSP_ONLY": "handler record only in the PSP 0x0000..0x055A table",
            "PS2_ONLY": "handler record only in the PS2 0x0000..0x0564 table",
            "DIFFERENT_HANDLER_SEMANTICS": "both tables contain a record and a manual proof explicitly establishes different behavior",
        },
        "classification_counts": dict(sorted(totals.items())),
        "rows": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    if args.markdown:
        args.markdown.parent.mkdir(parents=True, exist_ok=True)
        args.markdown.write_text(markdown(payload, manual), encoding="utf-8")
    print(f"Wrote {len(rows)} non-empty PSP/PS2 opcode rows to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
