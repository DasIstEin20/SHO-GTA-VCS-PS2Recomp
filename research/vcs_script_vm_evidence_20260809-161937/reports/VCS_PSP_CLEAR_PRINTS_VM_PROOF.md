# VCS PSP/PS2 opcode `0059`: no-parameter UI clear bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x002EB77C` | `script_vm_matcher/artifacts/review/vcs_clear_prints/002eb77c_FUN_002eb77c.c` |
| PS2 | `0x003DAE38` | `script_vm_matcher/artifacts/review/vcs_clear_prints/003dae38_FUN_003dae38.c` |

Each handler has no parameter decoder call, invokes exactly one native callee,
sets `v0 = 0` and returns. There is no condition-state, output-store or thread
field mutation inside the handler.

The portable contract is therefore one explicit `ClearPrintsAdapter()` call and
native VM status `CONTINUE`. The original native callee's display/print owner
has not been source-named, so the adapter deliberately owns that integration.
Missing host support faults as `Opcode0059RequiresClearPrintsAdapter`.
