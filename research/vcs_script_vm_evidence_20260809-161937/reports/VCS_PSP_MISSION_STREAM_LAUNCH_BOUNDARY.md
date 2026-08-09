# VCS PSP/PS2 `0289` mission-stream-and-process boundary

| Opcode | Public navigation label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `0289` | `LOAD_AND_LAUNCH_MISSION_INTERNAL` | `0x002B8400` | `0x003B24E0` |

## Directly recovered behavior

Both implementations collect exactly one raw selector and return native
`CONTINUE` (`0`). Unlike `0079`, this opcode is not a narrow Script VM call;
it is a mission-streaming pipeline that constructs and immediately processes
another script object.

The PS2 decompilation and PSP forced-linear window prove this sequence:

```text
selector = CollectParameters(1)[0]

if host_mission_mode > 0:
    if selector < 0xFFFE:
        return CONTINUE
    selector = 0xFFFF - selector

host_prepare_mission_stream()
range = host_mission_offset_table[selector .. selector + 1]
host_copy_script_slice_to_active_buffer(range)
target = host_create_script_from_active_buffer()
host_finish_mission_stream()
target->byte_0x20A = 1
target->byte_0x217 = 1
host_mission_active_flag = 1
zero ScriptSpace[NumTrueGlobals * 4 + 8, MostGlobals * 4]
CRunningScript::Process(target)
return CONTINUE
```

The names `host_*` describe verified boundaries, not source-function names.
The precise semantic meanings of the mode, two target bytes, table contents
and buffer lifetime have not been claimed. The global mission-active byte and
parsed-global clear are separately proven below.

## Concrete stream-transfer facts required by a PC host

The following is instruction/decompilation evidence, not a guessed file-format
API:

| Stage | PSP observation | PS2 observation | Portable requirement |
| --- | --- | --- | --- |
| Offset table | table base is the fixed `0x0004F340` region; entries are indexed as `selector * 4` | table base is `0x007407D0`; entries are indexed as `selector * 4` | preserve a 32-bit selector-indexed offset table, not a generic mission-ID map |
| Start/length arguments | native computes `entry[selector] + 8` and `entry[selector + 1] - entry[selector]` | `FUN_001791B8(..., entry[selector] + 8, 0)` then `FUN_00179188(..., entry[selector + 1] - entry[selector])` | retain both observed numeric arguments until the script container's eight-byte subheader is independently decoded |
| Non-positive length | falls back to a fixed engine scalar (`gp + 0x1F70`) | falls back to `DAT_004CD6E8` | model this as an explicit fallback-size input, not an empty slice |
| Source/buffer | distinct prepare call, source-base call and destination-buffer copy call | `FUN_002F4E88`, source `DAT_00487718`, destination `DAT_0048771C + DAT_004CD6D8` | own source handle, destination buffer and their lifecycle separately |
| Factory/finalize | copy is followed by `CTheScripts::StartNewScript` then a distinct finalize call | `CTheScripts::StartNewScript(DAT_004CD6D8)` then `FUN_002F4EA0()` | reproduce the proven idle→active factory contract; no unowned `ScriptThread` allocation shortcut |
| Mission-global clear | after both target bytes and the global active flag, arguments are `ScriptSpace + NumTrueGlobals*4 + 8`, `0`, `MostGlobals*4` | `FUN_0045D318(DAT_0048771C + DAT_004CD704*4 + 8, 0, DAT_004CD706 << 2)` | preserve exact source-order global clear before re-entrant child `Process()` |

For the reached selector `0`, the VM has proved only that the selector decodes
as raw zero. It has not read the live native table values or inferred the
mission-mode value, so a portable host must obtain the appropriate offset-table
and mode state from its own reconstructed script subsystem.

The VCS PSP initializer for this same table is now directly confirmed as
`ReadMultiScriptFileOffsetsFromScript` at `0x0006041C`; see
`reports/VCS_PSP_MISSION_OFFSET_TABLE_INIT_PROOF.md`.
The directly called PSP/PS2 factory is now separately confirmed as
`CTheScripts::StartNewScript` at `0x0005F470` / `0x00255AD8`; see
`reports/VCS_PSP_START_NEW_SCRIPT_FACTORY_PROOF.md`.

## Stream-scope and I/O wrapper proof

The formerly opaque prepare/finalize calls are now direct, small functions:

| Role name | PSP | PS2 | Direct fact |
| --- | ---: | ---: | --- |
| `MissionStreamScopeEnter` | `0x0020D878` | `0x002F4E88` | increments one global nesting counter before offset-table I/O |
| `MissionStreamScopeLeave` | `0x0020D898` | `0x002F4EA0` | decrements that counter after `StartNewScript`; the PS2 implementation calls a separate finalizer when it becomes zero |
| source-position wrapper | `0x0006D3F4` | `0x001791B8` | forwards the source object, `entry[selector] + 8` and zero mode to a lower I/O operation and returns its boolean result |
| source-to-active-buffer wrapper | `0x0006D3D0` | `0x00179188` | forwards the source object, `ScriptSpace + MainScriptSize` destination and computed length to a lower copy/read operation |

The direct artifacts are in
`script_vm_matcher/artifacts/review/vcs_mission_stream_primitives/`. The two
scope names are descriptive roles, not claims about original source spelling.
The lower I/O helpers are intentionally not source-named: the wrapper
arguments prove positioning and range transfer, while their broader file or
stream subsystem identity has not been independently recovered.

This narrows the host requirement further. A PC host must preserve a nesting
scope around source positioning/copy and execute its finalize-on-last-leave
behavior before setting the target request bytes. It must not replace the
sequence with an unscoped `read_file_slice` convenience call.

## Cross-platform evidence

PSP window
`script_vm_matcher/artifacts/review/vcs_mission_launch/002b8400_linear_140.txt`
shows `CollectParameters(1)`, the signed mode check, the `0xFFFD` comparison
and `0xFFFF - selector` mapping, code-range calculation from an offset table,
two buffer-copy calls, a script-object factory, byte-one writes at `+0x20A`
and `+0x217`, a global byte-one write, the exact parsed-global clear and
direct call to confirmed `CRunningScript::Process` at `0x0005E4A4`.

PS2 decompilation
`script_vm_matcher/artifacts/review/vcs_mission_launch/003b24e0_FUN_003b24e0.c`
independently recovers the same selector guard/remap, table-difference code
slice, target construction, two byte-one writes, global state write,
parsed-global clear, immediate `Process(target)` and zero return.

## Deliberate implementation boundary

`0289` is registered in `vcs_vm_core` only as a **selector-decoding explicit
fault**: `Opcode0289RequiresMissionStreamingSubsystem`. This preserves the
exact bytecode/IP boundary but deliberately does not call a generic adapter.
A correct PC implementation needs an owned mission-offset table, code-buffer
lifecycle, script factory, the two observed target request fields, the global
active byte plus exact parsed-global clear and a re-entrant target `Process()`
scheduling contract. Collapsing that into a generic `launch_mission(selector)`
callback would hide the very behavior that remains to be ported.

The read-only portable route from the true `MAIN.SCM` entry reaches this exact
boundary after 216 commands with its encoded selector equal to raw zero. The
separate host-contract harness then stages its real offset-table slice and
processes the child through `0238` to the first new `013F` boundary; resuming
the parent exposes the next `0289` selector as raw one. These observations
validate decoder/control-flow and container data only; they are not PPSSPP
runtime evidence for the native mission-mode value or streaming side effects.
Artifacts: `VCS_PSP_MAIN_SCM_ENTRY_ROUTE.json`,
`reports/VCS_PSP_MAIN_SCM_MISSION_STREAM_ROUTE.md` and
`reports/VCS_PSP_MAIN_SCM_MISSION_STREAM_TRACE.jsonl`.

This report therefore turns the opcode from an unnamed hole into a precise
integration boundary without falsely increasing portable-core coverage.

## External host vertical-slice status

The core's default handler remains the selector-decoding explicit fault above.
As of the PC-bootstrap vertical slice, the coupling is now reconstructed in a
separate `VcsMissionHost`, rather than hidden inside `ScriptVm` or rewritten as
`LaunchMission(id)`. It accepts a `VcsScriptContainer` from a read-only
directory/ISO9660 backend and preserves the proved order:

```text
selector -> table range -> scope enter -> source-to-active-buffer copy
         -> StartNewScript pooled factory -> scope leave
         -> +0x20A / +0x217 -> mission/global clear -> Process(child)
```

On the locally available extracted PS2 tree this reaches selector zero after
225 parent commands (`MAIN.SCM` main size `0x3CAE3`, entry `0x780C`) and the
real child reaches `013F` after two child commands. The PSP ISO9660 control
reaches the equivalent first child wall after 216 parent commands. In both
cases the missing `013F` model-object resolver is deliberately returned as the
child's precise fault; no pickup, world or mission success state is fabricated.
See `reports/VCS_PC_BOOTSTRAP_VERTICAL_SLICE.md`.
