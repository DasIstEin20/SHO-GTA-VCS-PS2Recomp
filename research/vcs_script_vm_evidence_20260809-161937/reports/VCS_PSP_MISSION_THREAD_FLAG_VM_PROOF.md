# VCS PSP/PS2 `04F2` active-script mission-marker condition proof

| Opcode | Public navigation label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `04F2` | `IS_THIS_SCRIPT_A_MISSION` | `0x002B89FC` | `0x003B2B20` |

## Recovered VM contract

```text
base = bool(current_active_script->byte_0x217)
condition_result = ReduceCondition(base, raw_opcode.NOT)
return VM_CONTINUE (0)
```

There are no SCM parameters or output destinations. The native code fetches
the script object through the VM's global active-script pointer rather than
using an external engine predicate. The portable API's `process(thread)` and
`process_one_command(thread)` establish that `thread` as the active script for
this semantic model.

## Direct PSP and PS2 evidence

PSP window
`script_vm_matcher/artifacts/review/vcs_thread_state_conditions/002b89fc_linear_100.txt`
loads the active script pointer from its GP-relative global, reads byte
`+0x217`, combines it with the dispatcher's raw NOT byte at `+0x20C`, and then
executes the proved `+0x206/+0x209` reducer before `v0 = 0`.

PS2 window
`script_vm_matcher/artifacts/review/vcs_thread_state_conditions/003b2b20_linear_100.txt`
independently shows the active-script global load, `lbu +0x217`, the same raw
NOT handling, both AND/OR reducer arms and zero return.

The separately proved `0289` streaming opcode writes one to `+0x217` on its
new target before immediately calling `Process(target)`. This establishes a
direct VM-state relationship, while the source-level field name remains
unclaimed.

## Portable implementation

`ScriptThread::is_mission_script` is the portable semantic analogue of native
`+0x217`, explicitly documented as not being an original source-field name.
`04F2` reads that field and uses the existing exact reducer; it needs no host
adapter. Tests cover true, raw-NOT inversion and AND-group countdown behavior.
