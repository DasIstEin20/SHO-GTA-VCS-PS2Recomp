# Evidence-gated Stories Script VM research

The active chain is:

`reLCS source → LCS Android exact C++ symbols → LCS PSP MIPS → VCS PSP MIPS`

It starts exclusively from the Script VM. No game binary is changed and no
match is written back into Ghidra automatically.

## Active workflow

The static Android anchor list is in
`config/android_lcs_script_vm_anchors.json`. It requires the known Android
input hash and exact agreement of address, namespace and C++ symbol.

`ghidra/ExportNormalizedMips.java` produces a relocation-tolerant MIPS
fingerprint retaining opcode, register role, small constants, memory widths
(through opcode), control flow and delay slots. It neutralizes direct branch,
call, data and GP/LUI-ORI address material.

`ghidra/ExportScriptVmSemanticSignals.java` is a triage export only.  The
confirmation gate is manual decompilation.  For VCS,
`ExportOpcodeDispatchTable.java` exports the indirect opcode descriptor table
used by the confirmed dispatcher.

After Ghidra exports exist, run:

```powershell
.\run-evidence-chain.ps1
```

For all `COMMON_SLOT` PSP/PS2 opcode handlers, run the second, read-only
semantic-shape stage:

```powershell
.\run-common-handler-semantics.ps1
```

It exports and compares parameter-decoder calls, direct `CRunningScript`
offset accesses, global read/write patterns, small constants, normalized callee
kinds, approximate CFG shape, and return constants. Its score is triage only:
an address or feature mismatch never automatically marks an opcode as
`DIFFERENT_HANDLER_SEMANTICS`.

The pipeline writes `REVIEW` candidates freely. It writes a confirmed mapping
only after independent semantic/callgraph validation. An identical MIPS hash
without such an anchor is still not eligible for a rename.

## PSP runtime oracle

`../tools/ppsspp_vcs_oracle/` adds an independent, read-only PSP runtime
evidence source. It uses stock PPSSPP's debugger WebSocket API and captures
only naturally executed script commands; it never patches the EBOOT, game
memory, bytecode or emulator. The initial capture set is `0000`, `0001`,
`0002`, `037A`, `037B` and `055A`. Each JSONL observation records the
dispatcher-selected runtime handler, input-byte window, ScriptThread/IP state
diff, native `v0` VM-dispatch status and optional watched-memory deltas. A
status of `0` is `CONTINUE` for the interpreter loop, not a script-level
output; the proven `WAIT` status `1` is `YIELD`. `oracle.py
compare-static` pairs those observations with the PS2 handler DB and manual
proof, but intentionally makes no automatic cross-platform equality claim.

## Key outputs

- `../android_lcs_exact_symbols.json`
- `../lcs_psp_script_vm_candidates.json` and `_confirmed.json`
- `../vcs_psp_script_vm_candidates.json` and `_confirmed.json`
- `../stories_vm_map.json`
- `../vcs_opcode_delta.{json,csv,md}`
- `../vcs_dispatcher_propagation.json`
- `../VCS_PSP_PS2_COMMON_SEMANTICS.json` and
  `../reports/VCS_PSP_PS2_COMMON_SEMANTICS.md`
- `../reports/` and `../agents/generated/stories_vm_research_context.*`
- `renderware_symbols/canonical_rw_symbols.json`

VCS `CRunningScript::ProcessOneCommand` is confirmed at `0x0005E5E8`: it reads
the IP, fetches and advances a 16-bit opcode, handles `0x8000`, strips
`0x7fff`, then calls an opcode-table target.  Its caller at `0x0005E4A4` loops
while the handler returns zero.  VCS is table-driven rather than calling LCS
range/page functions directly, so propagation records handler *roles* with
`automatic_rename: false`; it never assigns a same-number LCS command name.
The opcode delta is a structural navigation table, not a proof that numeric
LCS and VCS opcodes have identical semantics.

The VCS PSP table is intentionally exported only through `0x0000..0x055A`
(1371 slots). `VCS_PSP_OPCODE_TABLE.json` compares against the independently
confirmed PS2 table `0x0000..0x0564` in `VCS_PSP_PS2_OPCODE_COMPARE.json`:
1328 `COMMON_SLOT`, 0 `PSP_ONLY`, 10 `PS2_ONLY`, and 0 manually proven
`DIFFERENT_HANDLER_SEMANTICS`. `COMMON_SLOT` says only that both tables contain
a descriptor; manual semantic proof remains required before treating two
handlers as equal.

## Legacy material

`run-four-way.ps1` and `artifacts/four_way_*` are preserved for provenance,
but are quarantined as `UNCONFIRMED_REVIEW`: they used fuzzy matching and an
invalid first stage involving `ReLCS.dll`. Do not treat them as mappings.
