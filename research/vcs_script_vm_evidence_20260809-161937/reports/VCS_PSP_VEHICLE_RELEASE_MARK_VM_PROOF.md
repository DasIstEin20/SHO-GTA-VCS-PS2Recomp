# VCS PSP/PS2 opcode `0113`: vehicle release-mark bridge

## Scope

This proof concerns the VM-facing contract only. It does **not** claim the
original source spelling of either vehicle helper, nor does it equate this
command with `0049`, whose recovered native path destroys a vehicle entity.

| Build | Table target | Evidence |
| --- | --- | --- |
| VCS PSP | `0x002E1700` | `script_vm_matcher/artifacts/review/vcs_vehicle_release_mark/002e1700_linear_112.txt` |
| VCS PS2 | `0x003D2218` | `script_vm_matcher/artifacts/review/vcs_vehicle_release_mark/003d2218_FUN_003d2218.c` |

## PSP instruction-level path

`0x002E1700` calls `CollectParameters(thread, thread + 0x10, 1, ScriptParams)`.
It passes `ScriptParams[0]` through a vehicle-pool style resolver
(`0x003620A4`) and then calls the separate native helper at `0x0006017C` with
the resolved result. It reads byte `thread + 0x20A`; only when nonzero does it
call an additional context-cleanup path (`0x0006165C`) with the current thread
and collected parameter. It then sets `v0 = 0` and returns.

## PS2 cross-check

`0x003D2218` has the same shape: `CollectParameters(..., 1, ScriptParams)`,
vehicle resolution (`0x00471B10`), one distinct native helper
(`0x0025B300`) on the resolved vehicle, an optional `thread + 0x20A` cleanup
call (`0x00253F68`), then native handler status zero.

## Confirmed portable contract

```text
input:   exactly one encoded raw vehicle handle
host:    resolve/release-mark vehicle; perform the additional context cleanup
         iff the portable analogue of native thread+0x20A is nonzero
output:  none
flow:    CONTINUE (native v0 = 0)
```

The portable core exposes this through `VehicleReleaseMarkAdapter(handle,
native_context_flag)`. Pool lookup, vehicle reference policy, helper names,
and context-cleanup ownership remain host integration work. This is deliberately
separate from `VehicleDeleteAdapter` for `0049`.
