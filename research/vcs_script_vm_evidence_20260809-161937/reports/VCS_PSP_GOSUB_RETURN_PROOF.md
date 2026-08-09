# VCS PSP/PS2 GOSUB / RETURN proof

## Native entrypoints

| Opcode | VCS PSP target | VCS PS2 target |
| --- | --- | --- |
| `0025 GOSUB` | `0x002B7E08` | `0x003B2108` |
| `0026 RETURN` | `0x002B7E88 -> 0x0005F09C` | `0x003B2188 -> 0x00256308` |

The public VCS command list calls `0025` `GOSUB`, but the name is supported
here by direct stack/IP behavior rather than by the list alone.

## Creation

PSP `0x002B7E08` calls the confirmed `0x0006189C CollectParameters` with
`count = 1`, passing `&thread.IP` and the shared parameter buffer. PS2
`0x003B2108` exposes the rest of the same operation directly:

```text
CollectParameters(thread, &thread.IP, 1, ScriptParams)
old_depth = thread.call_depth
thread.call_depth = old_depth + 1
thread.stack[old_depth] = thread.IP       // post-target-parameter return IP
thread.IP = resolve_target(ScriptParams[0])
return VM_CONTINUE (0)
```

`resolve_target(x)` is already represented by the portable VM's proven script
target convention: non-negative `x` is an absolute ScriptSpace IP; negative
`x` maps to `main_script_size - x`. The latter is visible in PS2 as
`DAT_004CD6D8 - DAT_0050E1F8`.

The pushed GOSUB entry has no `0x00800000` function-frame tag.

## Return

PSP `0x0005F09C` and PS2 `0x00256308` first decrement `thread+0x204`, load the entry from
`thread+0x14 + 4 * depth`, stores it into `thread.IP`, and only enters its
CALL/CALL_NOT output-copy path if `(entry & 0x00800000) != 0`. Therefore a
tagless GOSUB frame performs exactly:

```text
thread.IP = stack[--thread.call_depth]
return VM_CONTINUE (0)
```

This proves that GOSUB uses the same depth/stack storage as CALL but not its
header, output, locals rollback or CALL_NOT condition machinery.

## Portable test

`vcs_vm_core` now executes a real `0025` byte sequence with an `INT32` target,
pushes a tagless frame with the post-parameter return IP, then executes `0026`
at the target and verifies restoration of that IP and empty call stack.

Evidence files:

- `script_vm_matcher/artifacts/review/vcs_gosub_abi/002b7e08_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_gosub_abi/0006189c_instruction_window.txt`
- `script_vm_matcher/artifacts/review/vcs_gosub_abi/003b2108_FUN_003b2108.c`
- `script_vm_matcher/artifacts/review/vcs_gosub_abi/0005f09c_FUN_0005f09c.c`
- `script_vm_matcher/artifacts/review/vcs_gosub_abi/003b2188_FUN_003b2188.c`
- `script_vm_matcher/artifacts/review/vcs_gosub_abi/00256308_FUN_00256308.c`
