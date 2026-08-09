# VCS PSP MAIN.SCM opcode usage

This is a static, linear inventory of the header-delimited main segment. It is not runtime hotness or a reachability claim.

- Decoded commands: `1893`
- Fully decoded main segment: `False`
- CALL/CALL_NOT sites: `98`
- Static `0289` sites: `11`
- Final cursor: `0x00A8C2`

## Stop condition

```json
{
  "offset": "0x00A8C0",
  "raw_opcode": "0x120D",
  "normalized_opcode": "0x120D",
  "reason": "opcode is absent from the VCS public command library"
}
```

## Static `0289` selector encodings

File offsets below are static scan coordinates, not execution reachability or native mission-mode values.

| File offset | Raw opcode | Tag | Selector | Classification |
| ---: | --- | --- | ---: | --- |
| `0x007891` | `0x0289` | `0x01` | 0 | `direct_integer_literal` |
| `0x007894` | `0x0289` | `0x07` | 1 | `direct_integer_literal` |
| `0x007898` | `0x0289` | `0x07` | 2 | `direct_integer_literal` |
| `0x00789C` | `0x0289` | `0x07` | 3 | `direct_integer_literal` |
| `0x0078A0` | `0x0289` | `0x07` | 4 | `direct_integer_literal` |
| `0x0078A4` | `0x0289` | `0x07` | 5 | `direct_integer_literal` |
| `0x0078A8` | `0x0289` | `0x07` | 6 | `direct_integer_literal` |
| `0x007A51` | `0x0289` | `0x07` | 7 | `direct_integer_literal` |
| `0x007AF6` | `0x0289` | `0x07` | 8 | `direct_integer_literal` |
| `0x007B8E` | `0x0289` | `0x07` | 8 | `direct_integer_literal` |
| `0x007BB8` | `0x0289` | `0x07` | 9 | `direct_integer_literal` |

## Most frequent decoded opcodes

| Opcode | Name | Static occurrences | Condition |
| --- | --- | ---: | --- |
| `0x0022` | `GOTO_IF_FALSE` | 298 | False |
| `0x0004` | `SET_VAR_INT` | 285 | False |
| `0x0005` | `SET_VAR_FLOAT` | 193 | False |
| `0x001B` | `IS_INT_VAR_EQUAL_TO_NUMBER` | 184 | True |
| `0x0002` | `GOTO` | 178 | False |
| `0x037B` | `CALL` | 96 | False |
| `0x0026` | `RETURN` | 75 | False |
| `0x0036` | `SET_VAR_FLOAT_TO_VAR_FLOAT` | 49 | False |
| `0x01BA` | `GOSUB_FILE` | 39 | False |
| `0x005E` | `RETURN_TRUE` | 27 | True |
| `0x0001` | `WAIT` | 24 | False |
| `0x0035` | `SET_VAR_INT_TO_VAR_INT` | 23 | False |
| `0x0045` | `IS_CHAR_STILL_ALIVE` | 20 | True |
| `0x002E` | `MULT_FLOAT_VAR_BY_FLOAT_VAR` | 19 | False |
| `0x005F` | `RETURN_FALSE` | 18 | True |
| `0x0007` | `ADD_VAL_TO_INT_VAR` | 16 | False |
| `0x00C9` | `DO_FADE` | 15 | False |
| `0x002C` | `SUB_FLOAT_VAR_FROM_FLOAT_VAR` | 14 | False |
| `0x0006` | `SET_VAR_TEXT_LABEL` | 12 | False |
| `0x0289` | `LOAD_AND_LAUNCH_MISSION_INTERNAL` | 11 | False |
| `0x0078` | `IF` | 11 | False |
| `0x000F` | `IS_INT_VAR_GREATER_THAN_NUMBER` | 11 | True |
| `0x0011` | `IS_INT_VAR_GREATER_THAN_INT_VAR` | 11 | True |
| `0x0079` | `LAUNCH_MISSION` | 9 | False |
| `0x00C8` | `SET_FADING_COLOUR` | 9 | False |
| `0x0010` | `IS_NUMBER_GREATER_THAN_INT_VAR` | 8 | True |
| `0x00CA` | `GET_FADING_STATUS` | 8 | True |
| `0x0009` | `SUB_VAL_FROM_INT_VAR` | 8 | False |
| `0x002A` | `ADD_FLOAT_VAR_TO_FLOAT_VAR` | 8 | False |
| `0x0377` | `GET_PAD_STATE` | 7 | False |
| `0x0107` | `SET_PLAYER_CONTROL` | 6 | False |
| `0x04E5` | `SET_NUMBER_OF_EMPIRE_BUSINESS_SITES_OWNED` | 6 | False |
| `0x010E` | `GET_GAME_TIMER` | 6 | False |
| `0x0049` | `DELETE_CAR` | 6 | False |
| `0x0030` | `DIV_FLOAT_VAR_BY_FLOAT_VAR` | 6 | False |
| `0x0013` | `IS_NUMBER_GREATER_THAN_FLOAT_VAR` | 6 | True |
| `0x001C` | `IS_INT_VAR_EQUAL_TO_INT_VAR` | 5 | True |
| `0x0029` | `ADD_INT_VAR_TO_INT_VAR` | 5 | False |
| `0x0039` | `CSET_VAR_FLOAT_TO_VAR_INT` | 5 | False |
| `0x0401` | `DISABLE_PAUSE_MENU` | 4 | False |
| `0x007F` | `IS_BUTTON_PRESSED` | 4 | True |
| `0x0268` | `CLEAR_HELP` | 4 | False |
| `0x00A5` | `HAS_PLAYER_BEEN_ARRESTED` | 4 | True |
| `0x0037` | `SET_VAR_TEXT_LABEL_TO_VAR_TEXT_LABEL` | 4 | False |
| `0x03F8` | `STORE_PLAYER_OUTFIT` | 4 | False |
| `0x0044` | `SET_CHAR_COORDINATES` | 4 | False |
| `0x02E2` | `SET_VAR_INT_TO_CONSTANT` | 4 | False |
| `0x001D` | `IS_FLOAT_VAR_EQUAL_TO_NUMBER` | 4 | True |
| `0x000C` | `MULT_FLOAT_VAR_BY_VAL` | 4 | False |
| `0x0017` | `IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR` | 4 | True |
| `0x009F` | `IS_CHAR_DEAD` | 3 | True |
| `0x0015` | `IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER` | 3 | True |
| `0x0059` | `CLEAR_PRINTS` | 3 | False |
| `0x0113` | `MARK_CAR_AS_NO_LONGER_NEEDED` | 3 | False |
| `0x001E` | `IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR` | 3 | True |
| `0x0014` | `IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR` | 3 | True |
| `0x0038` | `CSET_VAR_INT_TO_VAR_FLOAT` | 3 | False |
| `0x0012` | `IS_FLOAT_VAR_GREATER_THAN_NUMBER` | 3 | True |
| `0x0238` | `SCRIPT_NAME` | 2 | False |
| `0x0455` | `ADD_EMPIRE_HUD_TEXT_ELEMENT_WITH_NUMBERS` | 2 | False |
| `0x0318` | `IS_PLAYER_WEARING` | 2 | True |
| `0x0458` | `SET_EMPIRE_HUD_TEXT_ELEMENT_SIZE` | 2 | False |
| `0x0457` | `SET_EMPIRE_HUD_ELEMENT_COLOUR` | 2 | False |
| `0x00B9` | `SET_FIXED_CAMERA_POSITION` | 2 | False |
| `0x00B3` | `POINT_CAMERA_AT_PLAYER` | 2 | False |
| `0x034B` | `DOES_VEHICLE_EXIST` | 2 | True |
| `0x00A0` | `IS_CAR_DEAD` | 2 | True |
| `0x007C` | `IS_CHAR_IN_CAR` | 2 | True |
| `0x01FF` | `SET_FREE_RESPRAYS` | 2 | False |
| `0x02F8` | `FREEZE_CHAR_POSITION` | 2 | False |
| `0x0530` | `DISABLE_SPLASH_SCREEN_WHEN_MOVING_BETWEEN_LEVELS` | 2 | False |
| `0x009D` | `HAS_DEATHARREST_BEEN_EXECUTED` | 2 | True |
| `0x002B` | `SUB_INT_VAR_FROM_INT_VAR` | 2 | False |
| `0x001A` | `IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR` | 2 | True |
| `0x0373` | `GET_DOT_PRODUCT_3D` | 2 | False |
| `0x037A` | `CALL_NOT` | 2 | False |
| `0x03FD` | `IS_FINAL_GAME` | 1 | True |
| `0x03E5` | `IS_DEVELOPER` | 1 | True |
| `0x03E6` | `SET_DEVELOPER_FLAG` | 1 | False |
| `0x0456` | `REMOVE_EMPIRE_HUD_ELEMENT` | 1 | False |
| `0x044E` | `SET_EMPIRE_HUD_ELEMENT_VISIBLE_STATE` | 1 | False |
| `0x0451` | `SET_EMPIRE_HUD_ELEMENT_SIZE` | 1 | False |
| `0x0450` | `SET_EMPIRE_HUD_ELEMENT_START_POSITION` | 1 | False |
| `0x0452` | `SET_EMPIRE_HUD_ELEMENT_BACKGROUND_COLOUR` | 1 | False |
| `0x0453` | `SET_EMPIRE_HUD_ELEMENT_BORDER_COLOUR` | 1 | False |
| `0x0454` | `ADD_EMPIRE_HUD_TEXT_ELEMENT` | 1 | False |
| `0x007E` | `IS_CHAR_IN_ANY_CAR` | 1 | True |
| `0x0216` | `WARP_CHAR_FROM_CAR_TO_COORD` | 1 | False |
| `0x020F` | `UNDRESS_CHAR` | 1 | False |
| `0x03F2` | `UNLOCK_OUTFIT` | 1 | False |
| `0x04F3` | `LOCK_THE_COSTUME` | 1 | False |
| `0x0352` | `CLEAR_ALL_CHAR_ANIMS` | 1 | False |
| `0x028D` | `SET_NEAR_CLIP` | 1 | False |
| `0x0008` | `ADD_VAL_TO_FLOAT_VAR` | 1 | False |
| `0x01BB` | `GET_GROUND_Z_FOR_3D_COORD` | 1 | False |
| `0x00CF` | `SET_CHAR_HEADING` | 1 | False |
| `0x02EE` | `GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS` | 1 | False |
| `0x0407` | `PRINT_HELP_FOREVER_NO_BRIEF` | 1 | False |
| `0x0323` | `PRINT_HELP_FOREVER` | 1 | False |
| `0x0057` | `PRINT_NOW` | 1 | False |
