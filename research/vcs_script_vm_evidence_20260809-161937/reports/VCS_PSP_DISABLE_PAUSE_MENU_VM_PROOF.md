# VCS PSP/PS2 opcode `0401`: pause-menu boolean bridge

## Confirmed handler targets

| Platform | Dispatch target | Review evidence |
| --- | --- | --- |
| PSP | `0x0031A0C0` | `script_vm_matcher/artifacts/review/vcs_disable_pause_menu/0031a0c0_linear_64.txt` |
| PS2 | `0x00406240` | `script_vm_matcher/artifacts/review/vcs_disable_pause_menu/00406240_FUN_00406240.c` |

## Shared semantic sequence

Both handlers:

1. call their established plural collector with exactly one parameter;
2. read that collected raw parameter from the shared parameter buffer;
3. branch only on zero/non-zero;
4. obtain the platform's pause-menu object and call the same boolean setter
   path with `false` for zero or `true` for non-zero;
5. return native `v0 = 0` (`CONTINUE`).

The PSP forced-linear window makes the behaviour visible despite its initial
autoanalysis boundary: the collected value at `0x0001D698` selects the branch,
and the final call receives literal `1` or `0`. The PS2 decompilation shows the
same collector/count, branch and literal setter arguments.

## Portable boundary

`ScriptVm` implements this as `PauseMenuToggleAdapter(bool parameter_is_nonzero)`.
It owns collection and zero/non-zero normalization; the adapter owns the
platform's menu-object lifetime and application. Omitting the adapter produces
`Opcode0401RequiresPauseMenuToggleAdapter`, never a successful fake yield.

This validates a third real `MAIN.SCM` entry command in the read-only prefix
harness. The local input's command at `0x007875..0x007879` has a non-zero
parameter, so the recording adapter observed `true`.

No source-level class/function spelling is asserted for either menu setter.
