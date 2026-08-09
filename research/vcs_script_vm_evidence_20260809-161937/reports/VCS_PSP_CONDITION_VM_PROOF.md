# VCS PSP/PS2 condition VM proof

## Scope

This report proves the shared condition aggregation performed by zero-parameter
condition opcode `0x055A`. It deliberately does **not** assign a public SCM
name to the opcode or to its engine probes.

| Platform | Handler | Evidence |
| --- | --- | --- |
| VCS PSP | `0x0009E4C8` | `script_vm_matcher/artifacts/review/vcs_condition_vm/0009e4c8_FUN_0009e4c8.c` |
| VCS PS2 | `0x0012C1E0` | `script_vm_matcher/artifacts/review/vcs_condition_vm/0012c1e0_FUN_0012c1e0.c` |

The handler is selected by the independently exported descriptor tables. The
natural PPSSPP capture for this exact PSP target has raw opcode `0x055A`, no
parameters, native `v0 = 0`, and observes `thread+0x206: 0x0015 -> 0`:
`tools/ppsspp_vcs_oracle/telemetry/runtime_055A.jsonl`.

## Proven reducer

Both native handlers first compute an engine predicate `B`; their concrete
engine calls remain unnamed. PSP reaches `B = true` when
`FUN_00150AB0(...) == 0 || FUN_002CF87C() > 5`; PS2 has the structurally
equivalent `FUN_00241B80(...) == 0 || FUN_003CBFF8() >= 6` branch. They then
apply the dispatcher-supplied raw opcode NOT flag (`thread+0x20C`) as:

```text
C = B XOR thread.not_flag
```

The complete shared state transition is:

```text
S = thread.condition_state        // uint16 at +0x206
R = thread.condition_result       // bool at +0x209

if S == 0:
    R = C
else:
    S = S - 1
    if old S < 9:
        R = C AND R
    else:
        R = C OR R
        if S < 0x15:
            S = 0

return VM_CONTINUE (0)
```

This is not a guessed high-level grouping rule: it is a direct normalization of
the same conditional paths in both functions. In particular, the reset occurs
only on the OR path. The runtime capture's `0x15 -> 0` transition exercises the
`old S == 0x15`, decrement, OR and terminal-reset path.

## Implementation boundary

`vcs_vm_core` now implements the reducer and NOT application exactly. The PC
host supplies only `B` through `set_opcode_055a_condition_probe`; it must not
reimplement the count, AND/OR or NOT logic. The unlabelled engine probes remain
an explicit adapter boundary, so no speculative game-system symbol was created.
