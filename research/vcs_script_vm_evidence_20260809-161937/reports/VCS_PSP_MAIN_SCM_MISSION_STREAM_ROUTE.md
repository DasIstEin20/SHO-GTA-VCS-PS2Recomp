# VCS PSP `MAIN.SCM` first mission-stream route

## Scope

This is a read-only portable-host contract proof over the local original
`MAIN.SCM`, not PPSSPP telemetry and not evidence that the game naturally took
this route in a particular session. `0289` remains an explicit fault in the
generic VM core. The separate harness reconstructs only the subsequently
proved host ordering.

## Observed transfer

```text
parent entry `SCRIPT_NAME MAIN`
    → 216 portable VM commands
    → 0289 at IP 0x7889, selector raw 0

selector 0
    → MissionStreamScopeEnter
    → mission_offset_table[0] = 0x03A4D6
    → mission_offset_table[1] = 0x03B01B
    → source body offset 0x03A4D6 (file offset 0x03A4DE)
    → copy size 0x0B45
    → active buffer / child IP 0x03A4D6
    → CTheScripts::StartNewScript (idle → active)
    → MissionStreamScopeLeave

child immediate Process()
    → 0238 SCRIPT_NAME
    → 013F CREATE_PICKUP
    → explicit missing model-object resolver boundary
```

The native source-order values read by the confirmed table parser are:

| Field | Value |
| --- | ---: |
| `MainScriptSize` | `0x03A4D6` |
| `LargestMissionScriptSize` | `0x969C` |
| `NumTrueGlobals` | `5330` |
| `MostGlobals` | `1049` |
| mission-offset dwords copied | `99` |
| global clear start | `0x5350` |
| global clear size | `0x1064` |

The harness applies the confirmed `0289` target flags before child processing:
the portable analogues of native `+0x20A` and `+0x217` are both true. It also
sets the mission-active state and clears exactly `MostGlobals * 4` bytes at
`NumTrueGlobals * 4 + 8` before the re-entrant child call. This clear is now a
directly recovered `0289` operation, not an unspecified mission-context hook.

The production-native ordering surrounding the selected slice is now also
known: `MissionStreamScopeEnter` increments a global nesting counter before
source positioning/copy; the confirmed `CTheScripts::StartNewScript` removes
an idle thread, initializes it, assigns IP and puts it on the active list; and
`MissionStreamScopeLeave` decrements the counter afterwards (PS2 finalizes on
the zero transition). The harness models the resulting VM-visible child but
does not claim to own the original pool, I/O object or finalizer.

## Current boundary

`013F` decodes all five real inputs before faulting at the model-object
resolver. This is intentional: the VM now owns the direct parameter ABI,
negative-selector branch, ground-Z sentinel, output write and `CONTINUE`
status; model-object table storage, terrain query and pickup pool remain
explicit host adapters. See `reports/VCS_PSP_CREATE_PICKUP_VM_PROOF.md`.

An explicitly labelled synthetic-host dry-run (not semantic evidence for model
IDs, ground height or pickup handles) continues this child to its normal
`0023 TERMINATE_THIS_SCRIPT` yield. Its real bytecode sequence contains `71`
instances of `013F`, `52` instances of `01F9 CREATE_PICKUP_WITH_AMMO`, one
`0238`, one raw-dword assignment and one `0023`. The adapters supplied only
synthetic host results; the opcode count is route-triage information, not game
state validation.

The parent subsequently encounters another `0289` with decoded selector `1`.
This is recorded as a second occurrence, not silently treated as an
already-implemented generic mission launch. General selector scheduling remains
future host work.

The bounded static main-segment scan now finds 11 direct `0289` selector
literals: `0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9`. That gives a deterministic first
child-census batch but is not a reachability statement. See
`reports/VCS_PSP_MAIN_SCM_MISSION_SELECTOR_STATIC_CLUSTER.md`.

A read-only selector 0–9 census has now converted that static cluster into an
evidence-driven next-opcode queue without pretending the overrides are native
execution: `01E7` is the furthest unsupported child wall, while selector 5
reaches known host boundary `0045` and selector 9 yields correctly at `WAIT`.
See `reports/VCS_PSP_MAIN_SCM_MISSION_CHILD_CENSUS.md`.

## Reproduction

```powershell
.\vcs_vm_core\build\Release\vcs_vm_mission_stream_harness.exe `
  '.\GTA VCS [Debug Build]\PSP_GAME\USRDIR\RUNDATA\MAIN.SCM' `
  --parent-commands 512 --mission-commands 512 `
  --resume-parent-commands 128 --mission-mode 0 `
  --trace-jsonl '.\reports\VCS_PSP_MAIN_SCM_MISSION_STREAM_TRACE.jsonl'
```

`--mission-mode 0` is an explicit offline fixture selecting the stream branch;
it is not a statement about current native mission-mode memory.
