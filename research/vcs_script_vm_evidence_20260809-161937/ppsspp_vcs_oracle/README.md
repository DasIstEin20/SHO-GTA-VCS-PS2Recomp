# PPSSPP VCS Script-VM Oracle

This is a read-only runtime oracle for the decrypted VCS PSP debug ELF.  It
captures an opcode only when the running game reaches it naturally.  It does
not patch PPSSPP, alter the EBOOT, inject scripts, or leave breakpoints behind.

The initial proof set is `0000 NOP`, `0001 WAIT`, `0002 GOTO`, `037A CALL_NOT`,
`037B CALL`, and `055A`.  The configuration is deliberately tied to the
statically confirmed dispatcher at RVA `0x0005E5E8`; its runtime load base is
found by scanning PPSSPP user RAM for the dispatcher instruction signature.
It uses a conditional breakpoint at the instruction where the dispatcher has
already fetched the opcode into `a1` but has not advanced the ScriptThread IP,
so unrelated opcode dispatches continue without a host-side round trip.

## Prerequisites

- A **stock `PPSSPPHeadless.exe`** build.  The installed desktop release at
  `tools/ppsspp_win_1_20_4` is intentionally not used as a fallback, because it
  launches a GUI instead of a reproducible headless session.
- PPSSPP's remote debugger enabled in its config before startup:

  ```ini
  RemoteDebuggerOnStartup = True
  RemoteDebuggerLocal = True
  RemoteISOPort = 16482
  ```

  The debugger endpoint is `ws://127.0.0.1:16482/debugger`.
- The game must be running and reach the selected opcode.  The oracle does not
  fabricate execution of rare commands.

The local desktop PPSSPP configuration has been prepared for that endpoint.
If PPSSPP was already open when it was changed, restart it before invoking
`oracle.py status`. Its desktop executable is not accepted by
`run_headless.ps1`; that runner remains reserved for a supplied stock
`PPSSPPHeadless.exe` build.

## Run

```powershell
.\run_headless.ps1 -PpssppHeadless C:\tools\PPSSPPHeadless.exe -Game 'R:\Codex-workspace-misc\gta-vcs\GTA VCS [Debug Build]'
python .\oracle.py status
python .\capture_opcode.py 0x0001
python .\oracle.py compare-static
```

`capture_opcode.py` records the runtime load base, dispatcher, ScriptThread,
script-bytecode base, raw and normalized opcode, NOT bit, parameter-byte
window, dispatch descriptor and handler, IP before/after, and the `v0` **VM
dispatch status** (not a script-command return value), plus a byte and field
diff of the first `0x220` ScriptThread bytes. `0` means `CONTINUE`: the caller
immediately processes another opcode. A non-zero status stops this Process pass;
for the proven `WAIT` case, `1` is `YIELD`. It also adds temporary,
non-pausing PPSSPP `memory.breakpoint` write tracepoints for dynamic
ScriptThread IP/NOT and the proof case's expected fields; their hit counters
prove writes even when a value was rewritten unchanged. Optional
`watch_regions` in `breakpoints.json` add global-memory snapshot diffs.

Output is JSONL under `telemetry/`.  `oracle.py compare-static` produces
`telemetry/psp_runtime_vs_ps2_static.json`, linking actual PSP observations to
the PS2 handler database and manual semantic proof.  A snapshot diff is
truthfully an observed memory-state change, while the separate tracepoint hit
counts record same-value writes as well. Existing user memory breakpoints at
the exact same address and size are never replaced; that field is reported as
unavailable rather than losing the user's historical hit count.
