# VCS PSP `MAIN.SCM` selector 0–9 child census

## Scope and guardrails

This is a **read-only host-contract census**, not a PPSSPP trace and not a
whole-game execution claim. For each selector in the static literal batch
0–9, the harness parses the original 99-entry mission offset table, copies
the selected original byte slice into its owned staging buffer, applies the
recovered `0289` child-marker/global-clear ordering and calls the portable
child VM.

The parent bytecode actually decodes selector 0 and then selector 1. The
other selector values are explicit `--mission-selector` census overrides; the
harness prints that override and never alters `MAIN.SCM`. Pickup adapters for
selector 0 are synthetic solely to traverse the already-proved `013F/01F9`
ABI. No synthetic model IDs, terrain heights, pickup handles, mission mode or
game state are evidence about the original title.

## Results

| Selected child | Commands before stop | Stop opcode | Result | Meaning of result |
| ---: | ---: | --- | --- | --- |
| 0 | 126 | `0023` | normal VM `YIELD`; 123 synthetic pickup calls | triage-only completion through the confirmed pickup and terminate contracts |
| 1 | 127 | `01E7` | explicit unsupported opcode | next static semantic target |
| 2 | 2 | `00A9` | explicit unsupported opcode | next static semantic target |
| 3 | 4 | `0145` | explicit unsupported opcode | next static semantic target |
| 4 | 2 | `01AA` | explicit unsupported opcode | next static semantic target |
| 5 | 77 | `0045` | `Opcode0045RequiresConditionProbe` | known character-liveness host boundary; VM ABI is implemented |
| 6 | 2 | `0296` | explicit unsupported opcode | next static semantic target |
| 7 | 1 | `03C6` | explicit unsupported opcode | next static semantic target |
| 8 | 1 | `01EA` | explicit unsupported opcode | next static semantic target |
| 9 | 5 | `0001` | normal VM `WAIT`/`YIELD` | no unknown VM opcode reached in this processing pass |

All children stage at the same active-buffer ScriptSpace base
`0x03A4D6`; that is the recovered `MainScriptSize` destination, not an
assertion that their source offsets are identical.

## Resulting evidence-driven queue

1. `01E7` is the furthest real child route reached before an unsupported VM
   opcode and is the next direct PSP/PS2 static target.
2. `00A9`, `0145`, `01AA`, `0296`, `03C6` and `01EA` follow as independent
   short-route targets.
3. `0045` is **not** an unknown opcode: it needs a real character-liveness
   engine predicate. Do not replace it with a synthetic boolean merely to
   inflate the census.
4. Resume selector 9 only with a clock/scheduling pass; its `WAIT` outcome is
   already correct VM behavior, not a fault.

Reproduce a row with:

```powershell
.\vcs_vm_core\build\Release\vcs_vm_mission_stream_harness.exe `
  '.\GTA VCS [Debug Build]\PSP_GAME\USRDIR\RUNDATA\MAIN.SCM' `
  --mission-selector 1 --synthetic-pickup-dry-run --mission-commands 1024
```

The `--mission-selector` line is intentionally reported by the tool as a
read-only host-census override together with the decoded parent selector.
