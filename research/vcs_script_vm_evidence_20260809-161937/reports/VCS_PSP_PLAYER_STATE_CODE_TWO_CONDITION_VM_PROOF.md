# VCS PSP/PS2 opcode `00A5`: player-state-code-two condition

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x001DA7C4` | `script_vm_matcher/artifacts/review/vcs_player_arrested/001da7c4_linear_112.txt` |
| PS2 | `0x002D2F60` | `script_vm_matcher/artifacts/review/vcs_player_arrested/002d2f60_FUN_002d2f60.c` |

Both handlers collect exactly one raw parameter and resolve it through a
player-table path. The recovered engine predicate is the same on both
platforms:

```text
base_condition = (resolved_player.state_byte == 2)
base_condition ^= ScriptThread.not_flag
reduce_condition_result(ScriptThread, base_condition)
return CONTINUE
```

The PSP window exposes the direct byte load from `+0xD0`, comparison with
literal `2`, raw `+0x20C` NOT xor, and the exact shared reducer at
`+0x206/+0x209`. The PS2 decompilation independently exposes the same sequence.
The state-byte value is proven; its original source-level meaning is not
promoted from a public command library label.

The portable VM owns decoding, raw NOT and reduction, and requires a host
`Opcode00A5PlayerStateCodeTwoProbe(player_selector)` only for the opaque player
object/state access. Missing host integration faults as
`Opcode00A5RequiresPlayerStateCodeTwoProbe`.
