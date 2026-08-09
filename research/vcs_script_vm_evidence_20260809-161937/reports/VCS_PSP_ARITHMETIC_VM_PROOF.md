# VCS PSP/PS2 scalar-arithmetic VM proof

The following VM-only mutation commands were selected from the real VCS
`MAIN.SCM` static inventory. PSP forced-linear MIPS exposes the full code after
the normal decompiler's collector boundary; PS2 decompilation independently
corroborates the same operation and `CONTINUE` status.

| Opcode | Native action after destination resolver + `CollectParameters(1)` | PSP instruction | PS2 handler |
| --- | --- | --- | --- |
| `0008` | binary32 add | `add.s` | `0x003B1598` |
| `0009` | raw signed-int subtraction modulo `2^32` | `subu` | `0x003B1610` |
| `000A` | binary32 subtraction | `sub.s` | `0x003B1688` |
| `000B` | low 32 bits of signed-integer product | `mult` + `mflo` | `0x003B1700` |
| `000C` | binary32 multiplication | `mul.s` | `0x003B1778` |
| `000D` | signed integer quotient | `div` + `mflo` | `0x003B17F0` |
| `000E` | binary32 division | `div.s` | `0x003B1878` |

All seven PSP windows show the same prolog: resolve a variable destination from
`thread.IP`, call `CollectParameters(thread, &IP, 1, ScriptParams)`, operate
on the destination and `ScriptParams[0]`, write the result, put `0` in `v0`
and return. Thus their VM status is `CONTINUE`, not a script output.

## Direct dispatch aliases, without speculative public names

This is stronger than a histogram match: both opcode tables resolve the
following secondary slots to the **same native target on their respective
platforms** as the directly reviewed primary slot. The portable core therefore
implements the identical semantic contract, but uses deliberately generic alias
labels until a separate source-kind/public-name proof exists.

| Primary direct proof | PSP alias | PSP target | PS2 alias | PS2 target |
| --- | --- | --- | --- | --- |
| `0008` float add | `002A` | `0x002B7570` | `002A` | `0x003B1598` |
| `0009` integer subtract | `002B` | `0x002B75D8` | `002B` | `0x003B1610` |
| `000A` float subtract | `002C` | `0x002B7640` | `002C` | `0x003B1688` |
| `000B` integer multiply | `002D` | `0x002B76A8` | `002D` | `0x003B1700` |
| `000C` float multiply | `002E` | `0x002B7714` | `002E` | `0x003B1778` |
| `000D` integer divide | `002F` | `0x002B777C` | `002F` | `0x003B17F0` |
| `000E` float divide | `0030` | `0x002B77E8` | `0030` | `0x003B1878` |

The table facts are recorded in `VCS_PSP_OPCODE_TABLE.json` and
`VCS_PS2_HANDLER_DB.json`. `0029` likewise shares `0007`'s target, but its
PSP continuation has not yet earned the stronger direct-arithmetic proof and
remains separately labelled as partial evidence.

## Portable boundary for integer division

Both binaries issue native `div` directly. The recovered opcode contains no
language-level branch that defines a quotient for divisor zero or for signed
`INT_MIN / -1`. In those two cases the portable core returns an explicit fault
(`IntegerDivisionByZero` or `IntegerDivisionOverflow`) rather than inventing a
host-C++ result. For normal operands it uses C++17 signed division, whose
truncate-toward-zero quotient matches the observed MIPS `div` contract.

Float operators preserve and store raw IEEE-754 binary32 parameter bits. The
host-PC core does not normalize values through double precision or turn float
storage into an engine adapter.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b7570_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b75d8_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b7640_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b76a8_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b7714_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b777c_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/002b77e8_linear_26.txt`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1598_FUN_003b1598.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1610_FUN_003b1610.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1688_FUN_003b1688.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1700_FUN_003b1700.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1778_FUN_003b1778.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b17f0_FUN_003b17f0.c`
- `script_vm_matcher/artifacts/review/vcs_arithmetic_ops/003b1878_FUN_003b1878.c`
