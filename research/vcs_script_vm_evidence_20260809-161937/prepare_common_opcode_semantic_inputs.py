"""Build Ghidra target lists for every VCS PSP ↔ PS2 COMMON_SLOT.

The table comparison is the authority for membership.  This utility only
turns its COMMON_SLOT rows into compact TSV inputs for the semantic-feature
exporter; it makes no semantic assertion itself.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_targets(path: Path, rows: list[dict[str, Any]], platform: str) -> tuple[int, int]:
    values: list[tuple[int, str]] = []
    for row in rows:
        handler = row[platform]
        if not handler or not handler.get("target"):
            raise ValueError(f"COMMON_SLOT {row['opcode']} has no {platform} target")
        values.append((int(row["opcode_decimal"]), str(handler["target"]).lower()))
    values.sort()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "# Generated from VCS_PSP_PS2_OPCODE_COMPARE.json; opcode<TAB>table handler target\n"
        + "".join(f"0x{opcode:04X}\t{target}\n" for opcode, target in values),
        encoding="utf-8",
    )
    return len(values), len({target for _, target in values})


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison", type=Path, default=Path("VCS_PSP_PS2_OPCODE_COMPARE.json"))
    parser.add_argument("--psp-output", type=Path, default=Path("script_vm_matcher/artifacts/semantic_inputs/vcs_psp_common_handlers.tsv"))
    parser.add_argument("--ps2-output", type=Path, default=Path("script_vm_matcher/artifacts/semantic_inputs/vcs_ps2_common_handlers.tsv"))
    parser.add_argument("--manifest", type=Path, default=Path("script_vm_matcher/artifacts/semantic_inputs/vcs_common_handler_targets.json"))
    args = parser.parse_args()

    payload = json.loads(args.comparison.read_text(encoding="utf-8"))
    rows = [row for row in payload["rows"] if row["classification"] == "COMMON_SLOT"]
    expected = int(payload["classification_counts"]["COMMON_SLOT"])
    if len(rows) != expected:
        raise ValueError(f"COMMON_SLOT count mismatch: rows={len(rows)}, classification={expected}")
    psp_rows, psp_unique = write_targets(args.psp_output, rows, "psp")
    ps2_rows, ps2_unique = write_targets(args.ps2_output, rows, "ps2")
    manifest = {
        "schema": 1,
        "purpose": "Ghidra semantic feature targets; table membership only",
        "comparison": {"path": str(args.comparison), "sha256": sha256(args.comparison)},
        "common_slots": expected,
        "psp": {"path": str(args.psp_output), "rows": psp_rows, "unique_targets": psp_unique},
        "ps2": {"path": str(args.ps2_output), "rows": ps2_rows, "unique_targets": ps2_unique},
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Prepared {expected} COMMON_SLOT rows ({psp_unique} PSP and {ps2_unique} PS2 unique targets)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
