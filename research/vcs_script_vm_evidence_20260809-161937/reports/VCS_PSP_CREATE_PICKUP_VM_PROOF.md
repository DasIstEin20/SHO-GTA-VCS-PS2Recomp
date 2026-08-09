# VCS PSP/PS2 `013F` create-pickup bridge

| Opcode | VCS PSP table target | VCS PS2 table target |
| --- | ---: | ---: |
| `013F` | `0x001DDCD0` | `0x002D7CE0` |
| `01F9` | `0x001DE044` | `0x002D8050` |

## Direct semantic proof

The PSP function boundaries end prematurely in automatic analysis, so the
evidence uses their forced-linear instruction windows through native return.
The PS2 functions have complete manual decompilations. `013F` independently
shows this sequence:

```text
CollectParameters(5)
model = parameter[0]
if model < 0:
    model = host_model_object_record[-model].dword_at_0x18

type = low_byte(parameter[1])
x/y/z = binary32(parameter[2..4])
if z == -100.0f or z <= -250.0f:
    z = host_ground_height(x, y) + 0.5f

prepare one output destination
pickup_handle = host_pickup_constructor(model, type, x, y, z, 0, 0, 0, 0)
StoreParameters(1, pickup_handle)
return CONTINUE (0)
```

The VCS command corpus independently records `013F` as `CREATE_PICKUP` with
five inputs (`model_object`, `pickupType`, `x`, `y`, `z`) and one `Pickup`
handle output. That naming corroborates, but does not replace, the native ABI
proof above.

`01F9` proves the same constructor path with one inserted collected dword:

```text
CollectParameters(6)
model/type = parameter[0..1]
ammo = parameter[2]
x/y/z = binary32(parameter[3..5])
apply the same negative-model and ground-Z logic
pickup_handle = same host_pickup_constructor(model, type, x, y, z, ammo, 0, 0, 0)
StoreParameters(1, pickup_handle)
return CONTINUE (0)
```

The VCS corpus calls this `CREATE_PICKUP_WITH_AMMO`; its six-input/one-output
description agrees exactly with the two native implementations.

## Native observations

- PSP `001DDCD0_linear_128.txt` calls confirmed
  `CollectParameters` at `0x0006189C` with `5`, uses `28 * -model + 0x18` for
  the negative selector form, compares z with `-100.0f` and `-250.0f`, adds
  `0.5f` after the terrain-height call, then reaches the output/constructor/
  `StoreParameters(1)` path and ends with `v0 = 0`.
- PS2 `002D7CE0_FUN_002D7CE0.c` recovers the full same flow: five collection,
  negative-record indirection, sentinel branch, output-destination setup,
  pickup construction and `StoreParameters(1)` before `return 0`.
- PSP `001DE044_linear_128.txt` changes only the collection literal to `6`,
  loads z from parameter slot five and forwards parameter slot two as the
  fourth pickup-constructor argument; it preserves the `013F` model and
  terrain branches, `StoreParameters(1)` and zero return.
- PS2 `002D8050_FUN_002D8050.c` independently confirms that exact six-value
  layout and the same constructor call with its ammo argument populated.

The fixed record table's owner and the pickup manager's original function
names are deliberately not claimed. They are host integration boundaries.

## First mission reachability

The read-only mission-stream harness parses the original VCS `MAIN.SCM`,
stages selector-zero's first mission slice and immediately processes the child
thread. Its first command is `0238 SCRIPT_NAME`; its second is this `013F`.
This establishes that `013F` is the first new real opcode boundary after the
reconstructed `0289` transfer. It is portable-route evidence, not a claim of
native PPSSPP runtime capture.

The core implementation preserves every VM-owned step above and exposes only
three narrow host hooks: negative model-object resolution, ground-height
query, and pickup construction. Missing host stages fault explicitly.

## Evidence

- `script_vm_matcher/artifacts/review/vcs_mission_first_opcode/001ddcd0_linear_128.txt`
- `script_vm_matcher/artifacts/review/vcs_mission_first_opcode/002d7ce0_FUN_002d7ce0.c`
- `script_vm_matcher/artifacts/review/vcs_mission_first_opcode/002d7ce0_linear_128.txt`
- `script_vm_matcher/artifacts/review/vcs_mission_pickup_with_ammo/001de044_linear_128.txt`
- `script_vm_matcher/artifacts/review/vcs_mission_pickup_with_ammo/002d8050_FUN_002d8050.c`
- `research_feed/external/sannybuilder-library/vcs/vcs.json`
- `reports/VCS_PSP_MISSION_STREAM_LAUNCH_BOUNDARY.md`
