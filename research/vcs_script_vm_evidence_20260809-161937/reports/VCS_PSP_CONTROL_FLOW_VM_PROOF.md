# VCS PSP/PS2 hot control-flow VM proof

These four commands are selected from the static `MAIN.SCM` inventory, not
from a whole-function similarity ranking. They connect the recovered condition
reducer to scripts actually present in VCS.

| Opcode | Name | PSP handler | PS2 handler | Result |
| --- | --- | --- | --- | --- |
| `0022` | `GOTO_IF_FALSE` | `0x002B7CCC` | `0x003B1FD8` | PSP entry proves one parameter collection; PS2 proves false-branch target transfer. |
| `005E` | `RETURN_TRUE` | `0x002B80D4` | `0x003B21A8` | Same reducer with base predicate true, then shared return core. |
| `005F` | `RETURN_FALSE` | `0x002B815C` | `0x003B2230` | Same reducer with base predicate false, then shared return core. |
| `0078` | `IF` | `0x002B81E0` | `0x003B22B8` | One count parameter initializes the condition aggregation state. |

## `0078 IF`

PS2 collects one parameter, truncates it to `uint16`, writes it to
`thread+0x206`, and, only for nonzero counts, seeds `thread+0x209`:

```text
condition_state = uint16(count)
if condition_state != 0:
    condition_result = (condition_state < 9)
return VM_CONTINUE (0)
```

PSP has the same one-parameter collector entry. This produces an AND group
seeded true for counts `1..8`, and an OR group seeded false for counts `>=9`.
`count == 0` clears the state but does not overwrite the prior result.

## `005E RETURN_TRUE` and `005F RETURN_FALSE`

The PSP and PS2 decompilations are structurally identical to the already
proved `055A` reducer. `RETURN_TRUE` feeds base predicate `true`; `RETURN_FALSE`
feeds base predicate `false`. In both cases raw opcode NOT is still applied by
the reducer, so `NOT RETURN_FALSE` produces true. The handlers then call the
same `ReturnFromGosubOrFunction` core as opcode `0026` and report status 0.

## `0022 GOTO_IF_FALSE`

PS2 collects one script target and writes it to `thread+0x10` only when
`thread+0x209 == false`; otherwise the already advanced IP is retained. PSP
`0x002B7CCC` directly calls the confirmed `CollectParameters` with count 1,
but its Ghidra boundary ends at that helper, so this row is intentionally
classified `PARTIAL_MANUAL_REVIEW`, not cross-platform equality.

## Portable boundary

The core implements all four bytecode contracts. `GOTO_IF_FALSE` is marked
PS2-static/PSP-entry corroborated; the other three have complete PSP/PS2 static
reducer or state-machine proof.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_control_flow/002b7ccc_FUN_002b7ccc.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/002b80d4_FUN_002b80d4.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/002b815c_FUN_002b815c.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/002b81e0_FUN_002b81e0.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/003b1fd8_FUN_003b1fd8.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/003b21a8_FUN_003b21a8.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/003b2230_FUN_003b2230.c`
- `script_vm_matcher/artifacts/review/vcs_control_flow/003b22b8_FUN_003b22b8.c`
