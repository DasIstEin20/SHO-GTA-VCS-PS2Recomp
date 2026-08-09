# VCS MAIN.SCM bounded-prefix core coverage

This is coverage over the static linear inventory only — not a runtime 
hotness, reachability, or whole-file-execution claim.

## Boundary

- Successfully decoded commands: **1893**
- Cursor after last decoded command: `0x00A8C2`
- Fully decoded: **False**
- First stop: `0x120D` at `0x00A8C0` — opcode is absent from the VCS public command library

## Command coverage

| Category | Commands | Distinct opcodes | Meaning |
| --- | ---: | ---: | --- |
| Fully implemented | 1687 | 47 | Portable core has a direct evidenced contract. |
| Adapter gated | 116 | 18 | VM contract is implemented; an explicit host dependency remains. |
| Explicit fault | 90 | 63 | No executable core handler, or a deliberate coupled host boundary; VM faults rather than pretending success. |

## Highest-frequency still-unimplemented opcodes

| Opcode | Public navigation label | Static count |
| --- | --- | ---: |
| `0x0289` | `LOAD_AND_LAUNCH_MISSION_INTERNAL` | 11 |
| `0x03F8` | `STORE_PLAYER_OUTFIT` | 4 |
| `0x007C` | `IS_CHAR_IN_CAR` | 2 |
| `0x009D` | `HAS_DEATHARREST_BEEN_EXECUTED` | 2 |
| `0x00A0` | `IS_CAR_DEAD` | 2 |
| `0x00B3` | `POINT_CAMERA_AT_PLAYER` | 2 |
| `0x00B9` | `SET_FIXED_CAMERA_POSITION` | 2 |
| `0x01FF` | `SET_FREE_RESPRAYS` | 2 |
| `0x02F8` | `FREEZE_CHAR_POSITION` | 2 |
| `0x0318` | `IS_PLAYER_WEARING` | 2 |
| `0x034B` | `DOES_VEHICLE_EXIST` | 2 |
| `0x0373` | `GET_DOT_PRODUCT_3D` | 2 |
| `0x0455` | `ADD_EMPIRE_HUD_TEXT_ELEMENT_WITH_NUMBERS` | 2 |
| `0x0457` | `SET_EMPIRE_HUD_ELEMENT_COLOUR` | 2 |
| `0x0458` | `SET_EMPIRE_HUD_TEXT_ELEMENT_SIZE` | 2 |
| `0x0530` | `DISABLE_SPLASH_SCREEN_WHEN_MOVING_BETWEEN_LEVELS` | 2 |
| `0x0043` | `GET_CHAR_COORDINATES` | 1 |
| `0x0048` | `CREATE_CAR` | 1 |
| `0x0055` | `PRINT_BIG` | 1 |
| `0x0057` | `PRINT_NOW` | 1 |

The public labels above aid navigation only. They are not promoted to symbol-map names without direct per-handler proof.
