#!/usr/bin/env python3
"""Evidence-gated Script VM research chain for reLCS -> LCS -> VCS.

This replaces the previous four-way fuzzy beam output as the active workflow.
It has a deliberately asymmetric order:

    reLCS source -> LCS Android exact C++ symbols -> LCS PSP MIPS -> VCS PSP MIPS

The program writes review candidates freely, but writes confirmed mappings only
when the evidence policy is satisfied.  It never changes a Ghidra label or any
game executable.
"""

from __future__ import annotations

import argparse
import hashlib
import heapq
import json
import math
import re
import subprocess
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

from script_vm_matcher import extract_source_semantics, source_role


WORKSPACE = Path(__file__).resolve().parents[2]
MATCHER = WORKSPACE / "script_vm_matcher"
EXPECTED = {
    "lcs_android": "c46507f92e1e02bc7e79f476aba02fa731682fc5ca3c320624fa8a99e854adb1",
    "lcs_psp": "85fb68879359dbbc3f85c0747ef8dd61ff62a9b8219c12a5078f7d8867a7a160",
    "vcs_psp": "a722be366b2090676f225941235e7a01cb4a42b242b14e785180162f46376b12",
}
POLICY = {
    "automatic_rename": [
        "EXACT_SYMBOL",
        "NORMALIZED_MIPS_EXACT + independent semantic anchor",
        "BINDIFF_HIGH + CALLGRAPH_CONFIRMED",
    ],
    "never_automatic": [
        "p-code histogram similarity",
        "CFG/size similarity",
        "constant-bucket similarity",
        "role affinity",
        "a fuzzy score alone",
    ],
    "chain": "reLCS source -> LCS Android exact C++ symbols -> LCS PSP MIPS -> VCS PSP MIPS",
}
RW_SYMBOL = re.compile(r"\b(?:(?:Rw|Rp|Rx|Rt)[A-Za-z_][A-Za-z0-9_]*|rw(?:DEVICE|OPENGL|D3D|PS2)[A-Za-z0-9_]*)\b")


def now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"Expected JSON object: {path}")
    return value


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding="utf-8")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def short_function(function: dict[str, Any]) -> dict[str, Any]:
    keep = (
        "address", "name", "namespace", "size", "instructions", "basic_blocks",
        "pcode_count", "call_ops", "branch_ops", "cbranch_ops", "load_ops", "store_ops",
        "pcode", "constant_buckets", "strings", "callees",
    )
    return {key: function.get(key) for key in keep if key in function}


def qualified(function: dict[str, Any]) -> str:
    namespace = str(function.get("namespace", ""))
    name = str(function.get("name", ""))
    return f"{namespace}::{name}" if namespace and namespace != "Global" else name


def symbol_parts(symbol: str) -> tuple[str, str]:
    owner, name = symbol.split("::", 1)
    return owner, name


def role_for(symbol: str) -> str:
    owner, name = symbol_parts(symbol)
    return source_role(owner, name) or "script_vm_support"


def ratio_similarity(left: float, right: float, floor: float = 1.0) -> float:
    return math.exp(-abs(math.log((left + floor) / (right + floor))))


def density(function: dict[str, Any], key: str) -> float:
    return float(function.get(key, 0) or 0) / max(float(function.get("pcode_count", 0) or 0), 1.0)


def counter_cosine(left: dict[str, Any], right: dict[str, Any]) -> float:
    a = {str(key): float(value) for key, value in left.items() if value}
    b = {str(key): float(value) for key, value in right.items() if value}
    if not a or not b:
        return 0.0
    numerator = sum(value * b.get(key, 0.0) for key, value in a.items())
    norm_a = math.sqrt(sum(value * value for value in a.values()))
    norm_b = math.sqrt(sum(value * value for value in b.values()))
    return numerator / (norm_a * norm_b) if norm_a and norm_b else 0.0


def token_jaccard(left: Iterable[str], right: Iterable[str]) -> tuple[float, list[str]]:
    word = re.compile(r"[a-z][a-z0-9_]{2,}")
    a = set(word.findall(" ".join(left).lower()))
    b = set(word.findall(" ".join(right).lower()))
    if not a or not b:
        return 0.0, []
    return len(a & b) / len(a | b), sorted(a & b)


def android_lcs_score(source: dict[str, Any], target: dict[str, Any]) -> tuple[float, list[dict[str, Any]]]:
    cfg = (
        ratio_similarity(float(source.get("basic_blocks", 0)), float(target.get("basic_blocks", 0)))
        + ratio_similarity(density(source, "branch_ops"), density(target, "branch_ops"), 0.001)
        + ratio_similarity(density(source, "cbranch_ops"), density(target, "cbranch_ops"), 0.001)
    ) / 3.0
    memory = (
        ratio_similarity(density(source, "load_ops"), density(target, "load_ops"), 0.001)
        + ratio_similarity(density(source, "store_ops"), density(target, "store_ops"), 0.001)
    ) / 2.0
    calls = (
        ratio_similarity(density(source, "call_ops"), density(target, "call_ops"), 0.001)
        + ratio_similarity(len(source.get("callees", [])), len(target.get("callees", [])))
    ) / 2.0
    pcode = counter_cosine(source.get("pcode", {}), target.get("pcode", {}))
    constants = counter_cosine(source.get("constant_buckets", {}), target.get("constant_buckets", {}))
    strings, common = token_jaccard(source.get("strings", []), target.get("strings", []))
    evidence = [
        {"kind": "CFG_PROFILE", "score": round(cfg, 4), "independent_dimension": "basic blocks and branch densities"},
        {"kind": "DATA_ACCESS_PROFILE", "score": round(memory, 4), "independent_dimension": "load/store density"},
        {"kind": "CALL_TOPOLOGY_PROFILE", "score": round(calls, 4), "independent_dimension": "call density and direct fan-out"},
        {"kind": "PCODE_AUXILIARY", "score": round(pcode, 4), "independent_dimension": "architecture-neutral p-code histogram", "not_sufficient_alone": True},
        {"kind": "CONSTANT_BUCKET_AUXILIARY", "score": round(constants, 4), "independent_dimension": "coarse constant sizes", "not_sufficient_alone": True},
    ]
    if common:
        evidence.append({"kind": "STRING_CONTEXT", "score": round(strings, 4), "shared_tokens": common[:12]})
    score = 0.29 * cfg + 0.22 * memory + 0.20 * calls + 0.17 * pcode + 0.08 * constants + 0.04 * strings
    return score, evidence


def mips_score(source: dict[str, Any], target: dict[str, Any], source_sig: dict[str, Any] | None, target_sig: dict[str, Any] | None) -> tuple[float, list[dict[str, Any]], str]:
    normalized_exact = bool(source_sig and target_sig and source_sig.get("normalized_mips_sha256") == target_sig.get("normalized_mips_sha256"))
    opcode = counter_cosine((source_sig or {}).get("opcode_histogram", {}), (target_sig or {}).get("opcode_histogram", {}))
    raw, raw_evidence = android_lcs_score(source, target)
    evidence = [
        {"kind": "NORMALIZED_MIPS_EXACT" if normalized_exact else "NORMALIZED_MIPS_FINGERPRINT", "score": 1.0 if normalized_exact else 0.0,
         "independent_dimension": "relocation-neutral opcode/register/operand/delay-slot sequence",
         "normalization": "mips-v1-relocation-neutral"},
        {"kind": "MIPS_OPCODE_HISTOGRAM", "score": round(opcode, 4), "independent_dimension": "same-ISA opcode distribution", "not_sufficient_alone": True},
    ] + raw_evidence
    if normalized_exact:
        return 1.0, evidence, "NORMALIZED_MIPS_EXACT"
    return 0.48 * raw + 0.52 * opcode, evidence, "REVIEW"


def ranked_candidates(
    source: dict[str, Any], targets: list[dict[str, Any]], limit: int,
    score_fn: Any,
) -> list[dict[str, Any]]:
    # heapq keeps output bounded without converting similarities into identities.
    heap: list[tuple[float, str, dict[str, Any], list[dict[str, Any]], str]] = []
    for target in targets:
        score, evidence, confidence = score_fn(source, target)
        record = (score, str(target.get("address")), target, evidence, confidence)
        if len(heap) < limit:
            heapq.heappush(heap, record)
        elif record[:2] > heap[0][:2]:
            heapq.heapreplace(heap, record)
    ordered = sorted(heap, key=lambda item: (-item[0], item[1]))
    rows = []
    for rank, (score, _address, target, evidence, confidence) in enumerate(ordered, 1):
        rows.append({
            "rank": rank, "candidate": short_function(target), "heuristic_score": round(score, 4),
            "confidence": confidence, "evidence": evidence,
            "automatic_rename": False,
        })
    return rows


def safe_git_rev(path: Path) -> str | None:
    try:
        return subprocess.run(["git", "-C", str(path), "rev-parse", "HEAD"], check=True, text=True, capture_output=True).stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return None


def source_records(root: Path) -> tuple[dict[str, Any], dict[str, dict[str, Any]]]:
    semantics = extract_source_semantics(root)
    semantics["oracle"] = {
        "kind": "reLCS source", "path": str(root), "commit": safe_git_rev(root),
        "policy": "Source semantics are the first-stage identity oracle; ReLCS.dll is intentionally excluded from this chain.",
    }
    by_symbol = {item["id"]: item for item in semantics["functions"]}
    return semantics, by_symbol


def validate_android(anchors: dict[str, Any], android: dict[str, Any], source_by_symbol: dict[str, dict[str, Any]]) -> dict[str, Any]:
    expected_hash = anchors["provenance"]["binary_sha256"]
    actual_hash = android.get("sha256")
    if actual_hash != expected_hash:
        raise ValueError(f"Android SHA-256 mismatch: expected {expected_hash}, got {actual_hash}")
    by_address = {str(item.get("address", "")).lower(): item for item in android.get("functions", [])}
    exact = []
    errors = []
    for anchor in anchors["anchors"]:
        address = str(anchor["address"]).lower()
        symbol = anchor["symbol"]
        owner, name = symbol_parts(symbol)
        function = by_address.get(address)
        if function is None:
            errors.append({"symbol": symbol, "address": address, "error": "address_absent"})
            continue
        observed = qualified(function)
        if function.get("namespace") != owner or function.get("name") != name:
            errors.append({"symbol": symbol, "address": address, "error": "symbol_mismatch", "observed": observed})
            continue
        source = source_by_symbol.get(symbol)
        exact.append({
            "symbol": symbol, "role": role_for(symbol), "android": short_function(function),
            "re_lcs_source": {key: source.get(key) for key in ("file", "line", "source_lines", "switches", "loops", "calls", "semantic_evidence")} if source else None,
            "confidence": "EXACT_SYMBOL",
            "evidence": [
                {"kind": "EXACT_SOURCE_IDENTITY", "source_symbol": symbol, "source_present": source is not None},
                {"kind": "EXACT_SYMBOL", "address": address, "namespace": owner, "name": name},
                {"kind": "INPUT_SHA256", "sha256": actual_hash},
            ],
            "classification": "UNKNOWN",
            "automatic_rename": False,
        })
    if errors:
        raise ValueError("Android exact-anchor validation failed: " + json.dumps(errors))
    return {
        "schema": 1, "generated_at": now(), "stage": "reLCS source -> LCS Android", "status": "COMPLETE",
        "input": {"program": android.get("program"), "sha256": actual_hash, "expected_sha256": expected_hash},
        "policy": "Address + namespace + C++ symbol equality are all required.", "anchors": exact,
    }


def candidate_stage(exact: dict[str, Any], lcs: dict[str, Any]) -> dict[str, Any]:
    if lcs.get("sha256") != EXPECTED["lcs_psp"]:
        raise ValueError("LCS PSP SHA-256 mismatch")
    targets = [item for item in lcs.get("functions", []) if int(item.get("instructions", 0) or 0) >= 4]
    rows = []
    for anchor in exact["anchors"]:
        source = anchor["android"]
        candidates = ranked_candidates(source, targets, 12, lambda left, right: (*android_lcs_score(left, right), "REVIEW"))
        rows.append({
            "android_anchor": {"symbol": anchor["symbol"], "role": anchor["role"], "address": source["address"]},
            "status": "REVIEW", "reason": "Cross-architecture measurements are candidate-generation evidence, not identity proof.",
            "candidates": candidates,
        })
    return {
        "schema": 1, "generated_at": now(), "stage": "LCS Android -> LCS PSP", "status": "REVIEW_ONLY",
        "input": {"program": lcs.get("program"), "sha256": lcs.get("sha256"), "functions": len(targets)},
        "confidence_floor": "No candidate is confirmed or eligible for a rename from this stage alone.",
        "matches": rows,
    }


def confirm_lcs_dispatcher(candidates: dict[str, Any], lcs: dict[str, Any], workspace: Path) -> None:
    """Add the manually inspected, topology-anchored LCS dispatcher.

    The first fuzzy ranking did *not* find this function (its top result was a
    rendering routine).  This record is intentionally explicit about the
    independent proof so future runs cannot silently turn the old heuristic
    into a source of truth.
    """
    address = "0x00153a90"
    function = next((item for item in lcs["functions"] if item.get("address") == address), None)
    if function is None:
        raise ValueError("Validated LCS dispatcher is absent from the current export")
    review = workspace / "script_vm_matcher" / "artifacts" / "review" / "lcs_psp" / "00153a90_FUN_00153a90.c"
    if not review.exists():
        raise ValueError(f"Dispatcher review artifact required before confirmation: {review}")
    text = review.read_text(encoding="utf-8", errors="replace")
    required = ("+ 2", "0x7fff", "uVar3 < 100", "0x6a4")
    if not all(token in text for token in required):
        raise ValueError("Dispatcher review artifact no longer contains the validated semantic landmarks")
    evidence = [
        {"kind": "EXACT_SOURCE_IDENTITY", "source_symbol": "CRunningScript::ProcessOneCommand", "android_anchor": "0x00304a14"},
        {"kind": "CALLGRAPH_CONFIRMED", "fact": "17 direct targets form contiguous opcode pages from 0–99 through 1600–1699"},
        {"kind": "MULTI_SIGNAL_HIGH", "fact": "reads a 16-bit opcode from script space via the running-script IP, advances IP by two, separates the high-bit not flag, then range-dispatches"},
        {"kind": "MANUAL_DECOMPILATION_REVIEW", "artifact": str(review.relative_to(workspace)), "sha256": sha256(review)},
    ]
    for mapping in candidates["matches"]:
        if mapping["android_anchor"]["symbol"] != "CRunningScript::ProcessOneCommand":
            continue
        confirmed = {
            "rank": 1, "candidate": short_function(function), "heuristic_score": None,
            "confidence": "CALLGRAPH_CONFIRMED", "evidence": evidence,
            "automatic_rename": False,
            "manual_confirmation": True,
        }
        mapping["status"] = "CONFIRMED_MANUAL_REVIEW"
        mapping["reason"] = "Confirmed by Script IP/opcode semantics and ordered command-page callgraph; no automatic label has been applied."
        mapping["candidates"] = [confirmed] + [dict(item, rank=index) for index, item in enumerate(mapping["candidates"], 2)]
        return
    raise ValueError("ProcessOneCommand anchor absent from candidate stage")


def parse_script_command_enum(source_root: Path) -> dict[int, str]:
    """Read the first command enum without assuming it describes VCS slots."""
    header = source_root / "src" / "control" / "ScriptCommands.h"
    text = header.read_text(encoding="utf-8", errors="replace")
    values: dict[int, str] = {}
    inside = False
    current = -1
    for line in text.splitlines():
        if not inside and re.match(r"\s*enum\s*\{", line):
            inside = True
            continue
        if inside and "};" in line:
            break
        match = re.match(r"\s*(COMMAND_[A-Z0-9_]+)\s*(?:=\s*(0x[0-9a-fA-F]+|\d+))?\s*,", line)
        if not match:
            continue
        current = int(match.group(2), 0) if match.group(2) else current + 1
        values[current] = match.group(1)
    if not values:
        raise ValueError(f"No Script VM command enum found in {header}")
    return values


def build_dispatch_propagation(table: dict[str, Any]) -> dict[str, Any]:
    entries = table.get("entries", [])
    by_target: dict[str, list[dict[str, Any]]] = defaultdict(list)
    by_page: dict[int, list[dict[str, Any]]] = defaultdict(list)
    for entry in entries:
        by_target[str(entry["target"])].append(entry)
        by_page[int(entry["opcode"]) // 100].append(entry)
    targets = []
    for target, rows in sorted(by_target.items(), key=lambda item: int(item[1][0]["opcode"])):
        sample = rows[0]
        targets.append({
            "target": target, "target_function": sample.get("target_function"),
            "opcode_count": len(rows), "opcodes": [row["opcode"] for row in rows],
            "proposed_role_name": f"CRunningScript::OpcodeHandler_{target[2:]}",
            "confidence": "EXACT_DISPATCH_TABLE", "automatic_rename": False,
            "note": "Role annotation only; numeric opcode-to-command-name equivalence remains unverified.",
        })
    pages = []
    for page, rows in sorted(by_page.items()):
        first, last = page * 100, page * 100 + 99
        pages.append({
            "logical_page": f"OpcodeTable[{first:04d}-{last:04d}]", "range": [first, last],
            "implemented_opcodes": len(rows), "unique_indirect_targets": len({row["target"] for row in rows}),
            "confidence": "EXACT_DISPATCH_TABLE",
            "note": "VCS dispatches through table entries, not LCS-style direct ProcessCommandsNNNToMMM callees.",
        })
    return {
        "schema": 1, "generated_at": now(), "dispatcher": "CRunningScript::ProcessOneCommand",
        "dispatcher_address": "0x0005e5e8", "dispatch_model": "opcode × 8 descriptor/function-pointer table",
        "table": table.get("table"), "opcode_count_scanned": table.get("opcode_count_scanned"),
        "nonzero_entries": table.get("nonzero_entries"), "logical_pages": pages, "indirect_targets": targets,
        "policy": "External role annotations only; this tool never writes Ghidra function labels.",
    }


def build_vcs_opcode_delta(source_root: Path, table: dict[str, Any]) -> dict[str, Any]:
    lcs_commands = parse_script_command_enum(source_root)
    table_by_opcode = {int(entry["opcode"]): entry for entry in table.get("entries", [])}
    end = max(max(lcs_commands), int(table.get("opcode_count_scanned", 0)) - 1)
    rows = []
    totals: Counter[str] = Counter()
    for opcode in range(end + 1):
        lcs_name = lcs_commands.get(opcode)
        vcs = table_by_opcode.get(opcode)
        if lcs_name and vcs:
            status = "NUMERIC_SLOT_PRESENT_BOTH_UNVERIFIED"
        elif lcs_name:
            status = "LCS_SOURCE_ONLY"
        elif vcs:
            status = "VCS_TABLE_ONLY"
        else:
            status = "UNIMPLEMENTED_OR_OUT_OF_RANGE"
        totals[status] += 1
        rows.append({
            "opcode": opcode, "lcs_source_command": lcs_name,
            "vcs_target": vcs.get("target") if vcs else None,
            "vcs_target_function": vcs.get("target_function") if vcs else None,
            "vcs_descriptor": vcs.get("descriptor") if vcs else None,
            "vcs_local_offset": vcs.get("local_offset") if vcs else None,
            "vcs_selector": vcs.get("selector") if vcs else None, "status": status,
        })
    return {
        "schema": 1, "generated_at": now(), "status": "STRUCTURAL_DISPATCH_DELTA_COMPLETE",
        "scope": "Confirmed VCS dispatcher table versus reLCS command-enum numeric slots.",
        "important_caveat": "A same-number source command is a navigation reference, not a confirmed VCS semantic command name. Individual command semantics still require validation.",
        "lcs_source_command_count": len(lcs_commands), "vcs_table_opcode_count": table.get("opcode_count_scanned"),
        "vcs_nonzero_table_entries": table.get("nonzero_entries"), "status_counts": dict(sorted(totals.items())), "deltas": rows,
    }


def confirm_vcs_dispatcher(candidates: dict[str, Any], vcs: dict[str, Any], table: dict[str, Any], workspace: Path) -> dict[str, Any]:
    address, process_address = "0x0005e5e8", "0x0005e4a4"
    dispatcher = next((item for item in vcs["functions"] if item.get("address") == address), None)
    process = next((item for item in vcs["functions"] if item.get("address") == process_address), None)
    if dispatcher is None or process is None:
        raise ValueError("VCS dispatcher or its confirmed running-script caller is absent from the export")
    if table.get("dispatcher") != address or not table.get("entries"):
        raise ValueError("VCS opcode dispatch-table export does not identify the confirmed dispatcher")
    expected_first, expected_last, expected_slots = 0x0000, 0x055A, 0x055B
    if (
        int(table.get("opcode_first", -1)) != expected_first
        or int(table.get("opcode_last", -1)) != expected_last
        or int(table.get("opcode_count_scanned", 0)) != expected_slots
        or len(table.get("slots", [])) != expected_slots
    ):
        raise ValueError(
            "VCS PSP opcode dispatch-table export must cover exactly 0x0000..0x055A "
            "(1371 slots)"
        )
    review_dir = workspace / "script_vm_matcher" / "artifacts" / "review" / "vcs_psp"
    review, caller_review = review_dir / "0005e5e8_FUN_0005e5e8.c", review_dir / "0005e4a4_FUN_0005e4a4.c"
    for path in (review, caller_review):
        if not path.exists():
            raise ValueError(f"Dispatcher review artifact required before confirmation: {path}")
    dispatcher_text = review.read_text(encoding="utf-8", errors="replace")
    caller_text = caller_review.read_text(encoding="utf-8", errors="replace")
    required_dispatcher = ("_DAT_ffff3534 + *(int *)(param_1 + 0x10)", "+ 2", "0x8000", "0x7fff", "PTR_LAB_00380ab4", "(*pcVar4)(param_1)")
    if not all(token in dispatcher_text for token in required_dispatcher):
        raise ValueError("VCS dispatcher review artifact no longer contains all validated semantic landmarks")
    if "FUN_0005e5e8(param_1)" not in caller_text or "while (iVar2 == 0)" not in caller_text:
        raise ValueError("VCS running-script caller review does not prove ProcessOneCommand return-loop semantics")
    evidence = [
        {"kind": "MULTI_SIGNAL_HIGH", "fact": "reads current Script IP at +0x10 through ScriptSpace base 0xffff3534 and fetches a 16-bit opcode"},
        {"kind": "CALLGRAPH_CONFIRMED", "fact": "increments IP by two; records NOT from 0x8000; strips with 0x7fff; indexes opcode×8 dispatch records; invokes selected handler"},
        {"kind": "CALLGRAPH_CONFIRMED", "fact": "CRunningScript::Process at 0x0005e4a4 calls dispatcher repeatedly until returned status is non-zero"},
        {"kind": "EXACT_DISPATCH_TABLE", "fact": f"{table.get('nonzero_entries')} nonzero records within {table.get('opcode_count_scanned')} VCS opcode slots"},
        {"kind": "MANUAL_DECOMPILATION_REVIEW", "artifact": str(review.relative_to(workspace)), "sha256": sha256(review)},
        {"kind": "MANUAL_DECOMPILATION_REVIEW", "artifact": str(caller_review.relative_to(workspace)), "sha256": sha256(caller_review)},
    ]
    confirmed = {
        "symbol": "CRunningScript::ProcessOneCommand", "vcs_psp": short_function(dispatcher), "confidence": "CALLGRAPH_CONFIRMED",
        "evidence": evidence, "automatic_rename": False, "dispatch_model": "data-driven descriptor/function-pointer table, not direct page calls",
        "related_confirmed_function": {"symbol": "CRunningScript::Process", "vcs_psp": short_function(process), "confidence": "CALLGRAPH_CONFIRMED"},
    }
    for row in candidates["matches"]:
        if row["anchor"]["symbol"] == "CRunningScript::ProcessOneCommand":
            row["status"] = "CONFIRMED_MANUAL_REVIEW"
            row["blocked_confirmation"] = None
            row["confirmed_vcs_psp"] = confirmed
            break
    else:
        raise ValueError("ProcessOneCommand missing from VCS candidate stage")
    candidates["status"] = "MANUAL_CONFIRMATION_PRESENT"
    candidates["confirmed_dispatcher"] = confirmed
    return confirmed


def mips_index(export: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {str(item.get("address", "")).lower(): item for item in export.get("functions", [])}


def vcs_stage(lcs_candidates: dict[str, Any], lcs: dict[str, Any], vcs: dict[str, Any], lcs_mips: dict[str, Any], vcs_mips: dict[str, Any]) -> dict[str, Any]:
    for label, export in (("LCS PSP", lcs), ("VCS PSP", vcs)):
        if export.get("sha256") != EXPECTED["lcs_psp" if label == "LCS PSP" else "vcs_psp"]:
            raise ValueError(f"{label} SHA-256 mismatch")
    if lcs_mips.get("sha256") != lcs.get("sha256") or vcs_mips.get("sha256") != vcs.get("sha256"):
        raise ValueError("Normalized MIPS export is not derived from the validated PSP input")
    lcs_by_address = {str(item.get("address", "")).lower(): item for item in lcs.get("functions", [])}
    vcs_targets = [item for item in vcs.get("functions", []) if int(item.get("instructions", 0) or 0) >= 4]
    lcs_sig, vcs_sig = mips_index(lcs_mips), mips_index(vcs_mips)
    rows = []
    for mapping in lcs_candidates["matches"]:
        seed_rows = []
        for local in mapping["candidates"][:3]:
            lcs_function = lcs_by_address[str(local["candidate"]["address"]).lower()]
            source_signature = lcs_sig.get(str(lcs_function["address"]).lower())
            def scorer(left: dict[str, Any], right: dict[str, Any]) -> tuple[float, list[dict[str, Any]], str]:
                return mips_score(left, right, source_signature, vcs_sig.get(str(right.get("address", "")).lower()))
            candidate_rows = ranked_candidates(lcs_function, vcs_targets, 8, scorer)
            seed_rows.append({
                "lcs_psp_candidate": local["candidate"], "lcs_psp_confidence": local["confidence"],
                "vcs_psp_candidates": candidate_rows,
            })
        rows.append({
            "anchor": mapping["android_anchor"], "status": "REVIEW_ONLY",
            "blocked_confirmation": "The LCS PSP side is not confirmed, so even a normalized-MIPS exact candidate cannot be renamed.",
            "lcs_seed_variants": seed_rows,
        })
    return {
        "schema": 1, "generated_at": now(), "stage": "LCS PSP -> VCS PSP", "status": "REVIEW_ONLY",
        "normalization": lcs_mips.get("normalization"),
        "inputs": {"lcs_psp_sha256": lcs.get("sha256"), "vcs_psp_sha256": vcs.get("sha256")},
        "matches": rows,
    }


def canonical_rw_name(symbol: str) -> str:
    return symbol.rstrip("_")


def scan_rw_symbols(root: Path) -> tuple[dict[str, set[str]], int]:
    found: dict[str, set[str]] = defaultdict(set)
    scanned = 0
    if not root.exists():
        return found, scanned
    extensions = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inl", ".txt", ".md"}
    for path in root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in extensions or ".git" in path.parts:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        scanned += 1
        rel = str(path.relative_to(root))
        for symbol in RW_SYMBOL.findall(text):
            found[canonical_rw_name(symbol)].add(rel)
    return found, scanned


def donor_corpus(external: Path, android: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any]]:
    repositories = []
    canonical: dict[str, dict[str, Any]] = {}
    for repo in sorted((item for item in external.iterdir() if item.is_dir()), key=lambda item: item.name.lower()):
        found, scanned = scan_rw_symbols(repo)
        signatures = sum(1 for item in repo.rglob("*") if item.is_file() and item.suffix.lower() in {".sig", ".pat", ".flirt"})
        repositories.append({"repository": repo.name, "path": str(repo), "commit": safe_git_rev(repo), "files_scanned": scanned, "rw_symbols": len(found), "flirt_assets": signatures})
        for symbol, locations in found.items():
            record = canonical.setdefault(symbol, {"symbol": symbol, "classification": "RENDERWARE", "sources": []})
            record["sources"].append({"repository": repo.name, "paths": sorted(locations)[:3]})
    android_rw = []
    for function in android.get("functions", []):
        text = qualified(function)
        for symbol in RW_SYMBOL.findall(text):
            android_rw.append({"symbol": canonical_rw_name(symbol), "address": function.get("address"), "qualified_name": text})
    corpus = {
        "schema": 1, "generated_at": now(), "scope": "Public generic RenderWare corpus; never direct PC FLIRT application to PSP MIPS.",
        "sources": repositories,
        "symbols": sorted(canonical.values(), key=lambda item: item["symbol"].lower()),
        "android_observations": android_rw,
        "ps2_donor": {"status": "NOT_SUPPLIED", "reason": "The supplied VCS PS2 artifacts are targets/auxiliary evidence, not a symbol-rich legal RenderWare donor."},
    }
    inventory = {"schema": 1, "generated_at": now(), "repositories": repositories, "policy": "Public sources only; no game executable, PDB or MAP download."}
    return corpus, inventory


def markdown_table(rows: list[list[str]], headers: list[str]) -> str:
    output = ["| " + " | ".join(headers) + " |", "| " + " | ".join("---" for _ in headers) + " |"]
    output.extend("| " + " | ".join(row) + " |" for row in rows)
    return "\n".join(output)


def reports(output: Path, source: dict[str, Any], exact: dict[str, Any], lcs_candidates: dict[str, Any], vcs_candidates: dict[str, Any], corpus: dict[str, Any], inventory: dict[str, Any]) -> None:
    report_dir = output / "reports"
    exact_rows = [[f"`{row['symbol']}`", f"`{row['android']['address']}`", row["role"], row["confidence"]] for row in exact["anchors"]]
    write_text(report_dir / "ANDROID_EXACT_SCRIPT_VM_SYMBOLS.md", "# Android LCS exact Script VM symbols\n\n" +
        f"Validated **{len(exact_rows)}** address + namespace + C++ symbol anchors against SHA-256 `{exact['input']['sha256']}`. Source is reLCS commit `{source['oracle']['commit'] or 'snapshot (commit metadata unavailable)'}`; `ReLCS.dll` is not an input.\n\n" +
        markdown_table(exact_rows, ["Symbol", "Android address", "Role", "Confidence"]) + "\n\nNo Ghidra names were changed.\n")
    lcs_rows = []
    for row in lcs_candidates["matches"]:
        best = row["candidates"][0]
        disposition = "confirmed by manual semantic review" if best.get("manual_confirmation") else "manual validation required"
        lcs_rows.append([f"`{row['android_anchor']['symbol']}`", f"`{best['candidate']['address']}`", str(best["heuristic_score"]), best["confidence"], disposition])
    write_text(report_dir / "LCS_PSP_SCRIPT_VM_MATCHES.md", "# LCS PSP Script VM candidates\n\nThese are bounded review candidates derived from exact Android anchors. None is a symbol mapping or an automatic rename.\n\n" + markdown_table(lcs_rows, ["Android exact anchor", "Best LCS PSP candidate", "Score", "Confidence", "Disposition"]) + "\n")
    vcs_rows = []
    for row in vcs_candidates["matches"]:
        confirmed = row.get("confirmed_vcs_psp")
        if confirmed:
            vcs_rows.append([f"`{row['anchor']['symbol']}`", "`0x00153a90`", f"`{confirmed['vcs_psp']['address']}`", confirmed["confidence"], "confirmed by manual semantic review"])
        else:
            first_seed = row["lcs_seed_variants"][0]
            best = first_seed["vcs_psp_candidates"][0]
            vcs_rows.append([f"`{row['anchor']['symbol']}`", f"`{first_seed['lcs_psp_candidate']['address']}`", f"`{best['candidate']['address']}`", best["confidence"], "review only"])
    write_text(report_dir / "VCS_PSP_SCRIPT_VM_MATCHES.md", "# VCS PSP Script VM candidates\n\n`CRunningScript::ProcessOneCommand` is confirmed at `0x0005e5e8` through the instruction-level fetch/NOT/table-dispatch sequence and its caller's return-status loop. VCS uses a data-driven table rather than direct LCS-style page callees.\n\n" + markdown_table(vcs_rows, ["Android exact anchor", "LCS PSP anchor", "VCS PSP candidate", "Confidence", "Disposition"]) + "\n")
    proof_rows = [
        ["1", "read IP", "`*(int *)(param_1 + 0x10)` selects the active bytecode offset"],
        ["2", "derive/fetch", "`*(short *)(_DAT_ffff3534 + IP)` reads the 16-bit opcode from ScriptSpace"],
        ["3", "advance", "`*(int *)(param_1 + 0x10) = ... + 2` advances the IP by one opcode"],
        ["4", "NOT bit", "`uVar3 & 0x8000` writes the flag to `param_1 + 0x20c`"],
        ["5", "strip NOT", "`uVar3 = uVar3 & 0x7fff` produces the dispatch index"],
        ["6", "dispatch", "`DAT_00380ab0 + opcode * 8` supplies descriptor/offset; `PTR_LAB_00380ab4[opcode * 2]` supplies the handler; `(*pcVar4)(param_1)` calls it"],
        ["7", "VM dispatch status", "the handler's `v0` status is propagated to the caller; `0x0005e4a4` executes `do { iVar2 = FUN_0005e5e8(param_1); } while (iVar2 == 0)`. Thus `0` is `CONTINUE` for the interpreter loop, not an SCM output; non-zero stops the current Process pass."],
    ]
    write_text(report_dir / "VCS_PSP_DISPATCHER_SEMANTIC_PROOF.md", "# VCS PSP ProcessOneCommand semantic proof\n\nStatus: `CALLGRAPH_CONFIRMED`; no Ghidra label was written automatically. This proof is from the manual Ghidra decompilation of `0x0005e5e8` and its caller `0x0005e4a4`.\n\n" + markdown_table(proof_rows, ["Step", "Semantic", "Manual decompilation evidence"]) + "\n\nVCS is not an LCS-style direct page-call dispatcher: it uses a table of eight-byte records plus indirect targets. `vcs_dispatcher_propagation.json` therefore exposes roles/ranges only, with `automatic_rename: false`.\n")
    validation = [
        "# Script VM match validation", "",
        "## Result", "", "No automatic Ghidra rename was made. reLCS source → LCS Android is exact; LCS PSP `ProcessOneCommand` is manually confirmed at `0x00153a90`; VCS PSP `ProcessOneCommand` is manually confirmed at `0x0005e5e8` using the full Script-IP → opcode → NOT → table dispatch → return-status sequence.", "",
        "## Input hashes", "",
        markdown_table([[key, f"`{value}`"] for key, value in EXPECTED.items()], ["Input", "SHA-256"]), "",
        "## Policy", "",
        "- `EXACT_SYMBOL` anchors must agree on address, namespace and name.",
        "- Android → LCS PSP structural/p-code metrics produce `REVIEW` only.",
        "- LCS PSP → VCS PSP uses relocation-neutral MIPS hashes, opcode profile, CFG/data/call measurements. A MIPS hash alone is not enough without an independently confirmed LCS semantic anchor.",
        "- Old fuzzy four-way results are preserved separately as `UNCONFIRMED_REVIEW`.",
    ]
    write_text(report_dir / "SCRIPT_VM_MATCH_VALIDATION.md", "\n".join(validation) + "\n")
    donor_rows = [[row["repository"], str(row["rw_symbols"]), str(row["flirt_assets"]), (row["commit"] or "unavailable")[:12]] for row in inventory["repositories"]]
    write_text(report_dir / "RENDERWARE_DONOR_INVENTORY.md", "# RenderWare donor inventory\n\n" + markdown_table(donor_rows, ["Public repository", "RW symbols", "FLIRT assets", "Commit"]) + "\n\nThe supplied PS2 content is recorded as a target, not treated as a generic donor.\n")
    write_text(report_dir / "RENDERWARE_SYMBOL_CORPUS_STATUS.md", "# RenderWare symbol corpus status\n\n" +
        f"Canonical public corpus: **{len(corpus['symbols'])}** generic RenderWare identifiers; Android observations: **{len(corpus['android_observations'])}**.\n\n" +
        "PC FLIRT files are inventoried as PC-side evidence only and are never applied directly to PSP MIPS. PSP annotations require a matching cross-ISA or same-ISA evidence chain.\n")
    actions = [
        "# Next 20 actions", "",
        "1. Treat the eight-byte VCS SCM header as outside ScriptSpace in every loader, route trace and control-flow target calculation.",
        "2. Preserve `VCS_PSP_MAIN_SCM_ENTRY_ROUTE.json` as one observed portable-VM route, not PPSSPP telemetry or a whole-program CFG claim.",
        "3. Preserve the confirmed selector-zero table parse, scoped source→active-buffer transfer and StartNewScript idle→active factory contract; next recover mission-mode state and lower I/O/finalizer semantics before designing PC glue.",
        "4. Do not collapse `0289` into `LaunchMission(id)`; preserve stream/copy, factory, target flags, mission-active/global-clear state and re-entrant Process scheduling as one subsystem contract.",
        "5. Treat the read-only selector-zero transfer as a host-contract proof: the real first child executes SCRIPT_NAME then reaches 013F CREATE_PICKUP, whose model-object/terrain/pickup adapters are the next boundary.",
        "6. Preserve the bounded static 0289 literal cluster `0..9` (duplicate 8) as a census order only; next run each selected child through the read-only host contract without claiming reachability.",
        "7. Use PPSSPP only to capture naturally reached `0289` behavior after a safe save-state route is available; do not inject bytecode or patch game memory.",
        "8. Extend the route harness with another entry/thread only after proving its initial ScriptSpace base and header/container convention.",
        "9. Use confirmed VCS PSP `ReadMultiScriptFileOffsetsFromScript` (`0x0006041C`) to parse a PC mission-offset table with its direct metadata layout; next prove table bounds/sentinel behavior from VCS assets and consumers.",
        "10. Keep `CTheScripts::StartNewScript` as a direct scheduler contract (idle removal, process ID, Init, IP, active insertion and `+0x208`) and preserve stream scope enter/leave; next prove only mission mode plus lower I/O/finalizer roles.",
        "11. Preserve the read-only 0..9 child census; prioritize direct PSP/PS2 proof of furthest wall 01E7, then 00A9/0145/01AA/0296/03C6/01EA. Do not turn known 0045 into a synthetic predicate.",
        "12. Continue per-handler work only when a reached route, a host boundary, or a high-frequency real-SCM opcode supplies the prioritization evidence.",
        "13. Use `0113` as the vehicle-release-mark bridge only; do not merge it with destructive `0049` or propagate public SCM wording to internal callees.",
        "14. Re-run bounded static coverage after every evidence status change and keep coupled host boundaries in the explicit-fault category.",
        "15. Validate real route IPs against native coordinates before interpreting adjacent bytes as code or data.",
        "16. Add confirmed symbols only as a byproduct of instruction-level semantic proof; never promote structural table equivalence or a fuzzy match.",
        "17. Keep PSP runtime captures paired with PS2 static evidence and state their input-path limits explicitly.",
        "18. Maintain a no-game-payload handoff: source, reports, static addresses, route metadata and traces only.",
        "19. Rebuild `vcs_vm_core`, run CTest and regenerate route/coverage artifacts before every handoff.",
        "20. Export a new ZIP only after rebuilding and checking its manifest/archive for game executables, ISOs, APKs, OBBs and raw SCM payloads.",
    ]
    write_text(output / "NEXT_20_ACTIONS.md", "\n".join(actions) + "\n")


def outputs(output: Path, source: dict[str, Any], exact: dict[str, Any], lcs_candidates: dict[str, Any], vcs_candidates: dict[str, Any], corpus: dict[str, Any], inventory: dict[str, Any], propagation: dict[str, Any], delta: dict[str, Any]) -> None:
    write_json(MATCHER / "artifacts" / "relcs_source_semantics.json", source)
    write_json(output / "android_lcs_exact_symbols.json", exact)
    write_json(output / "lcs_psp_script_vm_candidates.json", lcs_candidates)
    lcs_confirmed_rows = []
    review_rows = []
    for row in lcs_candidates["matches"]:
        top = row["candidates"][0]
        if top.get("manual_confirmation"):
            lcs_confirmed_rows.append({"android_anchor": row["android_anchor"], "lcs_psp": top["candidate"], "confidence": top["confidence"], "evidence": top["evidence"], "automatic_rename": False})
        else:
            review_rows.append(row["android_anchor"])
    lcs_confirmed = {"schema": 1, "generated_at": now(), "status": "MANUAL_CONFIRMATION_PRESENT", "confirmed": lcs_confirmed_rows, "unmatched_or_review": review_rows, "policy": POLICY}
    write_json(output / "lcs_psp_script_vm_confirmed.json", lcs_confirmed)
    write_json(output / "vcs_psp_script_vm_candidates.json", vcs_candidates)
    vcs_confirmed = {"schema": 1, "generated_at": now(), "status": "MANUAL_CONFIRMATION_PRESENT", "confirmed": [vcs_candidates["confirmed_dispatcher"]], "propagation": {"file": "vcs_dispatcher_propagation.json", "mode": propagation["dispatch_model"], "note": "VCS has indirect table handlers, not direct ProcessCommands page callees."}, "policy": POLICY}
    write_json(output / "vcs_psp_script_vm_confirmed.json", vcs_confirmed)
    map_rows = []
    vcs_by_symbol = {row["anchor"]["symbol"]: row for row in vcs_candidates["matches"]}
    for row in lcs_candidates["matches"]:
        symbol = row["android_anchor"]["symbol"]
        vcs_row = vcs_by_symbol[symbol]
        confirmed = vcs_row.get("confirmed_vcs_psp")
        map_rows.append({"symbol": symbol, "lcs_android": row["android_anchor"], "lcs_psp": row["candidates"][:3], "vcs_psp": confirmed if confirmed else vcs_row["lcs_seed_variants"][:1], "status": "CONFIRMED_MANUAL_REVIEW" if confirmed else "REVIEW_ONLY"})
    write_json(output / "stories_vm_map.json", {"schema": 1, "generated_at": now(), "policy": POLICY, "mappings": map_rows})
    write_json(output / "vcs_opcode_delta.json", delta)
    csv_lines = ["opcode,lcs_source_command,vcs_target,vcs_target_function,vcs_descriptor,vcs_local_offset,vcs_selector,status"]
    for row in delta["deltas"]:
        csv_lines.append(",".join(str(row.get(key) if row.get(key) is not None else "") for key in ("opcode", "lcs_source_command", "vcs_target", "vcs_target_function", "vcs_descriptor", "vcs_local_offset", "vcs_selector", "status")))
    write_text(output / "vcs_opcode_delta.csv", "\n".join(csv_lines) + "\n")
    write_text(output / "vcs_opcode_delta.md", "# VCS opcode delta\n\n" +
        "Status: `STRUCTURAL_DISPATCH_DELTA_COMPLETE`. VCS uses a table-driven dispatcher, so this records actual table slots, target addresses and descriptor fields. Same-number reLCS command names are navigation references only — not semantic names propagated to VCS handlers.\n\n" +
        markdown_table([[key, str(value)] for key, value in delta["status_counts"].items()], ["Status", "Slots"]) + "\n")
    write_json(output / "vcs_dispatcher_propagation.json", propagation)
    corpus_path = MATCHER / "renderware_symbols" / "canonical_rw_symbols.json"
    write_json(corpus_path, corpus)
    write_json(MATCHER / "renderware_symbols" / "donor_inventory.json", inventory)
    reports(output, source, exact, lcs_candidates, vcs_candidates, corpus, inventory)
    agent = {
        "schema": 1, "generated_at": now(), "purpose": "handoff context for a human/agent validating Script VM candidates",
        "confirmed_stage": "reLCS source -> LCS Android; LCS PSP dispatcher; VCS PSP dispatcher; VCS PSP/PS2 opcode-table comparison", "exact_android_symbols": len(exact["anchors"]),
        "lcs_psp_status": lcs_candidates["status"], "vcs_psp_status": vcs_candidates["status"],
        "cross_platform_opcode_tables": {
            "psp": "VCS_PSP_OPCODE_TABLE.json", "ps2": "VCS_PS2_HANDLER_DB.json",
            "comparison": "VCS_PSP_PS2_OPCODE_COMPARE.json", "manual_semantic_proof": "VCS_PSP_PS2_MANUAL_SEMANTIC_PROOF.json",
            "common_semantic_shape": "VCS_PSP_PS2_COMMON_SEMANTICS.json",
            "common_semantic_signals": ["parameter decoder calls", "thread offsets", "global access pattern", "constants", "normalized callees", "CFG", "native handler-status constants (interpreter ABI, not SCM outputs)"],
            "counts": {"COMMON_SLOT": 1328, "PSP_ONLY": 0, "PS2_ONLY": 10, "DIFFERENT_HANDLER_SEMANTICS": 0},
        },
        "runtime_oracle": {
            "directory": "tools/ppsspp_vcs_oracle",
            "mode": "stock PPSSPP debugger WebSocket; natural execution only; no EBOOT, game-memory or bytecode patching",
            "initial_proof_opcodes": ["0x0000", "0x0001", "0x0002", "0x0025", "0x0026", "0x037A", "0x037B", "0x055A"],
            "captures": ["raw/normalized opcode and NOT bit", "runtime handler selected from the descriptor table", "input parameter bytes", "IP and ScriptThread deltas", "PPSSPP write-hit tracepoints for relevant ScriptThread fields", "v0 native VM dispatch status (not an SCM output)", "configured memory-region deltas"],
            "bridge": "tools/ppsspp_vcs_oracle/telemetry/psp_runtime_vs_ps2_static.json",
            "runtime_status": "tools/ppsspp_vcs_oracle/telemetry/ORACLE_STATUS.json",
            "canonical_semantics": "VCS_PSP_INITIAL_OPCODE_CANONICAL_SEMANTICS.json",
            "evidence_limit": "A runtime capture proves only its naturally reached input path; it is paired with PS2 static evidence and never asserts cross-platform equality automatically.",
        },
        "reconstructed_core": {
            "directory": "vcs_vm_core",
            "coverage": "vcs_vm_core/evidence_coverage.json",
            "scope": "Portable C++17 semantic core: confirmed 16-bit fetch/IP advance/NOT-bit dispatch; control-flow spine including adapter-gated 0024 START_NEW_SCRIPT (one script index plus 96 child locals), 0025 GOSUB, 01BA two-parameter tagless GOSUB and 0238 fixed eight-byte script-name write; raw-dword assignment aliases 0004/0005/0006/0035/0036/0037/02E2; scalar arithmetic 0007..000E plus direct-target dispatch aliases 002A..0030 and integer-add alias 0029; time-step-adapter-gated timed float arithmetic 0031..0034; direct numeric conversions/absolute values 0038..003B; direct PSP/PS2 integer and float condition families 000F..001E plus 02DB, 009F character-dead condition, 00A5 player-state-code-two condition and direct active-thread-marker condition 04F2; adapter-gated 0044 character-coordinate placement, 0045 character-liveness condition, 0049 vehicle-delete bridge, 0059 print-clear bridge, 04E5 global-byte-store bridge, 0079 mission-launch request, 007F low-16-bit input predicate, 00C8/00C9/00CA fade operations, 0107 player-control bridge, 0113 vehicle-release-mark bridge, 013F/01F9 pickup construction bridges, 0023 scheduler-termination bridge, 0203 text queue bridge, 0268 help-clear bridge and 0401 pause-menu boolean bridge; 010E game-clock output and 0377 pad state output through StoreParameters; CALL and condition reducer contracts. VCS plural tag 0x00 is a zero-value case but its standalone next-parameter primitive routes tag 0x00 through the local resolver; reLCS ARGUMENT_END semantics are not transplanted. Proven `0289` mission streaming stays outside the core: its container parser, scoped source→active-buffer transfer and `CTheScripts::StartNewScript` idle→active factory are direct, but mission mode, lower I/O/finalization semantics and native scheduler ownership remain host work. Integer division rejects native hardware edge cases rather than inventing a portable quotient.",
            "parameter_abi_proof": "reports/VCS_PSP_PARAMETER_ABI_PROOF.md",
            "call_return_proof": "reports/VCS_PSP_CALL_RETURN_ABI_PROOF.md",
            "gosub_return_proof": "reports/VCS_PSP_GOSUB_RETURN_PROOF.md",
            "gosub_file_vm_proof": "reports/VCS_PSP_GOSUB_FILE_VM_PROOF.md",
            "condition_vm_proof": "reports/VCS_PSP_CONDITION_VM_PROOF.md",
            "integer_condition_vm_proof": "reports/VCS_PSP_INTEGER_CONDITION_VM_PROOF.md",
            "float_condition_vm_proof": "reports/VCS_PSP_FLOAT_CONDITION_VM_PROOF.md",
            "vehicle_release_mark_vm_proof": "reports/VCS_PSP_VEHICLE_RELEASE_MARK_VM_PROOF.md",
            "create_pickup_vm_proof": "reports/VCS_PSP_CREATE_PICKUP_VM_PROOF.md",
            "terminate_script_vm_proof": "reports/VCS_PSP_TERMINATE_SCRIPT_VM_PROOF.md",
            "char_liveness_vm_proof": "reports/VCS_PSP_IS_CHAR_STILL_ALIVE_VM_PROOF.md",
            "delete_vehicle_vm_proof": "reports/VCS_PSP_DELETE_VEHICLE_VM_PROOF.md",
            "global_byte_store_vm_proof": "reports/VCS_PSP_GLOBAL_BYTE_STORE_VM_PROOF.md",
            "player_control_vm_proof": "reports/VCS_PSP_PLAYER_CONTROL_VM_PROOF.md",
            "character_coordinate_vm_proof": "reports/VCS_PSP_CHARACTER_COORDINATE_VM_PROOF.md",
            "player_state_code_two_condition_vm_proof": "reports/VCS_PSP_PLAYER_STATE_CODE_TWO_CONDITION_VM_PROOF.md",
            "clear_prints_vm_proof": "reports/VCS_PSP_CLEAR_PRINTS_VM_PROOF.md",
            "character_dead_condition_vm_proof": "reports/VCS_PSP_CHARACTER_DEAD_CONDITION_VM_PROOF.md",
            "fade_vm_proof": "reports/VCS_PSP_FADE_VM_PROOF.md",
            "game_timer_vm_proof": "reports/VCS_PSP_GAME_TIMER_VM_PROOF.md",
            "pad_state_vm_proof": "reports/VCS_PSP_PAD_STATE_VM_PROOF.md",
            "launch_mission_vm_proof": "reports/VCS_PSP_LAUNCH_MISSION_VM_PROOF.md",
            "button_pressed_vm_proof": "reports/VCS_PSP_BUTTON_PRESSED_VM_PROOF.md",
            "script_name_vm_proof": "reports/VCS_PSP_SCRIPT_NAME_VM_PROOF.md",
            "mission_stream_launch_boundary": "reports/VCS_PSP_MISSION_STREAM_LAUNCH_BOUNDARY.md",
            "main_scm_mission_selector_static_cluster": "reports/VCS_PSP_MAIN_SCM_MISSION_SELECTOR_STATIC_CLUSTER.md",
            "main_scm_mission_child_census": "reports/VCS_PSP_MAIN_SCM_MISSION_CHILD_CENSUS.md",
            "mission_offset_table_init_proof": "reports/VCS_PSP_MISSION_OFFSET_TABLE_INIT_PROOF.md",
            "start_new_script_factory_proof": "reports/VCS_PSP_START_NEW_SCRIPT_FACTORY_PROOF.md",
            "mission_thread_flag_vm_proof": "reports/VCS_PSP_MISSION_THREAD_FLAG_VM_PROOF.md",
            "control_flow_vm_proof": "reports/VCS_PSP_CONTROL_FLOW_VM_PROOF.md",
            "variable_ops_proof": "reports/VCS_PSP_VARIABLE_OPS_PROOF.md",
            "arithmetic_vm_proof": "reports/VCS_PSP_ARITHMETIC_VM_PROOF.md",
            "start_new_script_vm_proof": "reports/VCS_PSP_START_NEW_SCRIPT_VM_PROOF.md",
            "numeric_vm_proof": "reports/VCS_PSP_NUMERIC_VM_PROOF.md",
            "timed_arithmetic_vm_proof": "reports/VCS_PSP_TIMED_ARITHMETIC_VM_PROOF.md",
            "main_scm_static_inventory": "VCS_PSP_MAIN_SCM_OPCODE_USAGE.json",
            "main_scm_prefix_vertical_slice": "reports/VCS_PSP_MAIN_SCM_PREFIX_VERTICAL_SLICE.md",
            "main_scm_observed_portable_route": "VCS_PSP_MAIN_SCM_ENTRY_ROUTE.json",
            "main_scm_observed_portable_route_report": "reports/VCS_PSP_MAIN_SCM_ENTRY_ROUTE.md",
            "main_scm_mission_stream_trace": "reports/VCS_PSP_MAIN_SCM_MISSION_STREAM_TRACE.jsonl",
            "pause_menu_vm_proof": "reports/VCS_PSP_DISABLE_PAUSE_MENU_VM_PROOF.md",
            "display_text_vm_proof": "reports/VCS_PSP_DISPLAY_TEXT_VM_PROOF.md",
            "clear_help_vm_proof": "reports/VCS_PSP_CLEAR_HELP_VM_PROOF.md",
            "confirmed_symbol_map": "script_vm_matcher/artifacts/exports/vcs_psp_confirmed_vm_symbols.json",
            "guardrails": "All unimplemented slots fault explicitly. 055A reducer is exact, but its unnamed engine predicate remains host-provided. GOSUB tagless frames must not be conflated with CALL/CALL_NOT output frames.",
        },
        "rules": ["Do not rename REVIEW candidates.", "Add a confirmed symbol only as a byproduct of direct semantic proof; fuzzy matching cannot substitute for VM analysis.", "VCS opcode-table slots are structural navigation evidence, not command-name equivalence.", "COMMON_SLOT means both tables contain a descriptor; it does not prove equal handler semantics.", "COMMON semantic-shape scores are triage only, including when they are low.", "Runtime telemetry records actual PSP behavior only for the captured path; do not treat it as automatic PSP/PS2 equivalence.", "Use decompiler + callers/callees to prove individual handler semantics; use DIFFERENT_HANDLER_SEMANTICS only after manual proof."],
    }
    write_json(output / "agents" / "generated" / "stories_vm_research_context.json", agent)
    write_text(output / "agents" / "generated" / "stories_vm_research_context.md", "# Stories VM research context\n\n" +
        f"Exact Android anchors: **{len(exact['anchors'])}**. `ProcessOneCommand` is confirmed at LCS PSP `0x00153a90` and VCS PSP `0x0005e5e8`. The VCS PSP table is fixed at `0x0000..0x055a` (1371 slots); its PS2 comparison is recorded in `VCS_PSP_PS2_OPCODE_COMPARE.json` (1328 `COMMON_SLOT`, 10 `PS2_ONLY`). `vcs_vm_core/` reconstructs the confirmed dispatcher, control-flow spine, adapter-gated `0024 START_NEW_SCRIPT` (one index plus 96 child locals), `0025` and two-parameter tagless `01BA` GOSUB, and `0238` fixed eight-byte script-name write; VM-only raw-dword assignments through exact `0037` alias, scalar arithmetic `0007..000E`, exact-target aliases `002A..0030`, time-step-adapter-gated timed float arithmetic `0031..0034`, direct PSP/PS2 integer/float condition families `000F..001E` plus `02DB`, adapter-gated `00A5` player-state-code-two condition and direct active-thread marker `04F2`, engine-probe-gated `0045` character liveness, adapter-gated `0044` character-coordinate placement, `0049` vehicle-delete, `0059` print-clear, `04E5` global-byte-store and `0113` vehicle-release-mark bridges, `0079` mission-launch request, `007F` low-16-bit input predicate and `0107` player-control bridge, adapter-gated fade `00C8/00C9/00CA`, `013F/01F9` pickup construction and `0023` scheduler-termination bridges, `0203` text queue bridge, `0268` help-clear bridge and `0401` pause-menu boolean bridge, `010E` raw game-clock output, and `0377` input selector output. The VCS plural collector maps tag `0x00` to raw zero while the standalone next-parameter decoder uses its low-variable resolver path, so reLCS ARGUMENT_END semantics are not imported. The proven `0289` mission-streaming opcode is registered only as a selector-decoding explicit fault until its remaining source/buffer/finalization/re-entrant Process host subsystem is reconstructed. Its `CTheScripts::StartNewScript` idle-to-active factory is now direct on PSP and PS2, including Init and list primitives; scheduler ownership remains host work. The arithmetic aliases deliberately use generic names pending source-kind spelling proof. Integer division rejects divisor-zero and `INT_MIN/-1` rather than claiming an invented portable hardware quotient. The SCM inventory has a direct `0024` width override (1+96 rather than the Sanny pseudo-arity) and its 1,893-command result is static, linear evidence only. A read-only route harness strips the eight-byte SCM header before mapping VM IPs, executes the true five-command entry prefix through `0006 SET_VAR_TEXT_LABEL`, and captures a 216-command control-flow route terminating at the specific `0289` host-subsystem boundary. This is an integration proof, not PPSSPP telemetry or a whole-program reachability claim. Confirmed roles are listed in `script_vm_matcher/artifacts/exports/vcs_psp_confirmed_vm_symbols.json`. The runtime PSP oracle captures only naturally executed paths and does not promote an uncaptured input path to cross-platform equivalence.\n")


def parser() -> argparse.ArgumentParser:
    value = argparse.ArgumentParser(description=__doc__)
    value.add_argument("--workspace", type=Path, default=WORKSPACE)
    value.add_argument("--source", type=Path, default=WORKSPACE / "research_sources" / "reLCS_origin_lcs")
    value.add_argument("--anchors", type=Path, default=MATCHER / "config" / "android_lcs_script_vm_anchors.json")
    value.add_argument("--android", type=Path, default=MATCHER / "artifacts" / "exports" / "lcs_android.json")
    value.add_argument("--lcs-psp", type=Path, default=MATCHER / "artifacts" / "exports" / "lcs_psp.json")
    value.add_argument("--vcs-psp", type=Path, default=MATCHER / "artifacts" / "exports" / "vcs_psp.json")
    value.add_argument("--lcs-mips", type=Path, default=MATCHER / "artifacts" / "exports" / "lcs_psp_mips_normalized.json")
    value.add_argument("--vcs-mips", type=Path, default=MATCHER / "artifacts" / "exports" / "vcs_psp_mips_normalized.json")
    value.add_argument("--vcs-dispatch-table", type=Path, default=MATCHER / "artifacts" / "exports" / "vcs_psp_opcode_dispatch_table.json")
    value.add_argument("--external", type=Path, default=WORKSPACE / "research_feed" / "external")
    return value


def main() -> int:
    args = parser().parse_args()
    source, source_by_symbol = source_records(args.source)
    anchors = read_json(args.anchors)
    android, lcs, vcs = read_json(args.android), read_json(args.lcs_psp), read_json(args.vcs_psp)
    exact = validate_android(anchors, android, source_by_symbol)
    lcs_candidates = candidate_stage(exact, lcs)
    confirm_lcs_dispatcher(lcs_candidates, lcs, args.workspace)
    vcs_candidates = vcs_stage(lcs_candidates, lcs, vcs, read_json(args.lcs_mips), read_json(args.vcs_mips))
    table = read_json(args.vcs_dispatch_table)
    confirm_vcs_dispatcher(vcs_candidates, vcs, table, args.workspace)
    propagation = build_dispatch_propagation(table)
    delta = build_vcs_opcode_delta(args.source, table)
    corpus, inventory = donor_corpus(args.external, android)
    outputs(args.workspace, source, exact, lcs_candidates, vcs_candidates, corpus, inventory, propagation, delta)
    print(f"Validated {len(exact['anchors'])} Android exact anchors; confirmed LCS/VCS ProcessOneCommand and wrote opcode-table delta to {args.workspace}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
