# VCS PC bootstrap vertical slice

## Outcome

`gta-vcs-bootstrap.exe` is now a native C++17, read-only vertical-slice
executable. Given an extracted VCS PS2 disc tree or an ISO9660 image, it loads
the original `MAIN.SCM`, creates portable ScriptSpace, runs the verified VM,
handles the first real `0289` through `VcsMissionHost`, creates the real
mission child and stops at the first explicit host/VM boundary.

It is deliberately not a game launcher and contains no game executable, ISO,
SCM payload, asset, decryption key or rendering implementation.

```text
disc source
  -> IVcsFileSystem (DirectoryBackend | Ps2IsoBackend)
  -> VcsScriptContainer / ReadMultiScriptFileOffsetsFromScript layout
  -> vcs_vm_core main ScriptThread
  -> 0289 VcsMissionHost
  -> selected original mission bytes in active ScriptSpace buffer
  -> CTheScripts::StartNewScript-shaped pooled child
  -> re-entrant Process(child)
  -> explicit first missing host boundary
```

## Direct verification on local original data

### VCS PS2 extracted tree

Input was the existing read-only extracted tree. The tool resolved its root
`MAIN.SCM` without copying it to the workspace.

```text
main_size=0x3CAE3
entry_ip=0x780C
mission_count=101
parent executed=225, last opcode=0289
selector=0, stream scope depth=0, active child count=1
child executed=2, last opcode=013F
child fault=Opcode013FRequiresPickupModelObjectResolver
```

This input also matches the optional structural profile
`VCS_PS2_SLES_546.22_LAYOUT` (`MAIN.SCM` sizes, parser fields and 101-entry
mission table). The profile is deliberately not a cryptographic disc identity;
`--require-known-ps2-layout` requests that narrow reproducibility guard.

### VCS PSP ISO9660 control

The local PSP ISO was used only to exercise the new standard ISO9660 reader
with `PSP_GAME/USRDIR/RUNDATA/MAIN.SCM`. It is not presented as a PS2 runtime
claim.

```text
main_size=0x3A4D6
entry_ip=0x7860
mission_count=99
parent executed=216, last opcode=0289
selector=0, stream scope depth=0, active child count=1
child executed=2, last opcode=013F
child fault=Opcode013FRequiresPickupModelObjectResolver
```

Both paths prove the portable loader/model follows their respective original
SCM containers through the same evidenced `0289` boundary. Exit code `3` is
intentional here: it means the first unimplemented dependency is exposed,
instead of fabricating a successful mission.

## Exact host contract implemented

`VcsMissionHost` overrides the core's default explicit `0289` fault only when
bound by the bootstrap executable. Its order follows the PSP/PS2 proof:

1. decode one raw selector and apply the proved mission-mode guard/remap;
2. select `[offset[selector], offset[selector + 1])`, preserving the native
   largest-mission-size fallback for a non-positive range;
3. enter the reconstructed mission-stream scope;
4. copy the selected original script slice to `ScriptSpace + MainScriptSize`;
5. perform a preallocated idle-to-active `StartNewScript` semantic factory:
   process id, `Init`-equivalent reset, IP, active ownership and active flag;
6. leave the stream scope before target-marker writes;
7. set the two proven target markers, mission-active state and exact parsed
   global clear; then process the child re-entrantly;
8. return VM `CONTINUE` if the child yielded normally, or surface the child's
   specific fault to the bootstrap runner.

This is verified by a no-game-data unit test and the two real-data runs above.
The original native source spelling for lower stream I/O/finalizer helpers is
still intentionally unclaimed.

## What is proven, inferred, and unknown

| State | Scope |
| --- | --- |
| Proven | SCM header/metadata table parse; main entry search; selector-zero table transfer; scoped copy/factory/flags/global clear/re-entrant processing sequence; PSP/PS2 process routes; first child wall `013F`. |
| Inferred only as portable names | `scheduler_id`, mission-active bool and pool ownership are descriptive models of observed writes/list transitions, not Rockstar member spellings. |
| Unknown / deliberately absent | PS2 ISO was not available locally for the ISO backend test (the PS2 extracted tree was); actual model-object table, terrain ground height, pickup manager, world streaming, RenderWare, audio and full scheduler lifecycle. |

## Reproduce

```powershell
cmake -S vcs_vm_core -B vcs_vm_core/build
cmake --build vcs_vm_core/build --config Release
ctest --test-dir vcs_vm_core/build -C Release --output-on-failure

# Extracted VCS PS2 disc directory:
.\vcs_vm_core\build\Release\gta-vcs-bootstrap.exe '<extracted-vcs-ps2-dir>'

# ISO9660 VCS PS2 image (root MAIN.SCM):
.\vcs_vm_core\build\Release\gta-vcs-bootstrap.exe '<VCS_PS2.iso>'
```

An expected first result at the current milestone is:

```text
FAIL LOUDLY ... Opcode013FRequiresPickupModelObjectResolver
```

The next evidence-driven task is therefore not broad opcode collection: prove
the `013F` model-object resolver as the smallest real host adapter, then run
this exact executable again.
