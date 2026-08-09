# VCS PSP/PS2 `0024 START_NEW_SCRIPT` VM proof

`0024` is a VM/host boundary, not an ordinary two-parameter command. The
public Sanny entry represents it as `label, arguments`, but both native
handlers expose its real bytecode consumption:

1. `CollectParameters(thread, &thread.IP, 1, ScriptParams)` reads a signed
   script index.
2. A negative index is replaced with zero.
3. An engine-owned routine creates/returns the child script thread for that
   index.
4. `CollectParameters(thread, &thread.IP, 0x60, child + 0x54)` copies exactly
   96 raw local values into the child's local-variable region.
5. The handler returns `v0 = 0` (`CONTINUE`).

| Platform | Opcode target | Direct evidence |
| --- | --- | --- |
| PSP | `0x002B7D84` | forced-linear MIPS shows `a2=1`, clamp with `bltz`, child factory call, then `a2=0x60`, destination `v0+0x54`, and `v0=0` |
| PS2 | `0x003B2098` | decompilation independently shows the same `1`, negative clamp, factory result + `0x54`, `0x60`, and `return 0` sequence |

The portable `vcs_vm_core` exposes only the required host boundary:
`StartNewScriptAdapter(ScriptVm&, int32_t script_index) -> ScriptThread*`.
The adapter owns creation, lifetime and entry-IP initialization; the proven VM
core clamps the index, invokes it, decodes exactly 96 child-local values and
copies them at local slot zero. Missing/null adapters fault explicitly.

This discovery also corrects the static SCM inventory decoder. It now uses a
direct-evidence width override of **97 encoded parameters** for `0024` (one
index + 96 locals), rather than treating the public `arguments` pseudo-field as
one ordinary bytecode parameter. The inventory remains static linear evidence,
not a reachability or runtime-frequency claim.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_start_new_script/002b7d84_linear_48.txt`
- `script_vm_matcher/artifacts/review/vcs_start_new_script/003b2098_FUN_003b2098.c`
- `script_vm_matcher/artifacts/review/vcs_start_new_script/003b2098_instruction_window.txt`
