# VCS PSP/PS2 integer-condition VM proof

This batch follows opcode use in the real VCS `MAIN.SCM` static inventory,
not a whole-function similarity score. It proves five high-use condition
commands that feed the already recovered Script VM condition reducer.

| Opcode(s) | Public SCM spelling | PSP target | PS2 target | Recovered predicate |
| --- | --- | --- | --- | --- |
| `000F`, `0010`, `0011` | `IS_INT_VAR_GREATER_THAN_NUMBER`, `IS_NUMBER_GREATER_THAN_INT_VAR`, `IS_INT_VAR_GREATER_THAN_INT_VAR` | `0x002B7850` | `0x003B18F8` | signed `param[0] > param[1]` |
| `0015`, `0016`, `0017` | integer `>=` source-kind spellings | `0x002B79F0` | `0x003B19C8` | signed `param[0] >= param[1]` |
| `001B`, `001C`, `02DB` | integer equality source-kind spellings | `0x002B7B94` | `0x003B1C50` | `param[0] == param[1]` |

The public spellings describe the source kind accepted by the SCM assembler.
After `CollectParameters(2)`, the common native handlers only observe two raw
signed `int32` values, which is why each pair shares a target.

## Direct PSP proof despite the recovered function boundary

The normal PSP decompiler stopped its function at the `jal 0x0006189C`
collector call. The return continuation is nevertheless executable MIPS, not
data. Review-only forced-linear disassembly of the adjacent bytes establishes
the full body:

```text
002B7880: lw a0, 4(s2); lw a1, ScriptParams(s1); slt a0, a0, a1
           -> param[0] > param[1]
002B7A20: lw a0, ScriptParams(s1); lw a1, 4(s2); slt; xori a0, a0, 1
           -> param[0] >= param[1]
002B7BC4: lw a0, ScriptParams(s1); lw a1, 4(s2); xor; sltiu a0, a0, 1
           -> param[0] == param[1]
```

Each PSP sequence then visibly reads `thread+0x20C`, XORs it into the
predicate, and performs the already proved `+0x206/+0x209` AND/OR state
reduction before placing `0` in `v0`. This is an independent direct PSP proof
of raw-NOT handling, condition aggregation and native `CONTINUE` status.

The PS2 decompilations independently show the same two-parameter collection,
predicate, raw-NOT inversion and reducer. For example, `003B18F8` computes
`ScriptParams[1] < ScriptParams[0]`; `003B19C8` uses `>=` (or `<` when NOT);
and `003B1C50` uses equality (or inequality when NOT).

## Portable-core contract

`vcs_vm_core` implements all nine listed integer slots as two raw signed
parameters followed by `reduce_condition_result`. It deliberately shares the
arithmetic handlers for each recovered common target while retaining individual
opcode registrations and public names in evidence metadata. This does not
imply that the analogous floating-point target families are implemented.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7850_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b79f0_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7b94_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7880_linear_38.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7a20_linear_39.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7bc4_linear_39.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/003b18f8_FUN_003b18f8.c`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/003b19c8_FUN_003b19c8.c`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/003b1c50_FUN_003b1c50.c`
