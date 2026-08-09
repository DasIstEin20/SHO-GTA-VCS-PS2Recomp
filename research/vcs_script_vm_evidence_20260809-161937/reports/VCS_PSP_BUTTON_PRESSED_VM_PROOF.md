# VCS PSP/PS2 `007F` input-predicate condition proof

| Opcode | Public navigation label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `007F` | `IS_BUTTON_PRESSED` | `0x002ACC1C` | `0x003A7C70` |

## Recovered VM contract

```text
params = CollectParameters(2)
base = host_input_predicate(uint16(params[0]), uint16(params[1]))
condition_result = ReduceCondition(base, raw_opcode.NOT)
return VM_CONTINUE (0)
```

This proof establishes two input encodings, low-16-bit narrowing in source
order, the condition reducer and `CONTINUE`. It does not assign a native
meaning to either 16-bit input or claim a device/button enumeration merely
from the public navigation label.

## Direct PSP and PS2 evidence

The PSP forced-linear window
`script_vm_matcher/artifacts/review/vcs_button_pressed/002acc1c_linear_120.txt`
calls confirmed `CollectParameters` with count two. It loads `ScriptParams[0]`
and `[1]`, applies `andi ..., 0xFFFF` to each, and calls `0x0005ED50` with the
thread pointer followed by those values. Its result enters the exact `+0x20C`
raw-NOT / `+0x206,+0x209` condition reducer and returns zero.

The PS2 decompilation
`script_vm_matcher/artifacts/review/vcs_button_pressed/003a7c70_FUN_003a7c70.c`
independently shows `CollectParameters(..., 2, ...)`, a helper called as
`(thread, (undefined2)ScriptParams[0], (undefined2)ScriptParams[1])`, then the
same raw-NOT and shared condition-state reduction before `return 0`.

## Host boundary and portable implementation

The portable core exposes
`Opcode007FButtonPressedProbe(ScriptVm&, ScriptThread&, uint16_t, uint16_t)`.
It owns exactly the VM-visible collection, narrowing, NOT and condition-state
logic; the host owns interpretation of the two values and current input state.
A missing probe raises `Opcode007FRequiresButtonPressedProbe` rather than
pretending the condition is false.

## Portable tests

The test supplies high-bit-containing raw input dwords and proves that only
their low 16 bits reach the host in source order. It also proves raw NOT is
applied by the common reducer and that an unavailable host probe faults
explicitly.
