# VCS PSP mission-offset table initializer

## Confirmed identity

| Stage | Address | Evidence |
| --- | --- | --- |
| reLCS source | `CTheScripts::ReadMultiScriptFileOffsetsFromScript` | `research_sources/reLCS_origin_lcs/src/control/Script5.cpp` |
| LCS Android | `0x0031EAC0` | exact C++ symbol anchor |
| LCS PSP | `0x00079028` | `script_vm_matcher/artifacts/review/lcs_mission_offset_init/00079028_FUN_00079028.c/00079028_FUN_00079028.c` |
| VCS PSP | `0x0006041C` | `script_vm_matcher/artifacts/review/vcs_mission_offset_init/0006041c_FUN_0006041c.c/0006041c_FUN_0006041c.c` |

The LCS PSP and VCS PSP decompilations have the same concrete, no-call parser
shape as the exact reLCS source. This is direct semantic confirmation, not a
histogram-based promotion.

## Recovered ScriptSpace layout

Let `S` denote the native ScriptSpace base. The routine obtains a 32-bit
`object_size` from `S + (read_u32(S + 3) + 3)` and then reads the following
metadata from `S + object_size + 8`:

| Relative offset | Width | Confirmed consumer role |
| ---: | ---: | --- |
| `+0x08` | `u16` | true-global count used later to clear the mission global region |
| `+0x0A` | `u16` | global-region extent used later to clear the mission global region |
| `+0x0C` | `u32` | largest-mission-script fallback size |
| `+0x10` | signed `u16` | number of table dwords copied |
| `+0x12` | `u16` | exclusive-mission count/mode guard input |
| `+0x14 + 4*i` | `u32` | mission script offset table entry `i` |

For each `i` below the signed count, VCS PSP copies one little-endian dword to
the fixed `MultiScriptArray`-role region at `0x0004F340 + 4*i`. Opcode `0289`
then directly indexes this same region as `selector * 4` and reads entries
`selector` and `selector + 1`.

## Portable implications

1. `0289`'s table is parsed from the original SCM container; it must not be
   guessed from mission command IDs.
2. The existing eight-byte ScriptSpace/file-origin distinction is reinforced:
   this parser reaches metadata via the container's own offset arithmetic,
   while `0289` seeks a selected subscript at `offset + 8`.
3. A safe PC implementation may bounds-check the copied table and the
   `selector + 1` lookup, but must preserve the proven native table values and
   `largest_mission_script_size` fallback behavior.
4. `0289` consumes the two parsed global counts directly: it clears exactly
   `MostGlobals * 4` bytes beginning at `ScriptSpace + NumTrueGlobals * 4 + 8`
   after setting its target flags/global mission-active byte and before its
   re-entrant child `Process()` call. This is confirmed by the PSP/PS2
   argument sequence and exact reLCS source, not a guessed mission context API.

No VCS PS2 source-level name is assigned here. The VCS PSP name is earned by
the direct parser equality to the exact LCS source and the independently
matching LCS PSP implementation.
