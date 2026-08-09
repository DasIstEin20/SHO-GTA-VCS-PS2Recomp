# VCS PSP/PS2 float-condition VM proof

This is a direct instruction-level proof for the floating-point condition
families selected from the real VCS `MAIN.SCM` static inventory. It preserves
the comparison form actually executed by the native FPU, including unordered
values, instead of replacing it with superficially similar C++ operators.

| Opcode(s) | Public SCM spelling family | PSP target | PS2 target | Native predicate |
| --- | --- | --- | --- | --- |
| `0012`, `0013`, `0014` | float `>` source-kind spellings | `0x002B7918` | `0x003B1A90` | `!(param[0] <= param[1])` |
| `0018`, `0019`, `001A` | float `>=` source-kind spellings | `0x002B7ABC` | `0x003B1B70` | `!(param[0] < param[1])` |
| `001D`, `001E` | float equality source-kind spellings | `0x002B7B94` | `0x003B1C50` | raw-dword equality |

## Greater-than and greater-or-equal

PSP forced-linear code after `CollectParameters(2)` proves that the first
family loads the raw parameter dwords with `lwc1`, executes `c.le.s`, then
uses an inverted `bc1fl` path to make the result true only when the compare is
false. Its exact semantic expression is therefore:

```text
greater = !(left <= right)
```

The second PSP family uses `c.lt.s` plus the same inverted branch shape:

```text
greater_or_equal = !(left < right)
```

The PS2 windows independently use `lwc1`, `c.lt.s` for the first family with
operands reversed, and `c.le.s` for the second family with operands reversed.
They are the same two expressions. Both platforms then XOR the predicate with
`thread+0x20C`, run the shared `+0x206/+0x209` reducer, set `v0 = 0` and
return `CONTINUE`.

The negated expressions are deliberate: on these native FPU paths an unordered
comparison makes `c.le.s`/`c.lt.s` false and the inverted branch yields true.
The portable core represents that exact branch result as `!(left <= right)` or
`!(left < right)`, using preserved IEEE-754 binary32 parameter bits.

## Float equality is not numeric equality

`001D` and `001E` deliberately share `0x002B7B94` / `0x003B1C50` with integer
equality. The PSP instructions are `lw`, `lw`, `xor`, `sltiu`, not FPU
instructions. The predicate is raw 32-bit equality, so `+0.0f` and `-0.0f`
are different while two equal NaN payloads compare true. The portable core has
a regression test for both cases.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_float_conditions/002b7948_linear_42.txt`
- `script_vm_matcher/artifacts/review/vcs_float_conditions/002b7aec_linear_42.txt`
- `script_vm_matcher/artifacts/review/vcs_float_conditions/003b1a90_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_float_conditions/003b1b70_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/002b7bc4_linear_39.txt`
- `script_vm_matcher/artifacts/review/vcs_int_conditions/003b1c50_FUN_003b1c50.c`
