"""Compare review-only normalized PSP/PS2 features for every COMMON_SLOT.

The result is machine triage, not proof.  In particular, a low score or raw
address difference never changes an opcode from COMMON_SLOT to
DIFFERENT_HANDLER_SEMANTICS.  That transition is still reserved for a manual
semantic proof in VCS_PSP_PS2_MANUAL_SEMANTIC_PROOF.json.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


SIGNALS = (
    "parameter_decoder_calls",
    "thread_accesses",
    "global_access_pattern",
    "small_constants",
    "callee_kinds",
    "cfg",
    "native_handler_status_constants",
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def feature_map(payload: dict[str, Any]) -> dict[str, dict[str, Any]]:
    values: dict[str, dict[str, Any]] = {}
    for row in payload.get("handlers", []):
        target = str(row["target"]).lower()
        if target in values:
            raise ValueError(f"Duplicate semantic feature target {target}")
        values[target] = row
    return values


def counter_similarity(left: Iterable[Any] | dict[str, Any], right: Iterable[Any] | dict[str, Any]) -> tuple[float, str]:
    left_counter = Counter(left) if not isinstance(left, dict) else Counter({str(k): int(v) for k, v in left.items()})
    right_counter = Counter(right) if not isinstance(right, dict) else Counter({str(k): int(v) for k, v in right.items()})
    if not left_counter and not right_counter:
        return 1.0, "both_empty"
    overlap = sum((left_counter & right_counter).values())
    union = sum((left_counter | right_counter).values())
    score = overlap / union if union else 1.0
    return score, "exact" if score == 1.0 else ("overlap" if overlap else "disjoint")


def cfg_similarity(left: dict[str, Any], right: dict[str, Any]) -> tuple[float, str]:
    keys = ("basic_blocks_approx", "conditional_branches", "jumps", "call_sites", "return_sites", "loads", "stores")
    distance = sum(abs(int(left.get(key, 0)) - int(right.get(key, 0))) for key in keys)
    scale = sum(max(int(left.get(key, 0)), int(right.get(key, 0)), 1) for key in keys)
    score = 1.0 - distance / scale
    return score, "exact" if distance == 0 else ("close" if score >= 0.70 else "different_shape")


def signal(name: str, psp: dict[str, Any], ps2: dict[str, Any]) -> dict[str, Any]:
    source_name = "return_semantics" if name == "native_handler_status_constants" else name
    if name == "cfg":
        score, relation = cfg_similarity(psp.get(source_name, {}), ps2.get(source_name, {}))
        return {"psp": psp.get(source_name, {}), "ps2": ps2.get(source_name, {}), "similarity": round(score, 6), "relation": relation}
    default = {} if source_name in {"callee_kinds", "return_semantics"} else []
    score, relation = counter_similarity(psp.get(source_name, default), ps2.get(source_name, default))
    return {"psp": psp.get(source_name, default), "ps2": ps2.get(source_name, default), "similarity": round(score, 6), "relation": relation}


def triage(signals: dict[str, dict[str, Any]], psp: dict[str, Any], ps2: dict[str, Any], manual: dict[str, Any] | None) -> str:
    relation = manual.get("semantic_relation") if manual else None
    if relation == "same":
        return "MANUAL_SAME"
    if relation == "different":
        return "MANUAL_DIFFERENT"
    if manual:
        return "MANUAL_PARTIAL" if manual.get("status") == "PARTIAL_MANUAL_REVIEW" else "MANUAL_REVIEW"
    if psp.get("truncated") or ps2.get("truncated") or psp.get("source") != "function_body" or ps2.get("source") != "function_body":
        return "LIMITED_RECOVERY"
    average = sum(item["similarity"] for item in signals.values()) / len(signals)
    return "STRUCTURAL_HIGH" if average >= 0.82 else ("STRUCTURAL_MEDIUM" if average >= 0.52 else "STRUCTURAL_LOW")


def concise(value: Any, limit: int = 12) -> str:
    if isinstance(value, dict):
        rendered = ", ".join(f"{key}:{item}" for key, item in sorted(value.items())) or "—"
    else:
        rendered = ", ".join(map(str, value)) or "—"
    return rendered if len(rendered) <= 110 else rendered[:107] + "..."


def markdown(payload: dict[str, Any]) -> str:
    totals = payload["triage_counts"]
    lines = [
        "# VCS PSP ↔ PS2 COMMON_SLOT semantic-shape comparison",
        "",
        "This is a normalized MIPS-level triage of all table-common handlers. It compares parameter-decoder calls, direct `CRunningScript` offsets, global read/write shape, small constants, normalized callee kinds, approximate CFG and native handler-status constants. A status constant is interpreter ABI control flow, not a script-command output. It does **not** infer command identity or automatically emit `DIFFERENT_HANDLER_SEMANTICS`.",
        "",
        "| Triage | Slots |",
        "| --- | ---: |",
    ]
    lines.extend(f"| `{key}` | {value} |" for key, value in sorted(totals.items()))
    lines += ["", "## Highest-priority manual-review candidates", "", "| Opcode | Triage | Decoder / thread fields | VM dispatch-status constants | Mean signal similarity |", "| --- | --- | --- | --- | ---: |"]
    candidates = sorted(payload["rows"], key=lambda row: (row["triage"] not in {"STRUCTURAL_LOW", "LIMITED_RECOVERY", "MANUAL_PARTIAL"}, row["mean_similarity"], row["opcode_decimal"]))
    for row in candidates[:80]:
        s = row["signals"]
        lines.append(
            f"| `{row['opcode']}` | `{row['triage']}` | {concise(s['parameter_decoder_calls']['psp'])} / {concise(s['thread_accesses']['psp'])} | {concise(s['native_handler_status_constants']['psp'])} / {concise(s['native_handler_status_constants']['ps2'])} | {row['mean_similarity']:.3f} |"
        )
    lines += ["", "The complete per-opcode signal payload is in the companion JSON. A `COMMON_SLOT` remains a membership classification until manual decompilation establishes either semantic equivalence or a semantic difference.", ""]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison", type=Path, default=Path("VCS_PSP_PS2_OPCODE_COMPARE.json"))
    parser.add_argument("--psp-features", type=Path, required=True)
    parser.add_argument("--ps2-features", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("VCS_PSP_PS2_COMMON_SEMANTICS.json"))
    parser.add_argument("--markdown", type=Path, default=Path("reports/VCS_PSP_PS2_COMMON_SEMANTICS.md"))
    args = parser.parse_args()

    comparison = json.loads(args.comparison.read_text(encoding="utf-8"))
    psp = feature_map(json.loads(args.psp_features.read_text(encoding="utf-8")))
    ps2 = feature_map(json.loads(args.ps2_features.read_text(encoding="utf-8")))
    rows: list[dict[str, Any]] = []
    totals: Counter[str] = Counter()
    for row in comparison["rows"]:
        if row["classification"] != "COMMON_SLOT":
            continue
        psp_target = str(row["psp"]["target"]).lower()
        ps2_target = str(row["ps2"]["target"]).lower()
        if psp_target not in psp or ps2_target not in ps2:
            raise ValueError(f"Missing feature record for {row['opcode']}: PSP {psp_target} / PS2 {ps2_target}")
        psp_feature, ps2_feature = psp[psp_target], ps2[ps2_target]
        signals = {name: signal(name, psp_feature, ps2_feature) for name in SIGNALS}
        manual = row.get("semantic_proof")
        disposition = triage(signals, psp_feature, ps2_feature, manual)
        totals[disposition] += 1
        rows.append({
            "opcode": row["opcode"], "opcode_decimal": row["opcode_decimal"], "classification": "COMMON_SLOT",
            "psp_target": psp_target, "ps2_target": ps2_target,
            "psp_feature_source": psp_feature.get("source"), "ps2_feature_source": ps2_feature.get("source"),
            "manual_semantic_status": row.get("semantic_status", "UNREVIEWED"),
            "triage": disposition, "signals": signals,
            "mean_similarity": round(sum(item["similarity"] for item in signals.values()) / len(signals), 6),
        })
    expected = int(comparison["classification_counts"]["COMMON_SLOT"])
    if len(rows) != expected:
        raise ValueError(f"Expected {expected} COMMON_SLOT rows, compared {len(rows)}")
    payload = {
        "schema": 2,
        "purpose": "Machine semantic-shape triage for COMMON_SLOT only; native handler-status constants represent interpreter control flow, not script-command outputs; automatic scoring never changes opcode semantic classification.",
        "inputs": {
            "comparison": {"path": str(args.comparison), "sha256": sha256(args.comparison)},
            "psp_features": {"path": str(args.psp_features), "sha256": sha256(args.psp_features)},
            "ps2_features": {"path": str(args.ps2_features), "sha256": sha256(args.ps2_features)},
        },
        "signals": list(SIGNALS),
        "classification_guard": "Only manual proof can set DIFFERENT_HANDLER_SEMANTICS in VCS_PSP_PS2_OPCODE_COMPARE.json.",
        "common_slots_compared": len(rows), "triage_counts": dict(sorted(totals.items())), "rows": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.write_text(markdown(payload), encoding="utf-8")
    print(f"Compared semantic-shape features for {len(rows)} COMMON_SLOT handlers to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
