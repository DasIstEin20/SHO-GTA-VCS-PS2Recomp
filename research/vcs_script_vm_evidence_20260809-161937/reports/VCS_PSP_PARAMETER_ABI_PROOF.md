# VCS PSP parameter ABI proof

Status: **decoder primitives confirmed in the VCS PSP debug EBOOT**
`SHA-256 a722be366b2090676f225941235e7a01cb4a42b242b14e785180162f46376b12`.

This document distinguishes native PSP facts from the portable reconstruction.
It does not treat an automatic cross-build match as proof.

## Native anchors

| Native PSP address | Recovered role | Evidence |
| --- | --- | --- |
| `0x0006189C` | `CollectParameters` | Takes `(thread, &IP, count, ScriptParams)`, dispatches parameter tags, writes each raw value, advances the supplied IP and returns. |
| `0x0005E71C` | `CollectNextParameterWithoutIncreasingPC` | Copies the supplied IP, decodes one parameter, and never writes the caller's IP. |
| `0x0005DA7C` | `GetPointerToScriptVariable` | Reads a variable encoding, advances the supplied IP, and returns an address into the thread-local block or ScriptSpace. |
| `0x0005E890` | `StoreParameters` | Iterates `count` values from the shared parameter buffer and stores each through `0x0005E3D0 -> 0x0005DA7C`. |

Raw decompilations are preserved under
`script_vm_matcher/artifacts/review/vcs_psp_parameter_abi/`.

The independently dumped instruction window for the plural collector is
`script_vm_matcher/artifacts/review/vcs_gosub_abi/0006189c_instruction_window.txt`.

## `0x0006189C`: plural collector

This entry is directly reached by PSP `0025 GOSUB` with `count = 1`. It loads
`*IP`, adds the ScriptSpace base, dispatches the type byte through a jump table,
writes each decoded dword to the supplied `ScriptParams` cursor, and finally
writes the advanced ScriptSpace-relative cursor back through `&IP` before
returning. Its variable path calls the independently confirmed resolver at
`0x0005DA7C`. This is a native `CollectParameters` proof, not a name copied
from Android/LCS.

## `0x0005E71C`: value decoder

The function has the exact semantic shape of
`CollectNextParameterWithoutIncreasingPC`:

1. Copies its input IP to a private cursor.
2. Reads one type byte from `ScriptSpace + private_cursor`.
3. Decodes the immediate forms below, or rewinds the private cursor one byte
   and calls the variable resolver.
4. Returns the raw 32-bit parameter payload without updating the script
   thread's IP.

| Tag | PSP decoding |
| --- | --- |
| `0x00` | plural `CollectParameters`: one-byte raw zero; standalone next-parameter primitive instead follows its default resolver path |
| `0x01` | integer zero |
| `0x02` | float zero bits |
| `0x03` | unsigned byte shifted left 24 |
| `0x04` | unsigned little-endian word shifted left 16 |
| `0x05` | low byte shifted left 8 OR little-endian word shifted left 16 |
| `0x06` | little-endian 32-bit integer |
| `0x07` | **signed** 8-bit integer |
| `0x08` | **signed** 16-bit integer |
| `0x09` | little-endian 32-bit float bit pattern |
| other | resolve variable and load its raw 32-bit value |

The signed `0x07/0x08` behavior comes directly from the PSP decompilation
(`char`/`short` loads), not from the Android reference source.

The independently confirmed plural collector at `0x0006189C` also exposes tag
`0x0A`: it skips the tag, saves the ScriptSpace-relative address of the
following NUL-terminated bytes, walks to the terminator, and stores that offset
as the raw parameter value. The portable collector supports it as a raw `int32`
offset; a distinct `CollectNextParameterWithoutIncreasingPC` tag-`0x0A` path
has not been separately claimed.

## `0x0005DA7C`: variable resolver

The following ranges are direct PSP facts. `LP` means the thread's current
locals pointer; a local slot is an `int32`.

| Tag range | Encoded trailing bytes | Resolved storage |
| --- | --- | --- |
| `00..0C` | none | thread-local slot `0x5D + tag` |
| `0D..6C` | none | thread-local slot `LP + tag - 0x0D` |
| `6D..CC` | `index_id`, `size` | thread-local slot `LP + tag - 0x6D + local[LP + index_id]` |
| `CD..E5` | `index_in_block` | ScriptSpace dword at `4 * ((tag - 0xCD) * 256 + index_in_block)` |
| `E6+` | `index_in_block`, `index_id`, `size` | ScriptSpace dword at `4 * ((tag - 0xE6) * 256 + local[LP + index_id] + index_in_block)` |

For both array forms, PSP rejects `size == 0`, a negative index, or an index
greater than or equal to `size`. The portable core fails loudly with
`InvalidArrayIndex` for the same conditions.

`0x00` is a deliberately documented VCS edge case. The PSP plural collector's
direct jump-table case stores raw zero, decrements its count and continues; the
real `0203 DISPLAY_TEXT` site uses it as its first coordinate payload. In
contrast, PSP `0x0005E71C` has no case-`0` branch and routes the standalone
next-parameter call through the low variable resolver range. The portable core
therefore keeps plural single-parameter reads distinct from the exact
non-advancing primitive; it does **not** import reLCS's `ARGUMENT_END`
convention into VCS.

## Portable mapping

`vcs_vm_core` now exposes the five semantic primitives:

```cpp
collect_next_parameter_without_increasing_pc(...)
read_parameter(...)
collect_parameters(...)
get_pointer_to_script_variable(...)
store_parameters(...)
```

Float forms and the `0x0A` string offset remain raw `int32` payloads. Global
variables intentionally use byte-addressed `ScriptSpace`, matching the PSP
resolver, rather than a new parallel global vector. `read_parameter()` is the
portable convenience for the VCS plural collector's one-value semantics;
`collect_next_parameter_without_increasing_pc()` retains the direct standalone
decoder semantics.

## Boundary still open

The portable plural collector intentionally remains a simple semantic loop
instead of copying the PSP's coprocessor-assisted jump-table implementation.
String/GXT-like parameter forms and their engine adapters still require
opcode-level proof. Function CALL/RETURN and ordinary GOSUB/RETURN frame
semantics are documented in their separate reports.
