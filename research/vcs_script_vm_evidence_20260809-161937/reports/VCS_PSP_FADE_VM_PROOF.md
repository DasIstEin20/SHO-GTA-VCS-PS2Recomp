# VCS PSP/PS2 fade adapter proof (`00C8`, `00C9`, `00CA`)

## Native targets

| Opcode | Public label | PSP target | PS2 target |
|---|---|---:|---:|
| `00C8` | `SET_FADING_COLOUR` | `0x0018BC64` | `0x00287BE0` (inside recovered function `0x00287B08`) |
| `00C9` | `DO_FADE` | `0x0018BCC8` | `0x00287C38` |
| `00CA` | `GET_FADING_STATUS` | `0x0018BD58` | `0x00287CC8` |

The labels only locate the slots. The contracts are proved from the handler
windows/decompilations listed below.

## `00C8`: three low-byte colour channels

PSP `0018bc3c_linear_54.txt` enters at the actual target `0x0018BC64`, calls
the confirmed collector with count three, masks each collected dword with
`0xFF`, then calls a fixed engine fade-object method. PS2's corresponding
entry takes the same three collected dwords and invokes the same fixed fade
subsystem path.

```text
parameters = CollectParameters(3)
fade.set_colour(uint8(parameters[0]), uint8(parameters[1]), uint8(parameters[2]))
return VM_CONTINUE (0)
```

The VM does not invent a renderer or RGBA object layout; it requires a
`FadeSetColourAdapter`.

## `00C9`: exact duration normalization and mode narrowing

PSP `0018bcc8_linear_54.txt` and PS2
`00287c38_FUN_00287c38.c` independently show:

```text
parameters = CollectParameters(2)
duration = parameters[0] <= 2 ? 0.0f : float(parameters[0]) * 0.001f
mode = int16(parameters[1])
fade.start(duration, mode)
return VM_CONTINUE (0)
```

The threshold, binary32 multiplier, and signed 16-bit narrowing are native
semantics. The meaning of individual mode values belongs to the fixed engine
fade subsystem, so the portable core exposes `FadeStartAdapter` rather than a
guessed enum.

## `00CA`: no SCM parameter, shared condition reducer

PSP `0018bd58_FUN_0018bd58.c` / forced-linear window and PS2
`00287cc8_FUN_00287cc8.c` call the fixed fade object with **no collected SCM
parameters**, turn its nonzero result into the base condition, apply raw NOT
from `thread+0x20C`, then execute the same exact condition reducer over
`thread+0x206/+0x209` that was proved for `055A` and `0045`.

```text
base_condition = fade.status_probe()
reduce_condition_result(base_condition XOR raw_NOT)
return VM_CONTINUE (0)
```

The portable VM requires `FadeStatusProbe`; missing renderer state therefore
faults explicitly rather than returning a fabricated false condition.

## Evidence artifacts and test

- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bc64_FUN_0018bc64.c`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bcc8_FUN_0018bcc8.c`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bd58_FUN_0018bd58.c`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bc3c_linear_54.txt`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bcc8_linear_54.txt`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/0018bd58_linear_48.txt`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/00287b08_FUN_00287b08.c`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/00287c38_FUN_00287c38.c`
- `script_vm_matcher/artifacts/review/vcs_fade_vm/00287cc8_FUN_00287cc8.c`

The portable test verifies byte masking (`0x123 -> 0x23`, `-1 -> 0xFF`), the
`2000 -> 2.0f` duration conversion, signed `0xFFFF -> -1` mode narrowing,
raw-NOT condition aggregation and the explicit missing-status-probe fault.
