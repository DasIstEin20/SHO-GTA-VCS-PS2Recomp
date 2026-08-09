# Stories Script VM validation agent

Read `generated/stories_vm_research_context.json` before navigating candidates.

The active evidence chain is `reLCS source → LCS Android exact symbols → LCS
PSP → VCS PSP`. `REVIEW` means an address for human/Ghidra inspection, never a
name to apply. Require two independent proofs for a PSP identity: for example
script-IP/opcode semantics plus command-page callgraph, or normalized MIPS exact
plus a confirmed semantic anchor.

Keep RenderWare matches classified separately as `RENDERWARE`; do not turn a PC
FLIRT hit into a PSP-MIPS label. LCS PSP `0x00153A90` and VCS PSP
`0x0005E5E8` are manually confirmed Script-VM opcode dispatchers.  VCS is
descriptor-table driven, so `vcs_dispatcher_propagation.json` contains only
table-derived handler roles (`automatic_rename: false`).  Treat
`vcs_opcode_delta` as a structural slot comparison; never copy a same-number
LCS command name to a VCS handler without its own semantic proof.

The VCS PSP table range is fixed as `0x0000..0x055A` (1371 slots). Consult
`VCS_PSP_PS2_OPCODE_COMPARE.json` and
`VCS_PSP_PS2_MANUAL_SEMANTIC_PROOF.json` when moving between PSP and PS2:
`COMMON_SLOT` means both tables have a descriptor, not that the handlers have
the same semantics. The current audited comparison has 1328 `COMMON_SLOT`, 10
`PS2_ONLY`, and no PSP-only or manually proven-different handlers.

For every common handler, consult `VCS_PSP_PS2_COMMON_SEMANTICS.json`. It
records normalized decoder calls, thread offsets, global-access pattern,
constants, callee kinds, CFG shape and native handler-status constants for PSP
versus PS2. This final signal is interpreter ABI control flow, not a
script-command return value or output.
Its `STRUCTURAL_*` labels are prioritization signals only: only a manual proof
may turn a `COMMON_SLOT` into `DIFFERENT_HANDLER_SEMANTICS`.

Runtime observations are collected separately by
`tools/ppsspp_vcs_oracle/`.  Its stock-PPSSPP WebSocket capture is a PSP-side
oracle for naturally executed commands: it records the raw opcode and NOT bit,
dispatcher-selected handler, parameter-byte window, ScriptThread/IP deltas,
native `v0` VM-dispatch status, write-hit tracepoints for relevant ScriptThread fields and
configured memory-region deltas.  Read its JSONL telemetry
alongside the PS2 static evidence; a matching slot or one runtime observation
does not by itself prove cross-platform semantic equality.  Never use the
oracle to inject bytecode or patch a game, and do not promote an opcode beyond
the evidence its specific input path actually observed.

Do not describe a handler status as an SCM output. `Process()` repeats only
while the handler status is `0`: call that `CONTINUE`. A non-zero value stops
the current processing pass; `WAIT` status `1` is specifically a `YIELD`.
For the canonical initial opcode contracts, read
`VCS_PSP_INITIAL_OPCODE_CANONICAL_SEMANTICS.json`.

Read `tools/ppsspp_vcs_oracle/telemetry/ORACLE_STATUS.json` before making a
claim about capture coverage, then use
`telemetry/psp_runtime_vs_ps2_static.json` for the matching PS2 static row.

`vcs_vm_core/` is the evidence-gated portable reconstruction, not a binary
overlay. Its frozen pre-decoder baseline is
`vcs_vm_core/golden_baseline_20260809.json`; do not silently rewrite a baseline
fact. Consult `vcs_vm_core/evidence_coverage.json` before assuming a handler
exists. Unimplemented slots must fault explicitly. `055A` now has a complete
shared PSP/PS2 reducer proof (`reports/VCS_PSP_CONDITION_VM_PROOF.md`); only
its unnamed external engine predicate remains host-provided.

The VCS PSP parameter primitives are now manually identified:
`0x0005E71C` is `CollectNextParameterWithoutIncreasingPC`, `0x0005DA7C` is
`GetPointerToScriptVariable`, and `0x0005E890` is `StoreParameters`.
Read `reports/VCS_PSP_PARAMETER_ABI_PROOF.md` for exact tag ranges. Do not
erase the important `0x00` distinction: plural parameter collection consumes
it as an end marker, while the standalone no-PC-advance primitive routes it to
the low variable range.

Function `RETURN` is now independently proved: opcode `0026` targets
`0x002B7E88`, which calls `0x0005F09C`. It pops the CALL/CALL_NOT frame,
copies outputs from `locals[LP + input_count]`, restores LP, writes caller
destinations and flips condition state for CALL_NOT. Read
`reports/VCS_PSP_CALL_RETURN_ABI_PROOF.md`. Ordinary GOSUB is now separately
proved as a tagless post-parameter return-IP frame; read
`reports/VCS_PSP_GOSUB_RETURN_PROOF.md`. Do not infer function output behavior
for GOSUB.

`01BA GOSUB_FILE` is now direct PSP/PS2 VM proof in
`reports/VCS_PSP_GOSUB_FILE_VM_PROOF.md`. Both handlers collect exactly two
parameters, push the same tagless post-parameter return frame as GOSUB, then
transfer only through parameter 0 and return `CONTINUE`. Parameter 1 is
consumed but unread in the handler; keep its file-level meaning unnamed and do
not add a speculative host adapter. Its two target names are confirmed solely
as a byproduct of that direct semantic proof.

The next documented hot control-flow layer comes from the real VCS `MAIN.SCM`
static inventory (`VCS_PSP_MAIN_SCM_OPCODE_USAGE.json`). `IF` (`0078`),
`RETURN_TRUE` (`005E`) and `RETURN_FALSE` (`005F`) have complete PSP/PS2
condition-VM proof; `GOTO_IF_FALSE` (`0022`) has direct PS2 branch proof and a
PSP one-parameter collector entry. Read
`reports/VCS_PSP_CONTROL_FLOW_VM_PROOF.md`. The inventory is static and linear:
it must not be presented as runtime hotness/reachability, and its stop point is
an explicit data-boundary diagnostic.

The actual entrypoint command sequence is now exercised by the
read-only `vcs_vm_core/tools/MainScmPrefixHarness.cpp`; see
`reports/VCS_PSP_MAIN_SCM_PREFIX_VERTICAL_SLICE.md`. It loads a user-supplied
SCM, strips its eight-byte container header before constructing ScriptSpace,
then executes `0238 SCRIPT_NAME`, `00C9 DO_FADE`, `0401 DISABLE_PAUSE_MENU`,
the proven tagless `01BA GOSUB_FILE`, and `0006 SET_VAR_TEXT_LABEL`. The
header-origin rule is essential: target `0x007BFF` maps to file offset
`0x007C07`, not adjacent inline text. A 216-command route trace then reaches
the explicitly faulted `0289` mission-streaming subsystem boundary; read
`reports/VCS_PSP_MAIN_SCM_ENTRY_ROUTE.md`. Treat this only as
an integration proof for the recovered decoder/core: it is neither a PPSSPP
runtime trace nor a reachability claim, and it creates no new symbol name.

VCS has a decoder divergence that must outrank the reLCS structural reference:
PSP `CollectParameters` treats tag `0x00` as one raw zero value and continues,
whereas standalone `CollectNextParameterWithoutIncreasingPC` routes the same
tag through the low local-variable resolver. `0203 DISPLAY_TEXT` is the direct
regression proof. Never transplant the reLCS `ARGUMENT_END` convention into
VCS without a VCS call-path proof.

`0203 DISPLAY_TEXT` is now direct PSP/PS2 evidence in
`reports/VCS_PSP_DISPLAY_TEXT_VM_PROOF.md`: first collect two raw coordinate
payloads, then separately collect a raw text key/offset and enter the
host-owned GXT/text queue path. Its handler symbols are confirmed only at this
role level; no text-manager callee spelling is asserted.

`0268 CLEAR_HELP` is direct PSP/PS2 evidence in
`reports/VCS_PSP_CLEAR_HELP_VM_PROOF.md`: no parameters, manager ensure and
two native UI clear paths before `CONTINUE`. Model it as one explicit host
adapter; the PS2-only literal in its second internal callee is not a license to
invent a UI-state name.

`0049` is direct PSP/PS2 evidence in
`reports/VCS_PSP_DELETE_VEHICLE_VM_PROOF.md`: it consumes exactly one raw
vehicle handle, branches on native `+0x20A` into a cleanup path, then deletes a
non-null resolved entity and its references before `CONTINUE`. Keep pool,
cleanup-list and entity-lifetime APIs behind one host adapter; do not infer
their original callee spellings from a public opcode label.

`0113` is direct PSP/PS2 evidence in
`reports/VCS_PSP_VEHICLE_RELEASE_MARK_VM_PROOF.md`: it consumes exactly one
raw vehicle handle, invokes a distinct vehicle release-mark helper and only
uses native `+0x20A` for an additional context-cleanup path before
`CONTINUE`. It is deliberately **not** `0049`: keep the release policy, pool
and cleanup APIs behind a separate host adapter and do not promote the public
SCM wording into a native callee name.

`04E5` is direct PSP/PS2 evidence in
`reports/VCS_PSP_GLOBAL_BYTE_STORE_VM_PROOF.md`: it collects two raw dwords,
uses the first as offset into an engine-global byte region, writes the low byte
of the second, and returns `CONTINUE`. Use the neutral global-byte-store role;
the public Empire-related label is navigation context, not a proven native
subsystem name.

`0107` is direct PSP/PS2 evidence in
`reports/VCS_PSP_PLAYER_CONTROL_VM_PROOF.md`: it collects player selector plus
one raw control value, branches only on that value being zero, and enters a
large host player-safety/camera/reset sequence before `CONTINUE`. Keep it as
one `PlayerControlAdapter`; do not propagate names to its many unreviewed
engine callees.

`0044` is direct PSP/PS2 evidence in
`reports/VCS_PSP_CHARACTER_COORDINATE_VM_PROOF.md`: it consumes character
handle plus x/y/z binary32 payloads, and only the `z == -100.0` / `z <= -250.0`
range requests a native ground-height replacement before placement. Preserve
the raw payloads and sentinel in one host adapter; do not collapse it to an
unconditional `set_position` abstraction.

`00A5` is direct PSP/PS2 evidence in
`reports/VCS_PSP_PLAYER_STATE_CODE_TWO_CONDITION_VM_PROOF.md`: one player
selector is resolved, native state byte `+0xD0` is compared only to literal
`2`, then raw NOT and the common reducer execute. Keep the symbol role neutral
until an independent source-level state-code proof exists.

`0059` is direct PSP/PS2 evidence in
`reports/VCS_PSP_CLEAR_PRINTS_VM_PROOF.md`: no parameter decoding, one native
UI clear call, then `CONTINUE`. Model only that one host bridge; the native
callee's owning display subsystem is not a confirmed symbol.

`009F` is direct PSP/PS2 evidence in
`reports/VCS_PSP_CHARACTER_DEAD_CONDITION_VM_PROOF.md`: one character handle
feeds a multi-branch engine predicate (including null true and constants `1`,
`0x26`, `0x39`, `0x3A`), then raw NOT and the common reducer. Keep object
layout and status values in one host predicate, never as guessed C++ fields.

The first hot VM-only mutation batch is documented in
`reports/VCS_PSP_VARIABLE_OPS_PROOF.md`: `0004/0005/0006/0035/0036` are
raw-dword assignment aliases and `0007` is modulo-`2^32` integer addition.
`0037` is now separately confirmed as the exact same target as `0006` on both
tables, so it inherits only that proven raw-dword assignment role.
`0006` is public text-label syntax but the native handler still stores exactly
one collected dword; model literals as ScriptSpace offsets, not host pointers.
PSP proves resolver/collector setup and PS2 exposes the direct mutation. Treat these as
PS2-static/PSP-setup corroborated, not as an excuse to apply a generic name to
uninspected handlers.

The adjacent scalar arithmetic batch is now direct PSP/PS2 proof:
`0008/000A/000C/000E` respectively add, subtract, multiply and divide one
binary32 value; `0009/000B` subtract and multiply int32 values, with the
multiplication storing its low 32 result bits. `000D` is signed `div/mflo` for
normal operands. Read `reports/VCS_PSP_ARITHMETIC_VM_PROOF.md`. The portable
core explicitly faults for divisor zero and `INT_MIN / -1`, because the native
hardware-edge quotient has not been given a portable semantic contract. The
resulting helper roles may be added to the symbol map only because the direct
instruction windows prove them.

`002A..0030` are a separate, unusually strong dispatch fact: each aliases the
same corresponding `0008..000E` target in **both** PSP and PS2 tables. They are
implemented with the exact proven arithmetic semantics, but retain generic
`ARITHMETIC_DISPATCH_ALIAS_*` labels until a source-kind spelling is directly
proved. This is the intended distinction between a confirmed handler role and
a merely plausible public command name.

`0024 START_NEW_SCRIPT` now has direct PSP/PS2 proof in
`reports/VCS_PSP_START_NEW_SCRIPT_VM_PROOF.md`. It consumes **one** signed
script index (negative clamps to zero), requests a host-created child and then
consumes exactly **96** raw child-local values. Do not use the Sanny
`label, arguments` pseudo-arity to delimit native VCS bytecode here. The VM
owns the clamp and local copy; the child factory/lifetime/entry point stays an
explicit host adapter. This correction supersedes the earlier 21,277-command
linear inventory: the current evidence-bounded scan has 1,893 commands before
its first documented stop at `0x00A8C0`.

`0031..0034` are direct PSP/PS2 timed float arithmetic evidence in
`reports/VCS_PSP_TIMED_ARITHMETIC_VM_PROOF.md`. `0031/0032` share one native
target on both platforms, as do `0033/0034`: resolve one destination, collect
one raw binary32 value, multiply by the engine time-step scalar, add/subtract,
store and return `CONTINUE`. The scalar (`PSP gp+0x1E1C`, PS2
`DAT_004CD410`) is engine-owned. Keep it as an explicit host provider in the
portable VM; do not turn it into a guessed SCM parameter or wall-clock timer.
The two target names enter the symbol map only because the direct instruction
windows and cross-platform decompilations prove that role.

`0038..003B` are direct PSP/PS2 numeric VM evidence in
`reports/VCS_PSP_NUMERIC_VM_PROOF.md`. In particular, `0038` is native
`trunc.w.s` toward zero (not Sanny's floor wording), `003A` preserves
`INT_MIN` through `subu`, and `003B` preserves `-0`/NaN because it negates only
on strict float `< 0`. Their four confirmed handler roles are now in the
symbol map; do not manufacture an FPU-invalid conversion result for NaN or
out-of-range float→int inputs.

`0045 IS_CHAR_STILL_ALIVE` now has direct PSP/PS2 proof in
`reports/VCS_PSP_IS_CHAR_STILL_ALIVE_VM_PROOF.md`. The VM-owned contract is
one decoded handle, raw NOT and the already-proven condition reducer. Character
pool lookup, object layout and liveness state checks are engine-owned and must
come from the explicit host probe. The handler target's confirmed symbol role
therefore names only a character-liveness condition, not the hidden pool
helpers.

The first renderer boundary batch is `00C8/00C9/00CA`, documented in
`reports/VCS_PSP_FADE_VM_PROOF.md`. `00C8` consumes three dwords and passes
their low bytes to the fixed fade object; `00C9` consumes two, maps duration
`<=2` to `0.0f` otherwise `float * 0.001f`, and sign-narrows mode to `int16`;
`00CA` consumes no SCM parameters, queries fade status and feeds raw NOT into
the shared condition reducer. Keep all three fixed fade object operations as
host adapters. In particular, do not invent a renderer object layout or a
mode-value enum from the public command spelling alone.

`010E GET_GAME_TIMER` is fully VM-proved in
`reports/VCS_PSP_GAME_TIMER_VM_PROOF.md`. Both builds copy the game-clock raw
dword into `ScriptParams[0]` and call `StoreParameters(1)`. Reuse the VM's
injected clock abstraction (already required for WAIT) and preserve the raw
bits in the output destination; do not describe this as an opcode return
value.

`0377 GET_PAD_STATE` is direct PSP/PS2 evidence in
`reports/VCS_PSP_PAD_STATE_VM_PROOF.md`. Its real ABI is one selector input
followed by one `StoreParameters` output destination, despite the public list
showing two parameters. Only unsigned selectors `0..44` call the host pad
adapter; out-of-range raw selectors skip that dispatch and are written back
unchanged. Preserve this edge path and do not turn the 45 individual native
accessors into guessed public button names.

`0079 LAUNCH_MISSION` is direct PSP/PS2 evidence in
`reports/VCS_PSP_LAUNCH_MISSION_VM_PROOF.md`. It collects exactly one mission
ID, hands it to an engine resolver and writes one to byte `+0x20A` of the
returned target script object before native `CONTINUE`. Treat target lookup,
lifetime and request servicing as an explicit host adapter. The confirmed
symbol role records only the recovered request behavior; it does not invent a
source name for the native byte field.

`007F IS_BUTTON_PRESSED` is direct PSP/PS2 evidence in
`reports/VCS_PSP_BUTTON_PRESSED_VM_PROOF.md`. It collects exactly two raw
parameters, truncates both to `uint16` in source order for an engine input
predicate, then applies raw NOT and the established condition reducer. Keep
the two argument meanings and live input state host-owned: the confirmed
symbol names an input-predicate condition, not a guessed controller enum.

`0238 SCRIPT_NAME` is direct PSP/PS2 evidence in
`reports/VCS_PSP_SCRIPT_NAME_VM_PROOF.md`. It proves the parameter decoder's
direct ScriptSpace-string path reaches an exact eight-byte `strncpy` into the
thread field at `+0x20F`, followed by ASCII `A..Z` lowercasing in all eight
positions. Preserve it as a fixed byte array, not a generic string API; a
malformed pointer-like source must fail loudly rather than acquiring a guessed
native result.

`0289 LOAD_AND_LAUNCH_MISSION_INTERNAL` remains a coupled host boundary; read
`reports/VCS_PSP_MISSION_STREAM_LAUNCH_BOUNDARY.md` and
`reports/VCS_PC_BOOTSTRAP_VERTICAL_SLICE.md`. The core-alone handler still
faults after its one selector, but the separate read-only `VcsMissionHost`
now implements the recovered PC host ordering: selector remap, table-selected
range, scoped source-to-active-buffer copy, pooled `StartNewScript`, scope
leave, target markers/global clear and immediate `Process(child)`. It is
verified on an extracted PS2 tree and an ISO9660 PSP control. Its child fault
is deliberately propagated, not hidden: selector zero stops at `013F`'s
missing model-object resolver. Do not reduce any of this to a generic launch
callback or add a fake pickup/world result just to pass the boundary.

The first prerequisite is now direct rather than speculative: VCS PSP
`0x0006041C` is confirmed as
`CTheScripts::ReadMultiScriptFileOffsetsFromScript`; read
`reports/VCS_PSP_MISSION_OFFSET_TABLE_INIT_PROOF.md`. It parses the mission
metadata at `object_size + 8` and copies 32-bit mission offsets into the exact
table that `0289` indexes at `0x0004F340`. This name is earned by identical
parser semantics with the exact reLCS source and independently matching LCS
PSP code. It does not license a VCS PS2 source-level spelling.

The factory prerequisite is now also direct: VCS PSP `0x0005F470` and VCS PS2
`0x00255AD8` are confirmed as `CTheScripts::StartNewScript`; read
`reports/VCS_PSP_START_NEW_SCRIPT_FACTORY_PROOF.md`. Both remove the idle-head
thread, assign the prior monotonic process counter to `+0x08`, increment it,
run initialization, write requested IP to `+0x10`, insert into the active list
and set `+0x208 = 1`. Preserve this as the scheduler-host contract; do not
turn it into an unowned `new ScriptThread` shortcut or promote its internal
list/init callees without separate proof. That separate proof now exists:
`0x0005E3EC` / `0x00255358` are `CRunningScript::Init`,
`0x0005E6C0` / `0x00255A78` are `RemoveScriptFromList`, and
`0x0005E6F8` / `0x00255AB0` are `AddScriptToList`; see the same report. Init
zeros `+0x20A` and `+0x217`, proving that `0289` sets them deliberately after
constructing a recycled thread, though their original field spellings stay
unclaimed.

The remaining `0289` source/buffer path has a direct lifecycle contract too.
`0x0020D878` / `0x002F4E88` increment one stream-scope counter before the
offset-based I/O, and `0x0020D898` / `0x002F4EA0` decrement it after factory
creation; PS2 finalizes on the zero transition. The paired wrappers receive
the source object, `entry + 8`, destination `ScriptSpace + MainScriptSize` and
computed length. Keep their lower I/O implementation source-unnamed until
recovered separately. See `reports/VCS_PSP_MISSION_STREAM_LAUNCH_BOUNDARY.md`.

The bounded static `MAIN.SCM` scan now records every decoded `0289` first
parameter without resolving variables: all 11 sites are immediate literals
`0..9` with duplicate `8`. Treat this as a selector-census ordering aid, not
runtime reachability or native mission-mode evidence. Read
`reports/VCS_PSP_MAIN_SCM_MISSION_SELECTOR_STATIC_CLUSTER.md` before adding
more child harness routes.

The first read-only selector 0–9 child census is in
`reports/VCS_PSP_MAIN_SCM_MISSION_CHILD_CENSUS.md`. Its overrides do not claim
native reachability: they identify the next evidence-driven walls. Prioritize
`01E7` first (the furthest unsupported child route), then `00A9`, `0145`,
`01AA`, `0296`, `03C6` and `01EA`. `0045` is already a proven host predicate
boundary and must not be stubbed as a synthetic boolean merely to continue.

`04F2 IS_THIS_SCRIPT_A_MISSION` is direct PSP/PS2 VM state proof in
`reports/VCS_PSP_MISSION_THREAD_FLAG_VM_PROOF.md`: it reads the global active
script's `+0x217` byte and sends that predicate through raw NOT and the common
condition reducer. Use the portable `is_mission_script` name only as an
analogue, never as a claim about Rockstar's member spelling.

The first direct PSP/PS2 integer-condition batch is documented in
`reports/VCS_PSP_INTEGER_CONDITION_VM_PROOF.md`: `000F/0010/0011` evaluate
signed `param[0] > param[1]`, `0015/0016/0017` evaluate signed `>=`, and
`001B/001C/02DB` evaluate equality. Their PSP continuations are proven with
review-only forced-linear MIPS windows after `CollectParameters(2)`, not
inferred from a target hash.
Each passes raw NOT through the common condition reducer and returns native
`CONTINUE`. The generic symbol roles in the confirmed map name predicates, not
unverified public opcode aliases.

The adjacent float families are now also direct PSP/PS2 evidence:
`0012/0013/0014` use native `!(left <= right)` and `0018/0019/001A` use
`!(left < right)`, each before raw NOT and the common reducer. `001D/001E`
share the raw-dword XOR equality core, so float equality is bitwise rather than
numeric. Read `reports/VCS_PSP_FLOAT_CONDITION_VM_PROOF.md`; do not replace
these branch forms with an unreviewed generic float comparator.

`013F CREATE_PICKUP` is the first newly reached mission opcode after the
evidence-gated `0289` handoff. The PSP forced-linear window at `0x001DDCD0`
and PS2 manual decompilation at `0x002D7CE0` prove five inputs, negative
model-object indirection, the `z == -100 || z <= -250` terrain-height branch
with native `+0.5f`, pickup creation and `StoreParameters(1)`. The portable
core owns those ABI transforms; model-object table, world query and pickup
pool remain narrow host adapters. See `reports/VCS_PSP_CREATE_PICKUP_VM_PROOF.md`.

`01F9 CREATE_PICKUP_WITH_AMMO` is direct PSP/PS2 evidence at
`0x001DE044` / `0x002D8050`: it is the same pickup construction path with
`CollectParameters(6)` and parameter 2 forwarded as ammo. Keep both opcodes
on one host pickup API with `ammo = 0` for `013F`; do not split their
model-object or ground-Z semantics. `0023` at `0x002B7D38` / `0x003B2040`
is also confirmed: no parameters, mission-running clear only when `+0x217`
is set, active→idle scheduler transfer, active flag clear and native status 1
`YIELD`. The core must leave list/global ownership at an explicit scheduler
adapter. See `reports/VCS_PSP_TERMINATE_SCRIPT_VM_PROOF.md`.
