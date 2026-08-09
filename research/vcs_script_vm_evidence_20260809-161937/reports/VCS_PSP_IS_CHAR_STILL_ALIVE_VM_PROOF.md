# VCS PSP/PS2 `0045` condition-adapter proof

## Native entry points

| Opcode | Public library label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `0045` | `IS_CHAR_STILL_ALIVE` | `0x003233F0` | `0x00412FC8` |

This proof separates VM-owned behavior from the engine-owned character
predicate. The public label is navigation evidence, not a substitute for that
separation.

## VM-owned sequence

PSP forced-linear review
`script_vm_matcher/artifacts/review/vcs_is_char_still_alive/003233f0_linear_100.txt`
and independent PS2 decompilation
`script_vm_matcher/artifacts/review/vcs_is_char_still_alive/00412fc8_FUN_00412fc8.c`
both establish this sequence:

```text
char_handle = CollectParameters(1)[0]
base_condition = engine character-pool/state predicate(char_handle)
base_condition ^= thread.raw_NOT_flag (+0x20C)
reduce through shared condition state/result (+0x206/+0x209)
return VM_CONTINUE (0)
```

The condition aggregation instructions match the already proved `055A` reducer
exactly: state zero assigns; states `1..8` AND/decrement; states `>=9`
OR/decrement and reset at the established terminal threshold. This is not an
independent ad-hoc condition ABI.

## Engine predicate boundary

The PSP path resolves the collected handle through an engine-owned pool then
checks character-specific state/helper results (including state values `0x39`
and `0x3A`). The PS2 path independently has the same pool/helper structure.
Those pools, object layouts and helper meanings are outside the Script VM.

`vcs_vm_core` therefore owns the one-parameter decoder, raw NOT handling and
condition reducer, and requires an `Opcode0045ConditionProbe` supplied by the
host. Missing probe raises `Opcode0045RequiresConditionProbe`; no fake Ped
layout or guessed liveness rule is embedded in the portable VM.

## Portable test

The core test passes a handle through the probe, uses raw opcode `0x8045` to
verify NOT inversion enters the exact reducer, and separately verifies the
explicit missing-probe fault.
