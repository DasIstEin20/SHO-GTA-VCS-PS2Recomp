# VCS PSP/PS2 primitive variable-operation proof

The following frequent `MAIN.SCM` commands are VM-only: they require no game
subsystem adapter. PSP confirms resolver-plus-collector entry order; PS2 gives
the full raw dword store/add continuations.

| Opcode(s) | PSP | PS2 | Proven action |
| --- | --- | --- | --- |
| `0004`, `0005`, `0035`, `0036`, `02E2` | `0x002B7448` | `0x003B14B0` | resolve destination, collect one raw parameter, write its dword unchanged |
| `0006` | `0x002B74A8` | `0x003B2A20` | independently duplicated resolver + one-parameter raw-dword assignment |
| `0007`, `0029` | `0x002B7508` | `0x003B1520` | resolve destination, collect one raw parameter, add it to the current signed-int dword using native 32-bit wraparound |

## Assignment aliases

PS2 `0x003B14B0` calls the variable-pointer resolver with `&thread.IP`, calls
the one-parameter collector, then stores `ScriptParams[0]` through that pointer.
No integer/float conversion occurs. PSP `0x002B7448` has the same resolver then
collector sequence, but Ghidra cuts its continuation at the collector helper.

`0006 SET_VAR_TEXT_LABEL` has a separate PSP/PS2 target, but both direct
decompilations prove exactly the same resolver, one-parameter collector and
single-dword store as the assignment target above. Thus the native VM action
for `SET_VAR_INT`, `SET_VAR_FLOAT`, `SET_VAR_TEXT_LABEL`,
`SET_VAR_INT_TO_VAR_INT`, `SET_VAR_FLOAT_TO_VAR_FLOAT` and
`SET_VAR_INT_TO_CONSTANT` is raw-dword
assignment; the distinction is source/type intent before the handler runs.
The portable model stores a literal-string parameter as its ScriptSpace offset,
not a host `char*`. These cross-platform rows remain intentionally
`PARTIAL_MANUAL_REVIEW` until PSP's post-collector continuations are recovered
as a single normal Ghidra function.

## `0007 ADD_VAL_TO_INT_VAR` / `0029 ADD_INT_VAR_TO_INT_VAR`

PS2 `0x003B1520` resolves the destination, collects one parameter and executes
`*destination = *destination + ScriptParams[0]`. The portable core uses explicit
modulo-`2^32` arithmetic before reinterpreting as `int32`, avoiding C++ signed
overflow while matching MIPS/EE register semantics. PSP has the same resolver
and one-parameter collector setup; its continuation has the same Ghidra boundary
limitation. Both tables map `0029` to the same native target, so it has the
same raw parameter and wraparound action.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_variable_ops/002b7448_FUN_002b7448.c`
- `script_vm_matcher/artifacts/review/vcs_variable_ops/002b7508_FUN_002b7508.c`
- `script_vm_matcher/artifacts/review/vcs_variable_ops/003b14b0_FUN_003b14b0.c`
- `script_vm_matcher/artifacts/review/vcs_variable_ops/003b1520_FUN_003b1520.c`
- `script_vm_matcher/artifacts/review/vcs_text_label/002b74a8_FUN_002b74a8.c`
- `script_vm_matcher/artifacts/review/vcs_text_label/003b2a20_FUN_003b2a20.c`
