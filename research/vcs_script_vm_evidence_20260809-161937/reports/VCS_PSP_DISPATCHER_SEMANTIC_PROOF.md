# VCS PSP ProcessOneCommand semantic proof

Status: `CALLGRAPH_CONFIRMED`; no Ghidra label was written automatically. This proof is from the manual Ghidra decompilation of `0x0005e5e8` and its caller `0x0005e4a4`.

| Step | Semantic | Manual decompilation evidence |
| --- | --- | --- |
| 1 | read IP | `*(int *)(param_1 + 0x10)` selects the active bytecode offset |
| 2 | derive/fetch | `*(short *)(_DAT_ffff3534 + IP)` reads the 16-bit opcode from ScriptSpace |
| 3 | advance | `*(int *)(param_1 + 0x10) = ... + 2` advances the IP by one opcode |
| 4 | NOT bit | `uVar3 & 0x8000` writes the flag to `param_1 + 0x20c` |
| 5 | strip NOT | `uVar3 = uVar3 & 0x7fff` produces the dispatch index |
| 6 | dispatch | `DAT_00380ab0 + opcode * 8` supplies descriptor/offset; `PTR_LAB_00380ab4[opcode * 2]` supplies the handler; `(*pcVar4)(param_1)` calls it |
| 7 | VM dispatch status | the handler's `v0` status is propagated to the caller; `0x0005e4a4` executes `do { iVar2 = FUN_0005e5e8(param_1); } while (iVar2 == 0)`. Thus `0` is `CONTINUE` for the interpreter loop, not an SCM output; non-zero stops the current Process pass. |

VCS is not an LCS-style direct page-call dispatcher: it uses a table of eight-byte records plus indirect targets. `vcs_dispatcher_propagation.json` therefore exposes roles/ranges only, with `automatic_rename: false`.
