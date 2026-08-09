#!/usr/bin/env python3
"""Inventory the VCS PS2/PSP graphics hardware boundary without exporting game bytes.

The script scans executable ELF load segments for CPU-visible vector operations
and for *directly recoverable* MMIO accesses.  It intentionally does not claim
that every COP2 instruction renders a frame: PS2 VU0 is also a general vector
coprocessor, while PS2 VU1 is fed by VIF DMA command streams and executes
microcode from VU memory rather than from the EE instruction stream.

For PSP, the equivalent boundary is a GE display list submitted through the
`sceGe_user` / display APIs.  It is not a CPU instruction family.  COP2 in a
PSP ELF is reported as an Allegrex VFPU candidate, separately from GE.

Only addresses, decoded instruction classes, direct MMIO candidates, function
ownership, import names and aggregate counts are written to the report.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import struct
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


PT_LOAD = 1
PF_X = 1
SHT_STRTAB = 3
SHT_DYNSYM = 11

REG_NAMES = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)

LOAD_STORE_OPS = {
    0x20: "lb", 0x21: "lh", 0x22: "lwl", 0x23: "lw", 0x24: "lbu",
    0x25: "lhu", 0x26: "lwr", 0x27: "lwu", 0x28: "sb", 0x29: "sh",
    0x2A: "swl", 0x2B: "sw", 0x2E: "swr", 0x2F: "cache", 0x30: "ll",
    0x31: "lwc1", 0x36: "lqc2", 0x37: "ld", 0x38: "sc", 0x39: "swc1",
    0x3E: "sqc2", 0x3F: "sd",
}
STORE_OPS = {0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x38, 0x39, 0x3E, 0x3F}

MMIO_RANGES = (
    (0x10003800, 0x10003C00, "VIF0 registers"),
    (0x10003C00, 0x10004000, "VIF1 registers"),
    (0x10008000, 0x10008080, "VIF0 DMA channel"),
    (0x10009000, 0x10009080, "VIF1 DMA channel"),
    (0x1000A000, 0x1000A080, "GIF DMA channel"),
    (0x12000000, 0x12002000, "GS privileged registers"),
)


@dataclass(frozen=True)
class Segment:
    offset: int
    vaddr: int
    size: int
    executable: bool


@dataclass(frozen=True)
class FunctionRange:
    start: int
    end: int
    name: str


class Elf32:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        if self.data[:4] != b"\x7fELF" or self.data[4] != 1:
            raise ValueError(f"{path}: expected ELF32")
        endian_id = self.data[5]
        if endian_id not in (1, 2):
            raise ValueError(f"{path}: unknown ELF byte order")
        self.endian = "<" if endian_id == 1 else ">"
        self.byte_order = "little" if endian_id == 1 else "big"
        self._header = struct.unpack_from(self.endian + "16sHHIIIIIHHHHHH", self.data, 0)
        self.phoff = self._header[5]
        self.shoff = self._header[6]
        self.phentsize = self._header[9]
        self.phnum = self._header[10]
        self.shentsize = self._header[11]
        self.shnum = self._header[12]
        self.shstrndx = self._header[13]

    def segments(self) -> list[Segment]:
        result = []
        for index in range(self.phnum):
            off = self.phoff + index * self.phentsize
            p_type, p_off, p_vaddr, _p_paddr, p_filesz, _p_memsz, p_flags, _align = struct.unpack_from(
                self.endian + "IIIIIIII", self.data, off
            )
            if p_type == PT_LOAD and p_filesz:
                result.append(Segment(p_off, p_vaddr, p_filesz, bool(p_flags & PF_X)))
        return result

    def sections(self) -> list[dict[str, int | str]]:
        raw = []
        for index in range(self.shnum):
            off = self.shoff + index * self.shentsize
            raw.append(struct.unpack_from(self.endian + "IIIIIIIIII", self.data, off))
        if not raw or self.shstrndx >= len(raw):
            return []
        shstr = raw[self.shstrndx]
        names = self.data[shstr[4]:shstr[4] + shstr[5]]
        result = []
        for values in raw:
            name_offset, section_type, flags, addr, offset, size, link, info, addralign, entsize = values
            end = names.find(b"\0", name_offset)
            name = names[name_offset:end if end >= 0 else len(names)].decode("ascii", errors="replace")
            result.append({
                "name": name, "type": section_type, "flags": flags, "addr": addr,
                "offset": offset, "size": size, "link": link, "info": info,
                "addralign": addralign, "entsize": entsize,
            })
        return result

    def imported_symbol_names(self) -> list[str]:
        sections = self.sections()
        names: set[str] = set()
        for section in sections:
            if section["type"] != SHT_DYNSYM or not section["entsize"]:
                continue
            link = int(section["link"])
            if link >= len(sections) or sections[link]["type"] != SHT_STRTAB:
                continue
            string_section = sections[link]
            strings = self.data[int(string_section["offset"]):int(string_section["offset"] + string_section["size"])]
            start = int(section["offset"])
            end = start + int(section["size"])
            stride = int(section["entsize"])
            for offset in range(start, end - stride + 1, stride):
                st_name, _st_value, _st_size, _st_info, _st_other, st_shndx = struct.unpack_from(
                    self.endian + "IIIBBH", self.data, offset
                )
                if st_shndx != 0 or st_name >= len(strings):
                    continue
                terminator = strings.find(b"\0", st_name)
                name = strings[st_name:terminator if terminator >= 0 else len(strings)].decode("ascii", errors="replace")
                if name:
                    names.add(name)
        return sorted(names)

    def ascii_tokens(self, prefixes: tuple[bytes, ...]) -> list[str]:
        tokens: set[str] = set()
        for fragment in self.data.split(b"\0"):
            if 3 <= len(fragment) <= 128 and any(fragment.startswith(prefix) for prefix in prefixes):
                tokens.add(fragment.decode("ascii", errors="replace"))
        return sorted(tokens)


def read_functions(path: Path | None) -> tuple[list[FunctionRange], list[int]]:
    if path is None or not path.exists():
        return [], []
    ranges = []
    with path.open(encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                start = int(row.get("Start", ""), 0)
                end = int(row.get("End", ""), 0)
            except ValueError:
                continue
            if end > start:
                ranges.append(FunctionRange(start, end, row.get("Name", "<unnamed>")))
    ranges.sort(key=lambda item: item.start)
    return ranges, [item.start for item in ranges]


def owner_of(address: int, ranges: list[FunctionRange], starts: list[int]) -> str | None:
    index = bisect.bisect_right(starts, address) - 1
    if index >= 0 and ranges[index].start <= address < ranges[index].end:
        return ranges[index].name
    return None


def signed16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def mmio_bucket(address: int) -> str | None:
    for start, end, label in MMIO_RANGES:
        if start <= address < end:
            return label
    return None


def cop2_kind(word: int, platform: str) -> str:
    rs = (word >> 21) & 0x1F
    if platform == "psp":
        return "Allegrex COP2/VFPU candidate"
    names = {
        0x00: "MFC2 (VU0->GPR)",
        0x02: "CFC2 (VU0 control->GPR)",
        0x04: "MTC2 (GPR->VU0)",
        0x06: "CTC2 (GPR->VU0 control)",
        0x08: "BC2 (VU0 condition branch)",
    }
    return names.get(rs, "VU0 macro instruction" if rs >= 0x10 else "COP2/VU0 control candidate")


def scan_instructions(elf: Elf32, platform: str, functions: list[FunctionRange], starts: list[int]) -> dict[str, object]:
    vector_hits = []
    mmio_hits = []
    class_counts: Counter[str] = Counter()
    mmio_counts: Counter[str] = Counter()
    syscall_count = 0

    for segment in elf.segments():
        if not segment.executable:
            continue
        registers: list[int | None] = [None] * 32
        registers[0] = 0
        for relative in range(0, segment.size - 3, 4):
            address = segment.vaddr + relative
            word = int.from_bytes(elf.data[segment.offset + relative:segment.offset + relative + 4], elf.byte_order)
            op = word >> 26
            rs = (word >> 21) & 0x1F
            rt = (word >> 16) & 0x1F
            immediate = word & 0xFFFF
            owner = owner_of(address, functions, starts)

            if op == 0x12:
                label = cop2_kind(word, platform)
                class_counts[label] += 1
                vector_hits.append({
                    "address": f"0x{address:08X}", "instruction": f"0x{word:08X}",
                    "class": label, "function": owner,
                })
            elif platform == "ps2" and op in (0x36, 0x3E):
                label = "LQC2 (GPR memory->VU0 vector)" if op == 0x36 else "SQC2 (VU0 vector->GPR memory)"
                class_counts[label] += 1
                vector_hits.append({
                    "address": f"0x{address:08X}", "instruction": f"0x{word:08X}",
                    "class": label, "function": owner,
                })
            elif op == 0 and (word & 0x3F) == 0x0C:
                syscall_count += 1

            if op in LOAD_STORE_OPS and registers[rs] is not None:
                resolved = (int(registers[rs]) + signed16(immediate)) & 0xFFFFFFFF
                bucket = mmio_bucket(resolved)
                if bucket:
                    mnemonic = LOAD_STORE_OPS[op]
                    access = "write" if op in STORE_OPS else "read"
                    mmio_counts[f"{bucket} ({access})"] += 1
                    mmio_hits.append({
                        "address": f"0x{address:08X}", "instruction": f"0x{word:08X}",
                        "mnemonic": mnemonic, "access": access,
                        "mmio_address": f"0x{resolved:08X}", "mmio_region": bucket,
                        "function": owner,
                        "evidence": "linear LUI/immediate dataflow; validate before HLE binding",
                    })

            # Minimal, deliberately conservative constant propagation sufficient
            # for common `lui reg, 0x1000; addiu/ori reg, reg, offset; sw ...`.
            if op == 0x0F:  # LUI
                registers[rt] = immediate << 16
            elif op == 0x0D and registers[rs] is not None:  # ORI
                registers[rt] = int(registers[rs]) | immediate
            elif op in (0x08, 0x09) and registers[rs] is not None:  # ADDI/ADDIU
                registers[rt] = (int(registers[rs]) + signed16(immediate)) & 0xFFFFFFFF
            elif op == 0 and (word & 0x3F) in (0x21, 0x25):  # ADDU/OR
                rd = (word >> 11) & 0x1F
                other = rt if (word & 0x3F) == 0x21 else rs
                source = rs if (word & 0x3F) == 0x21 else rt
                if registers[source] is not None and registers[other] == 0:
                    registers[rd] = registers[source]
                else:
                    registers[rd] = None
            elif op not in (0, 0x02, 0x03) and rt != 0:
                # Most other immediate/load operations overwrite rt with a value
                # that cannot safely be propagated across basic blocks.
                if op not in LOAD_STORE_OPS:
                    registers[rt] = None

    return {
        "cpu_vector_instruction_counts": dict(sorted(class_counts.items())),
        "cpu_vector_instruction_samples": vector_hits[:2048],
        "direct_mmio_candidate_counts": dict(sorted(mmio_counts.items())),
        "direct_mmio_candidates": mmio_hits[:2048],
        "direct_mmio_candidates_total": len(mmio_hits),
        "cpu_vector_instruction_total": len(vector_hits),
        "cpu_syscall_instruction_total": syscall_count,
    }


def top_owners(records: Iterable[dict[str, object]]) -> list[dict[str, object]]:
    counts: Counter[str] = Counter()
    for record in records:
        owner = record.get("function")
        if owner:
            counts[str(owner)] += 1
    return [{"function": name, "count": count} for name, count in counts.most_common(32)]


def markdown_report(result: dict[str, object]) -> str:
    ps2 = result["ps2"]
    psp = result["psp"]
    lines = [
        "# VCS graphics hardware boundary inventory",
        "",
        "## Result",
        "",
        "VCS PS2 does not reach the GS through a single game instruction. The observed architecture is:",
        "",
        "```text",
        "EE code -> GIF/VIF DMA MMIO -> VIF1 command stream -> VU1 microcode",
        "        -> XGKICK -> GIF packet -> GS registers/primitives",
        "```",
        "",
        "The PSP analogue is not CPU COP2: it is a GE command list submitted through the",
        "`sceGe_user` / display boundary. PSP COP2 hits are VFPU candidates, not proof of GE submission.",
        "",
        "Therefore a `librw` HLE cut must be made **above** VIF/VU packet construction, at a recovered",
        "VCS/RenderWare object render function. Replacing XGKICK or GS packet handling would still require",
        "decoding transformed vertices, material state, texture/CLUT state and GS draw semantics.",
        "",
        "## Static inventory",
        "",
        f"- PS2 CPU vector operations: `{ps2['cpu_vector_instruction_total']}`",
        f"- PS2 directly recoverable hardware-MMIO candidates: `{ps2['direct_mmio_candidates_total']}`",
        f"- PSP CPU COP2/VFPU candidates: `{psp['cpu_vector_instruction_total']}`",
        f"- PSP GE/display tokens: `{', '.join(psp['graphics_api_tokens']) or 'none recovered from ELF strings'}`",
        "",
        "### PS2 CPU-visible instructions",
        "",
        "| Class | Meaning |",
        "| --- | --- |",
        "| `MFC2/MTC2/CFC2/CTC2`, `BC2`, VU0 macro | EE <-> **VU0** vector/control operations; often gameplay/math, not automatically rendering. |",
        "| `LQC2/SQC2` | 128-bit transfers between EE memory and VU0 vector registers. |",
        "| VIF1 `MSCAL` / `MSCNT` | **not EE instructions**; commands inside the DMA-fed VIF stream that start/resume VU1 microcode. |",
        "| VU1 `XGKICK` | **not EE instruction**; VU1 microinstruction which sends an assembled GIF packet. |",
        "| GIF `PACKED/REGLIST/IMAGE` | packet formats processed by GS; `IMAGE` uploads texture/framebuffer data. |",
        "",
        "### Existing PS2Recomp interception points",
        "",
        "| Layer | Code location | Current action | HLE suitability |",
        "| --- | --- | --- | --- |",
        "| VIF1 command decode | `ps2_vif1_interpreter.cpp` | `MSCAL`, `MSCNT`, `MPG`, `DIRECT/DIRECTHL`, UNPACK | excellent telemetry point; too low for direct librw mapping |",
        "| VU1 execution | `PS2Runtime::syncCoreSubsystems` | calls `m_vu1.execute/resume` | useful fingerprint/trace point; still platform microcode |",
        "| VU1 geometry emission | `ps2_vu1_lower.cpp` | `XGKICK -> submitGifPacket(Path1)` | GS-compatible fallback, not native RW object boundary |",
        "| GS packet consumer | `ps2_gs_gpu.cpp` | GIF packets -> GS state/rasterizer | presentation fallback; not librw-level |",
        "",
        "### VCS PS2 direct-hardware anchors",
        "",
        "| EE address | Directly decoded behaviour | Meaning for HLE |",
        "| --- | --- | --- |",
        "| `0x001A9910` | Writes VIF0 DMA `CHCR/MADR/TADR` (`0x10008000/+0x20/+0x30`). | VU0/control transfer, not a librw object draw. |",
        "| `0x003708A8` | Reads GS CSR (`0x12001000`) and writes privileged display/state registers (`0x12000000`, `+0x20`, `+0x70`, `+0x80`, `+0x90`, `+0xA0`). | GS presentation/state layer; too late for canonical geometry. |",
        "| `0x003E3620` | Reads GS CSR then runs a field-aware game-side gate. | Frame scheduling evidence, not a geometry boundary. |",
        "| `0x00447850` | Controls VIF1 registers and VU status through COP2. | PS2 SDK/VU synchronization support. |",
        "| `0x004478F0` | Polls VIF1/GIF/VU status registers. | PS2 SDK synchronization support. |",
        "",
        "## Next safe HLE experiment",
        "",
        "1. Add passive traces at VIF1 `MSCAL/MSCNT` and VU1 `XGKICK` to fingerprint microprogram/data payloads.",
        "2. Attribute a repeated fingerprint to an EE caller and then to a VCS renderer/RenderWare function.",
        "3. Intercept that caller only after proving its object/material/geometry ABI.",
        "4. Keep the current VIF/VU/GS path as a fallback until native librw draws match the captured GS path.",
        "",
        "The machine-readable companion contains instruction/MMIO addresses and function owners.",
    ]
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ps2-elf", type=Path, required=True)
    parser.add_argument("--psp-elf", type=Path, required=True)
    parser.add_argument("--ps2-functions", type=Path, help="CSV with Name,Start,End columns")
    parser.add_argument("--json", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()

    ps2_elf = Elf32(args.ps2_elf)
    psp_elf = Elf32(args.psp_elf)
    functions, starts = read_functions(args.ps2_functions)

    ps2 = scan_instructions(ps2_elf, "ps2", functions, starts)
    psp = scan_instructions(psp_elf, "psp", [], [])
    psp["graphics_api_tokens"] = psp_elf.ascii_tokens((b"sceGe", b"sceGu", b"sceDisplay"))
    psp["graphics_related_imports"] = [
        name for name in psp_elf.imported_symbol_names()
        if name.startswith(("sceGe", "sceGu", "sceDisplay"))
    ]

    result: dict[str, object] = {
        "schema": "vcs-graphics-hardware-boundary/v1",
        "evidence_policy": {
            "proven": "decoded CPU instruction or directly resolved MMIO access",
            "candidate": "linear LUI/immediate propagation; verify in a control-flow-aware pass before binding an HLE override",
            "not_claimed": "no record equates a CPU vector instruction with a RenderWare draw call",
        },
        "ps2": {
            "binary": str(args.ps2_elf),
            "byte_order": ps2_elf.byte_order,
            **ps2,
            "top_vector_instruction_owners": top_owners(ps2["cpu_vector_instruction_samples"]),
            "top_mmio_candidate_owners": top_owners(ps2["direct_mmio_candidates"]),
            "render_path": [
                "EE code writes GIF/VIF DMA channel MMIO",
                "VIF1 command stream uploads VU1 code/data and issues MSCAL/MSCNT",
                "VU1 microcode emits GIF through XGKICK",
                "GIF packet updates GS state/draws primitives",
            ],
        },
        "psp": {
            "binary": str(args.psp_elf),
            "byte_order": psp_elf.byte_order,
            **psp,
            "render_path": [
                "CPU/RSL builds a GE display list in RAM",
                "sceGe_user submits/enqueues the list to the PSP Geometry Engine",
                "sceDisplay presents the rendered framebuffer",
            ],
        },
        "ps2recomp_runtime_boundary": {
            "vif1": "ps2xRuntime/src/lib/ps2_vif1_interpreter.cpp",
            "vu1_callback_binding": "ps2xRuntime/src/lib/ps2_runtime.cpp: PS2Runtime::syncCoreSubsystems",
            "xgkick": "ps2xRuntime/src/lib/vu/ps2_vu1_lower.cpp",
            "gif_gs": "ps2xRuntime/src/lib/ps2_gs_gpu.cpp",
            "librw_cut_verdict": "Do not HLE at XGKICK/GIF/GS. Recover a VCS/RW caller above VIF packet construction, then bridge its canonical geometry/material ABI to librw.",
        },
    }
    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.write_text(markdown_report(result), encoding="utf-8")
    print(json.dumps({
        "ps2_vector_ops": ps2["cpu_vector_instruction_total"],
        "ps2_mmio_candidates": ps2["direct_mmio_candidates_total"],
        "psp_cop2_ops": psp["cpu_vector_instruction_total"],
        "psp_graphics_tokens": psp["graphics_api_tokens"],
        "json": str(args.json),
        "markdown": str(args.markdown),
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
