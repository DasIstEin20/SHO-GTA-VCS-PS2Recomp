# VCS PSP/PS2 opcode `0107`: player-control host bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x001DA8A4` | `script_vm_matcher/artifacts/review/vcs_player_control/001da8a4_linear_208.txt` |
| PS2 | `0x002D30E8` | `script_vm_matcher/artifacts/review/vcs_player_control/002d30e8_FUN_002d30e8.c` |

Both handlers call `CollectParameters(..., 2, shared_parameter_buffer)`. They
resolve raw parameter zero through an engine player-table path, then split only
on whether raw parameter one is zero:

```text
PlayerControlAdapter(raw_parameter[0], raw_parameter[1] != 0)
```

Each branch invokes a larger platform-owned player sequence. Its directly
observed behavior includes an engine player/safety call, camera or reset calls,
and special-state checks; those callees are not source-named here. Both paths
return `v0 = 0` (`CONTINUE`) and neither invokes `StoreParameters` nor the VM
condition reducer.

The automatic PSP function boundary was truncated after the collector call, so
the forced-linear window is the evidence for its complete CFG. The PS2
decompilation independently exposes the same two-parameter and zero/non-zero
branch structure. reLCS `COMMAND_SET_PLAYER_CONTROL` is a structural reference
only; VCS host-side details remain platform integration. Missing host support
faults as `Opcode0107RequiresPlayerControlAdapter`.
