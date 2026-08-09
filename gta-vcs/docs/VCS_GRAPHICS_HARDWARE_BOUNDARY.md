# VCS graphics hardware boundary inventory

## Result

VCS PS2 does not reach the GS through a single game instruction. The observed architecture is:

```text
EE code -> GIF/VIF DMA MMIO -> VIF1 command stream -> VU1 microcode
        -> XGKICK -> GIF packet -> GS registers/primitives
```

The PSP analogue is not CPU COP2: it is a GE command list submitted through the
`sceGe_user` / display boundary. PSP COP2 hits are VFPU candidates, not proof of GE submission.

Therefore a `librw` HLE cut must be made **above** VIF/VU packet construction, at a recovered
VCS/RenderWare object render function. Replacing XGKICK or GS packet handling would still require
decoding transformed vertices, material state, texture/CLUT state and GS draw semantics.

## Static inventory

- PS2 CPU vector operations: `999`
- PS2 directly recoverable hardware-MMIO candidates: `72`
- PSP CPU COP2/VFPU candidates: `4157`
- PSP GE/display tokens: `sceDisplay, sceGe_user`

### PS2 CPU-visible instructions

| Class | Meaning |
| --- | --- |
| `MFC2/MTC2/CFC2/CTC2`, `BC2`, VU0 macro | EE <-> **VU0** vector/control operations; often gameplay/math, not automatically rendering. |
| `LQC2/SQC2` | 128-bit transfers between EE memory and VU0 vector registers. |
| VIF1 `MSCAL` / `MSCNT` | **not EE instructions**; commands inside the DMA-fed VIF stream that start/resume VU1 microcode. |
| VU1 `XGKICK` | **not EE instruction**; VU1 microinstruction which sends an assembled GIF packet. |
| GIF `PACKED/REGLIST/IMAGE` | packet formats processed by GS; `IMAGE` uploads texture/framebuffer data. |

### Existing PS2Recomp interception points

| Layer | Code location | Current action | HLE suitability |
| --- | --- | --- | --- |
| VIF1 command decode | `ps2_vif1_interpreter.cpp` | `MSCAL`, `MSCNT`, `MPG`, `DIRECT/DIRECTHL`, UNPACK | excellent telemetry point; too low for direct librw mapping |
| VU1 execution | `PS2Runtime::syncCoreSubsystems` | calls `m_vu1.execute/resume` | useful fingerprint/trace point; still platform microcode |
| VU1 geometry emission | `ps2_vu1_lower.cpp` | `XGKICK -> submitGifPacket(Path1)` | GS-compatible fallback, not native RW object boundary |
| GS packet consumer | `ps2_gs_gpu.cpp` | GIF packets -> GS state/rasterizer | presentation fallback; not librw-level |

### VCS PS2 direct-hardware anchors

| EE address | Directly decoded behaviour | Meaning for HLE |
| --- | --- | --- |
| `0x001A9910` | Writes VIF0 DMA `CHCR/MADR/TADR` (`0x10008000/+0x20/+0x30`). | VU0/control transfer, not a librw object draw. |
| `0x003708A8` | Reads GS CSR (`0x12001000`) and writes privileged display/state registers (`0x12000000`, `+0x20`, `+0x70`, `+0x80`, `+0x90`, `+0xA0`). | GS presentation/state layer; too late for canonical geometry. |
| `0x003E3620` | Reads GS CSR then runs a field-aware game-side gate. | Frame scheduling evidence, not a geometry boundary. |
| `0x00447850` | Controls VIF1 registers and VU status through COP2. | PS2 SDK/VU synchronization support. |
| `0x004478F0` | Polls VIF1/GIF/VU status registers. | PS2 SDK synchronization support. |

## Next safe HLE experiment

1. Add passive traces at VIF1 `MSCAL/MSCNT` and VU1 `XGKICK` to fingerprint microprogram/data payloads.
2. Attribute a repeated fingerprint to an EE caller and then to a VCS renderer/RenderWare function.
3. Intercept that caller only after proving its object/material/geometry ABI.
4. Keep the current VIF/VU/GS path as a fallback until native librw draws match the captured GS path.

The machine-readable companion contains instruction/MMIO addresses and function owners.
