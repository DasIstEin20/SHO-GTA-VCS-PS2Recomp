# VCS PSP/PS2 `01BA` tagless two-parameter GOSUB proof

## Entry points

| Opcode | Public library label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `01BA` | `GOSUB_FILE` | `0x002B82F8` | `0x003B23D8` |

The public spelling identifies the slot for navigation only. The recovered
semantics below come from the native handler bodies.

## Same VM path on both platforms

PSP forced-linear review
`script_vm_matcher/artifacts/review/vcs_gosub_file/002b82f8_linear_90.txt`
shows:

```asm
CollectParameters(thread, &thread.IP, 2, ScriptParams)
depth = thread + 0x204
thread.stack[depth] = thread.IP       ; +0x14 + 4 * old_depth
thread.IP = ScriptParams[0]
v0 = 0
```

PS2 independent decompilation
`script_vm_matcher/artifacts/review/vcs_gosub_file/003b23d8_FUN_003b23d8.c`
recovers the same sequence directly: `CollectParameters(..., 2, ...)`, increment
the 16-bit depth at `+0x204`, write old `+0x10` IP to `+0x14 + 4*depth`, replace
`+0x10` with the first collected dword, then return `0`.

It is therefore a normal tagless GOSUB frame compatible with the already
proved `0026 RETURN` path. In the portable core it uses the same verified
absolute/negative-main-script target resolver as `0025 GOSUB`.

## Important boundary

The handler **does consume two encoded parameters**. Only `ScriptParams[0]`
is subsequently read by either native target to become the script IP.
`ScriptParams[1]` is consumed but unused inside this handler. The native
evidence does not establish its external file-system or script-loader meaning,
so the portable core deliberately calls it an ignored second VM parameter and
does not manufacture a file adapter or semantic label for it.

```text
parameters = CollectParameters(2)
push tagless post-parameter return IP
IP = resolve_script_target(parameters[0])
return VM_CONTINUE (0)
```

## Portable test

`vcs_vm_core` tests an `01BA` byte sequence with two encoded dwords, verifies
that the return IP is after **both** encodings, executes `0026 RETURN` at the
target and observes exact tagless-frame restoration. This protects the
otherwise easy mistake of treating `01BA` as a one-parameter alias of `0025`.
