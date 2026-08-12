# GTA Vice City Stories: semantic recompilation investigation

## Scope and evidence policy

This is a focused investigation of the **SLES_546.22** VCS PS2 Script VM,
correlated with VCS PSP, reLCS source semantics, PS2 decompilation and
targeted PPSSPP runtime captures. It is not a claim that all VCS systems are
ported or that every same-number PSP/PS2 opcode has identical behavior.

Only direct instruction/decompilation evidence is promoted to `gta.csv`.
Names beginning with `vcs_` are C++-safe semantic roles, not claims about
Rockstar's original source spellings. No ELF, ISO, decrypted executable, SCM
payload, APK, OBB or game asset is stored in this repository.

## Confirmed PS2 Script VM anchors

| PS2 address | Recompiler map name | Recovered role |
| ---: | --- | --- |
| `0x00256130` | `vcs_CRunningScript_Process` | Checks wake time and repeats command processing while native status is `0`. |
| `0x00256260` | `vcs_CRunningScript_ProcessOneCommand` | Fetches a 16-bit opcode, advances IP, strips the high NOT bit, selects an 8-byte descriptor and invokes its handler. |
| `0x00256308` | `vcs_ReturnFromGosubOrFunction` | Pops tagless GOSUB or CALL/CALL_NOT frames; CALL copies outputs and rolls back local-frame state. |
| `0x00255AD8` | `vcs_CTheScripts_StartNewScript` | Idle-head removal, monotonic process ID, Init, requested IP, active-list insertion and active flag. |
| `0x00255358` | `vcs_CRunningScript_Init` | Resets VM-visible stack/control/local state and observed mission markers. |
| `0x00255A78` / `0x00255AB0` | `vcs_CRunningScript_{Remove,Add}ScriptFromList` | Scheduler-list transitions used by the script factory. |
| `0x003B24E0` | `vcs_Opcode0289_MissionStreamAndImmediateProcess` | Selector remap, offset-table slice, script creation, mission markers/global clear and re-entrant child Process. |
| `0x003B2B20` | `vcs_Opcode04F2_ActiveScriptMissionMarkerCondition` | Restored function boundary; checks the active-script mission marker through shared condition machinery. |
| `0x0012C1E0` | `vcs_Opcode055A_ConditionReducer` | Shared raw-NOT/AND/OR/countdown condition-state reducer. |
| `0x002F4E88` / `0x002F4EA0` | `vcs_MissionStreamScope{Enter,Leave}` | Stream-scope nesting counter around mission I/O; leave finalizes on the zero transition. |

The dispatcher pipeline is structurally equivalent on PSP and PS2:

```text
thread.IP -> ScriptSpace + IP -> u16 opcode -> IP += 2
          -> raw NOT bit -> opcode & 0x7fff -> descriptor -> handler -> status
```

`0` means native **VM CONTINUE**, not an SCM output value. `WAIT` returns `1`
to yield the current pass; `NOP`, `GOTO` and `CALL` return `0` to continue it.

## Semantic leverage for static recompilation

The VCS table has 1,371 PSP slots (`0x0000..0x055A`), of which 1,328 are
present and common with PS2; PS2 appends ten slots `0x055B..0x0564`. The same
43 holes survive in the common range. This is strong VM-lineage evidence, but
not an automatic proof that all 1,328 handlers are semantically equal.

For the recompiler, the practical results are:

1. A descriptor-table legal target set can constrain the opcode-handler
   `JALR`, rather than treating it as unconstrained indirect control flow.
2. Confirmed `gta.csv` names make generated output and traces navigable without
   changing MIPS semantics.
3. A high-level replacement can be introduced at a verified boundary instead
   of translating the full interpreter, its CALL ABI and every opcode handler.
4. PSP runtime captures can act as a semantic oracle for IP, thread fields,
   handler status, parameter bytes and relevant writes.

## Hybrid-static-recomp experiment

The investigation suggests a third architecture alongside full static
recompilation and a complete reVC-style native port:

```text
recompiled VCS gameplay / AI / vehicle / camera code
    |-- Script VM            -> native semantic VM/HLE
    |-- disc/filesystem      -> native VFS/ISO backend
    |-- IOP RPC / SIF        -> targeted IOP HLE profile
    |-- RenderWare boundary  -> librw/native host where evidence permits
    |-- audio/platform APIs  -> targeted native adapters
```

This is a proposal, not an enabled override. The first safe experiment is
build-scoped and opt-in:

1. retain generated PS2 gameplay code;
2. add a game override for `0x00256130` and `0x00256260`;
3. bridge only the proved `ScriptThread`/ScriptSpace contract to a native VM;
4. compare IP, call frames, condition state and handler status with the PSP
   oracle at `NOP`, `WAIT`, `GOTO`, `CALL_NOT`, `CALL` and `0289`;
5. retain the recompiled fallback until state comparison is exact.

Do **not** intercept renderer, VU, DMA, IOP or audio merely because they are
inconvenient. Each future cut needs a documented semantic boundary and a
build-specific address binding.

## Current vertical slice and next boundary

A separate C++17 semantic VM prototype loads the original SCM container and
executes the main-script/mission scheduler. With diagnostic registries and the
evidenced model/zone maps, the PSP route passed an 8,192-frame soak with
15,763,781 commands, 58 active scripts, 6 idle scripts and no fault. This is
an empty-world compatibility route, not game playability.

The next port boundary is a real reVC/reLCS host implementation for the VM's
compiled gameplay interface: pool-backed peds/vehicles, wanted state, camera,
HUD and garages. Symbol work continues where it supports that integration or
high-frequency unimplemented MAIN.SCM commands; diagnostic registry breadth
is no longer treated as the final runtime architecture.
