# VCS PSP/PS2 numeric conversion and absolute-value VM proof

This batch is direct static PSP/PS2 proof of four variable-only VM mutations.
Both platforms resolve the documented variable operands through a tiny wrapper
around `GetPointerToScriptVariable`, mutate the selected storage and return
native `v0 = 0` (`CONTINUE`).

| Opcode | Native behavior | PSP instruction evidence | PS2 target |
| --- | --- | --- | --- |
| `0038 CSET_VAR_INT_TO_VAR_FLOAT` | source binary32 → destination int32, truncation toward zero | `trunc.w.s`, `mfc1`, `sw` | `0x003B1E30` |
| `0039 CSET_VAR_FLOAT_TO_VAR_INT` | source int32 → destination binary32 | `mtc1`, `cvt.s.w`, `swc1` | `0x003B1E98` |
| `003A ABS_INT` | integer conditional negation | `bgez`; negative path `subu zero, value` | `0x003B1F00` |
| `003B ABS_FLOAT` | float conditional negation | `c.lt.s value, +0`; `neg.s` only on true | `0x003B1F38` |

## Evidence-boundary details

- `0038` is **not floor**: PSP uses `trunc.w.s`, so normal negative values
  round toward zero. The portable core rejects NaN, infinity and values outside
  `int32` instead of claiming a native FPU-invalid conversion payload.
- `003A` uses `subu`, not a saturating absolute value. Therefore `INT_MIN`
  remains `INT_MIN` after modulo-`2^32` negation.
- `003B` branches on strict `< 0.0`; it does not negate `-0.0`, and unordered
  NaN remains unnegated. The portable model preserves those raw binary32 bits.

`FUN_0005E3D0` is directly shown to be a forwarding wrapper around confirmed
`FUN_0005DA7C GetPointerToScriptVariable`; it is not assigned an independent
semantic name.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_numeric_ops/0005e3d0_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/002b7f88_linear_38.txt`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/002b7fe8_linear_38.txt`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/002b8048_linear_28.txt`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/002b8088_linear_28.txt`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/003b1e30_FUN_003b1e30.c`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/003b1e98_FUN_003b1e98.c`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/003b1f00_FUN_003b1f00.c`
- `script_vm_matcher/artifacts/review/vcs_numeric_ops/003b1f38_FUN_003b1f38.c`
