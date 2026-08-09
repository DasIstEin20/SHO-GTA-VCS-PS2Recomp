# VCS Script VM core

This is a portable, deliberately narrow C++17 reconstruction of the verified
VCS Script VM core. It is a staging library for a PC port, not a claim that
the remaining 1,284 PSP handlers are reconstructed.

Machine-readable implementation boundaries are in `evidence_coverage.json`.

## Implemented and evidenced

- 16-bit little-endian opcode fetch, IP advance, high `NOT` bit storage and
  `0x7fff` normalization.
- A `0x0000..0x055A` handler namespace with unimplemented slots rejecting
  explicitly rather than masquerading as a successful script yield.
- Interpreter ABI: handler status `0` is `Continue`, while a non-zero status
  stops the current processing pass. `WAIT` status `1` is `Yield`.
- `0000 NOP`, `0001 WAIT`, `0002 GOTO`, `0022 GOTO_IF_FALSE`, `0024 START_NEW_SCRIPT`
  (host-owned child factory plus VM-proved 96-local initialization), `0025 GOSUB`,
  `0026 RETURN`, `005E RETURN_TRUE`, `005F RETURN_FALSE`, `0078 IF`,
  host-mission-launch-adapter-gated `0079 LAUNCH_MISSION`,
  host-input-predicate-gated `007F IS_BUTTON_PRESSED` (two low-16-bit inputs
  and the exact shared condition reducer),
  host-pause-menu-adapter-gated `0401 DISABLE_PAUSE_MENU` (one collected
  parameter normalized to zero/non-zero),
  host-text-adapter-gated `0203 DISPLAY_TEXT` (two raw coordinate payloads
  plus a separately collected raw text key/offset), `01BA GOSUB_FILE` (two consumed parameters but only parameter 0 used as the
  tagless GOSUB target), host-help-adapter-gated `0268 CLEAR_HELP` (no SCM
  parameters; host owns the two native UI clear paths),
  fade-adapter-gated `00C8 SET_FADING_COLOUR`, `00C9 DO_FADE`, `00CA GET_FADING_STATUS`,
  `010E GET_GAME_TIMER` (one raw-dword output via `StoreParameters`),
  `0238 SCRIPT_NAME` (exact fixed eight-byte copy and ASCII case conversion),
  `04F2 IS_THIS_SCRIPT_A_MISSION` (native `+0x217` marker through the exact
  condition VM),
  host-vehicle-delete-adapter-gated `0049` (one raw vehicle handle plus the
  observed native `+0x20A` context bit),
  host-vehicle-release-mark-adapter-gated `0113` (one raw vehicle handle plus
  the observed native `+0x20A` context bit; deliberately separate from
  destructive `0049`),
  host-pickup-adapter-gated `013F CREATE_PICKUP` / `01F9 CREATE_PICKUP_WITH_AMMO`
  (model object, low-byte type, optional ammo and three binary32 coordinates;
  preserves native negative-model and ground-Z sentinel paths before one
  handle output),
  host-scheduler-adapter-gated `0023 TERMINATE_THIS_SCRIPT` (no parameters;
  proven mission-state/list handoff followed by `YIELD`),
  host-global-byte-store-adapter-gated `04E5` (two raw dwords: byte-region
  offset and low-byte value),
  host-player-control-adapter-gated `0107` (player-table selector plus a
  zero/non-zero enable value),
  host-character-coordinate-adapter-gated `0044` (one handle, three binary32
  coordinates and the native ground-height sentinel),
  host-player-state-code-two-probe-gated `00A5` (one selector, raw NOT and the
  exact shared condition reducer),
  host-clear-prints-adapter-gated `0059` (no SCM parameters),
  host-character-dead-probe-gated `009F` (one handle, raw NOT and exact shared
  condition reducer),
  input-adapter-gated `0377 GET_PAD_STATE` (one selector plus one output;
  native unsigned out-of-range pass-through),
  `037A CALL_NOT`, `037B CALL`; raw-dword assignment aliases `0004`, `0005`,
  `0006`/`0037` text-label raw-dword assignment; `0035`, `0036`; scalar arithmetic
  `0007` through `000E` plus direct-target aliases `002A` through `0030`
  (with explicit portable faults for integer division's unportable native edge
  cases); time-step-adapter-gated timed float arithmetic `0031` through `0034`;
  numeric conversion and absolute-value operations `0038` through
  `003B`; and direct integer-condition
  handlers `000F`, `0010`, `0011`, `0015`, `0016`, `0017`, `001B`, `001C`,
  `02DB`; engine-probe-gated `0045 IS_CHAR_STILL_ALIVE`; and float-condition handlers `0012`, `0013`, `0014`, `0018`,
  `0019`, `001A`, `001D`, `001E` (including MIPS unordered and raw-equality
  behavior).
- Confirmed CALL frame tags: `CALL = 0x00800000`,
  `CALL_NOT = 0x00C00000`.
- The parameter forms evidenced by the PSP decoder primitives: zero/integer/
  float raw forms; signed `INT8`/`INT16`; local variables and local arrays;
  global variables and global arrays; explicit parameter output storage. Float
  values remain raw `int32` bit patterns by design. String/GXT adapter paths
  still need their own opcode-level proof.

`055A` now implements its exact PSP/PS2 condition reducer: raw NOT application,
AND/OR selection, countdown and reset. Its underlying engine predicate is still
injected through a narrow host probe, because the external game probes have not
earned semantic names. See `reports/VCS_PSP_CONDITION_VM_PROOF.md`.

The core is intentionally a semantic model, not a packed binary `CRunningScript`
overlay. Platform glue owns the conversion between its real thread structure
and `ScriptThread`.

## Evidence boundary

The implementation is constrained by:

- `VCS_PSP_INITIAL_OPCODE_CANONICAL_SEMANTICS.json`
- `VCS_PSP_PS2_MANUAL_SEMANTIC_PROOF.json`
- `reports/VCS_PSP_DISPATCHER_SEMANTIC_PROOF.md`
- `reports/VCS_PSP_PARAMETER_ABI_PROOF.md`
- `reports/VCS_PSP_CALL_RETURN_ABI_PROOF.md`
- `reports/VCS_PSP_GOSUB_RETURN_PROOF.md`
- `reports/VCS_PSP_GOSUB_FILE_VM_PROOF.md`
- `reports/VCS_PSP_IS_CHAR_STILL_ALIVE_VM_PROOF.md`
- `reports/VCS_PSP_DELETE_VEHICLE_VM_PROOF.md`
- `reports/VCS_PSP_VEHICLE_RELEASE_MARK_VM_PROOF.md`
- `reports/VCS_PSP_CREATE_PICKUP_VM_PROOF.md`
- `reports/VCS_PSP_TERMINATE_SCRIPT_VM_PROOF.md`
- `reports/VCS_PSP_GLOBAL_BYTE_STORE_VM_PROOF.md`
- `reports/VCS_PSP_PLAYER_CONTROL_VM_PROOF.md`
- `reports/VCS_PSP_CHARACTER_COORDINATE_VM_PROOF.md`
- `reports/VCS_PSP_PLAYER_STATE_CODE_TWO_CONDITION_VM_PROOF.md`
- `reports/VCS_PSP_CLEAR_PRINTS_VM_PROOF.md`
- `reports/VCS_PSP_CHARACTER_DEAD_CONDITION_VM_PROOF.md`
- `reports/VCS_PSP_FADE_VM_PROOF.md`
- `reports/VCS_PSP_GAME_TIMER_VM_PROOF.md`
- `reports/VCS_PSP_PAD_STATE_VM_PROOF.md`
- `reports/VCS_PSP_LAUNCH_MISSION_VM_PROOF.md`
- `reports/VCS_PSP_BUTTON_PRESSED_VM_PROOF.md`
- `reports/VCS_PSP_SCRIPT_NAME_VM_PROOF.md`
- `reports/VCS_PSP_MAIN_SCM_PREFIX_VERTICAL_SLICE.md`
- `reports/VCS_PSP_DISABLE_PAUSE_MENU_VM_PROOF.md`
- `reports/VCS_PSP_DISPLAY_TEXT_VM_PROOF.md`
- `reports/VCS_PSP_CLEAR_HELP_VM_PROOF.md`
- `reports/VCS_PSP_MISSION_THREAD_FLAG_VM_PROOF.md`
- `reports/VCS_PSP_CONDITION_VM_PROOF.md`
- `reports/VCS_PSP_INTEGER_CONDITION_VM_PROOF.md`
- `reports/VCS_PSP_FLOAT_CONDITION_VM_PROOF.md`
- `reports/VCS_PSP_CONTROL_FLOW_VM_PROOF.md`
- `reports/VCS_PSP_VARIABLE_OPS_PROOF.md`
- `reports/VCS_PSP_ARITHMETIC_VM_PROOF.md`
- `reports/VCS_PSP_START_NEW_SCRIPT_VM_PROOF.md`
- `reports/VCS_PSP_NUMERIC_VM_PROOF.md`
- `reports/VCS_PSP_TIMED_ARITHMETIC_VM_PROOF.md`
- `tools/ppsspp_vcs_oracle/telemetry/*.jsonl`

The CALL byte layout is additionally corroborated by the captured PSP byte
windows: the three header bytes, an `INT32` target and the input parameters
reproduce the observed transfer target for `037A/037B`. The PSP `RETURN` core
now additionally proves function-frame pop, output copying, locals rollback
and the CALL_NOT condition flip. The tagless GOSUB frame is independently
proved and tested; it does not inherit function-frame output behavior.

## Build and test

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Read-only VCS PC bootstrap vertical slice

`gta-vcs-bootstrap` is a small integration executable, not an emulator and
not a game launcher. Its one supported path is deliberately narrow:

```text
ISO9660/extracted VCS disc source
    -> MAIN.SCM
    -> ScriptSpace + portable VM
    -> recovered 0289 mission-stream host
    -> real selected child script
    -> first explicit VM/host boundary
```

The `vcs_mission_host` library contains only the evidenced pieces needed for
that handoff:

- `IVcsFileSystem`, `DirectoryBackend`, and `Ps2IsoBackend` (a read-only
  ISO9660 PVD/directory reader; no mounting or extracted-data writes).
- `VcsScriptContainer`, which parses the eight-byte SCM header and the exact
  metadata/offset-table form confirmed for
  `ReadMultiScriptFileOffsetsFromScript`.
- `IVcsMissionHost`/`VcsMissionHost`, which binds opcode `0289` with its
  scoped offset-range copy, preallocated idle-to-active child factory, marker
  writes, global clear and re-entrant child `Process()` ordering.

It intentionally has no model resolver, world, renderer, texture loader,
audio or generic asset adapter. Thus the expected first run is a meaningful
non-zero `FAIL LOUDLY`, currently the evidenced `013F CREATE_PICKUP` model
resolver boundary—not a false claim that the game booted.

```powershell
# Extracted PS2 disc tree (MAIN.SCM at its root)
.\build\Release\gta-vcs-bootstrap.exe '<extracted-vcs-ps2-dir>'

# PS2 ISO9660 image; default lookup is MAIN.SCM
.\build\Release\gta-vcs-bootstrap.exe '<VCS_PS2.iso>'

# Optional structural guard for the locally evidenced SLES_546.22 SCM layout.
# It checks SCM/container fields, not a content hash or disc ownership.
.\build\Release\gta-vcs-bootstrap.exe '<VCS_PS2.iso>' `
  --require-known-ps2-layout

# A PSP ISO is also useful to validate the standard ISO9660 reader, but the
# path is PSP-specific and this does not claim it is a PS2 execution proof.
.\build\Release\gta-vcs-bootstrap.exe '<VCS_PSP.iso>' `
  --main 'PSP_GAME/USRDIR/RUNDATA/MAIN.SCM'
```

Exit code `3` means the program reached a deliberate evidence boundary. The
output names both the parent opcode and (when applicable) the exact child
opcode/fault that needs the next host adapter. See
`../reports/VCS_PC_BOOTSTRAP_VERTICAL_SLICE.md` for the verified routes and
their limits.

## Read-only real-MAIN.SCM prefix proof

The `vcs_vm_main_scm_prefix_harness` executable is deliberately separate from
the unit tests. Given a local, user-supplied VCS `MAIN.SCM`, it removes the
eight-byte SCM header before constructing `ScriptSpace`, finds the unique
`SCRIPT_NAME "MAIN"` entry marker and executes a bounded prefix without
modifying the SCM or launching a game. Its recording adapters cover `DO_FADE`,
pause-menu and display-text. The first five-command proof exercises the actual
entry prefix (`0238 SCRIPT_NAME`, `00C9 DO_FADE`, `0401 DISABLE_PAUSE_MENU`,
`01BA GOSUB_FILE`, `0006 SET_VAR_TEXT_LABEL`); it reports the normalized script
name, VM-relative IP, native opcode, GOSUB frame depth and recorded host-boundary calls.

```powershell
.\build\Release\vcs_vm_main_scm_prefix_harness.exe `
  '..\GTA VCS [Debug Build]\PSP_GAME\USRDIR\RUNDATA\MAIN.SCM'
```

It expects `InstructionBudgetExhausted` after exactly the requested command
count; that is the intentional successful stop boundary, not a VM failure.
Passing `--commands N` exposes the next evidence-gated boundary, but does not
turn one control-flow route into a whole-program reachability claim.

## Read-only first-mission transfer proof

`vcs_vm_mission_stream_harness` is a separate host-contract harness for the
first `0289` encountered by the real `MAIN.SCM` route. It keeps `0289` in the
core as an explicit boundary, then reconstructs only its proven host ordering:
parse the mission offset table, copy one selected source slice into the active
buffer at `MainScriptSize`, clear the proved global region, set the two target
markers and immediately process the child. It never writes the input SCM.

```powershell
.\build\Release\vcs_vm_mission_stream_harness.exe `
  '..\GTA VCS [Debug Build]\PSP_GAME\USRDIR\RUNDATA\MAIN.SCM' `
  --mission-mode 0
```

`--mission-mode` is an explicit host fixture; it is not a claim about a
particular live PPSSPP session. On the known source it reaches the real first
mission `013F CREATE_PICKUP` boundary after child `SCRIPT_NAME`.

Do not register a real game opcode handler until its individual semantic proof
exists. `register_handler()` is the intended evidence-gated extension point.
`set_start_new_script_adapter()`, `set_time_step_provider()` and the `055A`
condition probe are intentionally narrow host boundaries, not invented engine
implementations.
