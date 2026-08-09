# VCS PSP/PS2 opcode `0044`: character-coordinate placement bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x003232F8` | `script_vm_matcher/artifacts/review/vcs_char_coordinates/003232f8_linear_128.txt` |
| PS2 | `0x00412DE8` | `script_vm_matcher/artifacts/review/vcs_char_coordinates/00412de8_FUN_00412de8.c` |

Both handlers call `CollectParameters(..., 4, shared_parameter_buffer)` and
use the payload as one character handle plus three IEEE-754 binary32 values:

```text
character = resolve_character(raw_parameter[0])
x, y, z = raw_parameter[1], raw_parameter[2], raw_parameter[3]
if z == -100.0f or z <= -250.0f:
    z = engine_ground_height(x, y)
engine_character_coordinate_placement(thread, character, x, y, z, thread+0x20F)
return CONTINUE
```

The PSP forced-linear window explicitly proves the four-parameter collector,
the `-100.0` equality / `-250.0` less-or-equal sentinel tests, the x/y ground
query and the final placement call. The PS2 decompilation independently shows
the same three values, sentinel condition, resolved replacement z and thread
name-field argument. The vector preparation, terrain lookup and placement
callees remain host-owned.

`CharacterCoordinateAdapter(handle, x_raw, y_raw, z_raw,
resolve_ground_height)` preserves all raw binary32 payloads and makes the
sentinel decision explicit. It does not invent a source-level spelling for any
engine helper. Missing host integration faults as
`Opcode0044RequiresCharacterCoordinateAdapter`.
