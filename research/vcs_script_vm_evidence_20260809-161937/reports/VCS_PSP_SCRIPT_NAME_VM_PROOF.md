# VCS PSP/PS2 `0238` eight-byte script-name proof

| Opcode | Public navigation label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `0238` | `SCRIPT_NAME` | `0x002B8360` | `0x003B2440` |

## Recovered VM contract

```text
source = CollectParameters(1)[0]
copy exactly 8 bytes with native strncpy semantics into ScriptThread + 0x20F
for i in 0..7:
    if 'A' <= name[i] <= 'Z': name[i] += 0x20
return VM_CONTINUE (0)
```

The name field is a fixed eight-byte buffer, not an abstract C++ string. A
source with eight non-NUL bytes fills the field completely; a shorter source
gets the native `strncpy` NUL padding. The sole direct string-tag representation
in `vcs_vm_core` is a ScriptSpace-relative byte offset.

## Direct PSP and PS2 evidence

The PSP forced-linear window
`script_vm_matcher/artifacts/review/vcs_script_name/002b8360_linear_100.txt`
collects one parameter, converts a nonzero offset to `ScriptSpace + offset`,
calls `0x00354F80(destination=thread+0x20F, source, length=8)`, then loops
over exactly eight bytes and adds `0x20` only for ASCII values `0x41..0x5A`.
It returns `v0 = 0`.

The PSP helper decompilation
`script_vm_matcher/artifacts/review/vcs_script_name/00354f80_FUN_00354f80.c`
is the exact `strncpy` shape: copy through NUL or length exhaustion, then
zero-fill the remaining length.

The PS2 handler
`script_vm_matcher/artifacts/review/vcs_script_name/003b2440_FUN_003b2440.c`
independently recovers the same one-parameter collection, fixed length-eight
copy into `thread+0x20F`, ASCII uppercase-only loop and `return 0`.

## Portable boundary

`ScriptThread::script_name` is an eight-byte array. The core preserves fixed
copy and ASCII conversion. A zero, negative or out-of-ScriptSpace source is
not a safe portable dereference, so it produces
`InvalidScriptStringAddress`; this is intentional fail-loud behavior rather
than an invented native result for malformed bytecode.

## Portable tests

Tests cover a NUL-padded mixed-case name, an eight-character non-NUL name,
the lowercasing rule, IP advancement through the string encoding and the
explicit invalid-source fault.
