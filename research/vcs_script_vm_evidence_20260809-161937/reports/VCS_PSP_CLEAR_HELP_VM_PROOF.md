# VCS PSP/PS2 opcode `0268`: clear-help host bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x002ED0E8` | `script_vm_matcher/artifacts/review/vcs_clear_help/002ed0e8_FUN_002ed0e8.c` |
| PS2 | `0x003DC6D0` | `script_vm_matcher/artifacts/review/vcs_clear_help/003dc6d0_FUN_003dc6d0.c` |

Both handlers consume no SCM parameter. Each ensures its global help/HUD
manager exists, invokes two zero-argument native clearing paths on it, and
returns `v0 = 0` (`CONTINUE`). The PS2 second callee receives an additional
constant flag, but it remains inside the same native help-clear sequence; no
meaning for that internal flag is asserted.

The portable core owns no parameter consumption and one explicit
`ClearHelpAdapter()` call. The adapter owns manager allocation/lifetime and the
two platform UI clear operations. Absent host support faults as
`Opcode0268RequiresClearHelpAdapter`.
