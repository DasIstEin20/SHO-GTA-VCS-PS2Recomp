# VCS PSP/PS2 `0023` terminate-this-script scheduler bridge

| Opcode | VCS PSP target | VCS PS2 target |
| --- | ---: | ---: |
| `0023` | `0x002B7D38` | `0x003B2040` |

## Direct behavior

Both manual decompilations are complete and agree exactly:

```text
if thread.byte_0x217 != 0:
    global_mission_running_byte = 0
host_remove_from_active_script_list(thread)
host_add_to_idle_script_list(thread)
thread.byte_0x208 = 0
return YIELD (1)
```

There are no SCM parameters or output destinations. The return is native VM
status `1`, so it stops the current `Process()` pass. It is not a script-level
numeric output.

The VCS command corpus's `TERMINATE_THIS_SCRIPT` label agrees with the direct
control-flow evidence but is not the primary naming source.

## Portable boundary

The core records `ScriptThread::is_active = false` and
`ScriptThread::terminated = true`, then returns `YIELD`. A narrow scheduler
adapter owns the native active-list removal, idle-list insertion and global
mission-running byte. It receives the proven `+0x217` marker; the marker's
source-level spelling is not inferred.

Missing host scheduler support faults explicitly rather than pretending a
thread was terminated. A later `process(thread)` on an inactive/terminated
portable thread yields without decoding another opcode.

## Evidence

- `script_vm_matcher/artifacts/review/vcs_terminate_script/002b7d38_FUN_002b7d38.c`
- `script_vm_matcher/artifacts/review/vcs_terminate_script/002b7d38_linear_64.txt`
- `script_vm_matcher/artifacts/review/vcs_terminate_script/003b2040_FUN_003b2040.c`
- `script_vm_matcher/artifacts/review/vcs_terminate_script/003b2040_linear_64.txt`
- `research_sources/reLCS_origin_lcs/src/control/Script.cpp`
