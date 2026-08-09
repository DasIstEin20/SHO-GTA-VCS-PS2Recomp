# VCS PSP/PS2 opcode `009F`: character-dead condition bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x00323E98` | `script_vm_matcher/artifacts/review/vcs_char_dead/00323e98_linear_160.txt` |
| PS2 | `0x00413968` | `script_vm_matcher/artifacts/review/vcs_char_dead/00413968_FUN_00413968.c` |

Both handlers collect exactly one raw character handle, compute an engine-owned
dead-state predicate, then perform the standard VM sequence: raw NOT from
`ScriptThread +0x20C`, shared reduction at `+0x206/+0x209`, and `v0 = 0`
(`CONTINUE`).

The directly recovered predicate includes a null-handle true case. For a
non-null entity it selects a native object path and tests state values including
`1`, `0x26`, `0x39`, and `0x3A`. Those numeric object/status meanings are not
treated as portable binary layout; the host owns the predicate implementation.

`Opcode009FCharacterDeadProbe(handle)` is therefore the only host boundary.
The VM itself owns parameter consumption, NOT and condition aggregation.
Missing host support faults as `Opcode009FRequiresCharacterDeadProbe`.
