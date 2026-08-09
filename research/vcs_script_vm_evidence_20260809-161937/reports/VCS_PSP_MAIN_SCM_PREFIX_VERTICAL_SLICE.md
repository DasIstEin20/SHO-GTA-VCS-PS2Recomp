# VCS PSP `MAIN.SCM` bounded execution route

## Correct ScriptSpace origin

The VCS `MAIN.SCM` file begins with an eight-byte container header. Native
`CRunningScript` IPs and in-bytecode control-flow targets are relative to the
first byte **after** that header. The portable harness now strips the header
before constructing `ScriptSpace`.

This matters concretely: the entry's `01BA` target `0x007BFF` maps to file
offset `0x007C07`, which begins a valid `0006` command. Treating the file
header as script data shifted all targets by eight and falsely read adjacent
ASCII text as opcode `0x4843`. That older result is retired, not counted as an
unsupported command.

## Five-command integration proof

```text
entry_ip=0x7860 final_ip=0x7C0C last_opcode=0x0006
executed=5 fault=InstructionBudgetExhausted script_name=main call_depth=1
recorded_do_fade_calls=1 fade_seconds=0 fade_mode=0
recorded_pause_menu_calls=1 pause_menu_parameter_is_nonzero=true
recorded_display_text_calls=0
```

`InstructionBudgetExhausted` is the intentional success boundary. The VM
executed actual original bytecode, with IPs in native ScriptSpace coordinates:

| ScriptSpace extent | File extent | Opcode | Observed portable effect |
| --- | --- | --- | --- |
| `0x007860..0x007868` | `0x007868..0x007870` | `0238 SCRIPT_NAME` | exact eight-byte field becomes `main` |
| `0x007868..0x00786D` | `0x007870..0x007875` | `00C9 DO_FADE` | recording adapter receives `0.0f`, `int16(0)` |
| `0x00786D..0x007871` | `0x007875..0x007879` | `0401 DISABLE_PAUSE_MENU` | recording adapter receives `true` |
| `0x007871..0x007BFF` | `0x007879..0x007885` | `01BA GOSUB_FILE` | tagless return frame; VM-relative target `0x007BFF` |
| `0x007BFF..0x007C0C` | `0x007C07..0x007C14` | `0006 SET_VAR_TEXT_LABEL` | decodes a global destination and inline `CH_JERR` ScriptSpace string |

The last row validates both the header-origin correction and the direct
NUL-string parameter decoder: the next bytes after its NUL terminator begin
the following valid `0006` record rather than being treated as an opcode from
inside the string.

## First real execution wall

With `--commands 256`, the core follows this same route for 216 commands and
stops at a known, explicitly modeled boundary:

```text
entry_ip=0x7860 final_ip=0x788C last_opcode=0x0289
executed=216 fault=Opcode0289RequiresMissionStreamingSubsystem
script_name=main call_depth=0
```

`0289` is not an arbitrary missing slot. Direct PSP/PS2 proof shows that it
decodes one selector and then requires the coupled mission offset table,
stream/copy buffer, script factory, request flags, global mission state and
re-entrant `Process(target)` call. The core consumes the proved selector and
fails with that specific boundary fault; it does not misrepresent the subsystem
as `LaunchMission(id)`. See
`reports/VCS_PSP_MISSION_STREAM_LAUNCH_BOUNDARY.md`.

## Reproducibility and limits

- Input: `GTA VCS [Debug Build]/PSP_GAME/USRDIR/RUNDATA/MAIN.SCM`
- SHA-256: `4d278aeaadd2565c5c82c52df42c132191cc57fe06230ff5d529d6aad51dfc67`
- Header `MainScriptSize` (post-header ScriptSpace bytes): `0x03A4D6`
- File offset immediately after MAIN: `0x03A4DE` (`8 + MainScriptSize`)
- Unique `SCRIPT_NAME "MAIN"` file offset: `0x007868`; VM IP: `0x007860`
- Read-only harness source: `vcs_vm_core/tools/MainScmPrefixHarness.cpp`

This is an integration proof for one statically chosen control-flow route. It
does not show that this route is naturally reached in a particular PPSSPP
session, prove the whole `MAIN.SCM`, or reconstruct the mission streaming host
subsystem. It makes no writes to the SCM and never launches or patches a game.
