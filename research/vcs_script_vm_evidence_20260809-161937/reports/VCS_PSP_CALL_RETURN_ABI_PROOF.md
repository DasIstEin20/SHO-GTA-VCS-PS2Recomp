# VCS PSP CALL / RETURN ABI proof

Status: **function CALL/CALL_NOT return path statically confirmed in VCS PSP**.
The evidence target is `GTA_VCS_Debug_EBOOT.ELF`, SHA-256
`a722be366b2090676f225941235e7a01cb4a42b242b14e785180162f46376b12`.

This proof covers the function frames created by `037A CALL_NOT` and `037B
CALL`. Ordinary `0025 GOSUB` creation is now separately proven in
`reports/VCS_PSP_GOSUB_RETURN_PROOF.md`.

## Entry points

| Opcode | PSP table target | Role |
| --- | --- | --- |
| `0026 RETURN` | `0x002B7E88` | Calls `0x0005F09C` and returns native status `0`/CONTINUE. |
| `037A CALL_NOT` | `0x002B8714` | Pushes a frame with tag `0x00C00000`; dispatches the CALL argument machinery. |
| `037B CALL` | `0x002B8714` | Pushes a frame with tag `0x00800000`; dispatches the CALL argument machinery. |
| shared return core | `0x0005F09C` | Pops the frame, copies outputs, rolls back locals and stores caller outputs. |

Raw decompilations are in
`script_vm_matcher/artifacts/review/vcs_psp_parameter_abi/`.

## Native frame encoding

`0x002B8714` increments the call depth at `thread + 0x204` and writes the
saved value at `thread + 0x14 + 4 * old_depth`:

```text
encoded_return = call_header_ip
               | CALL tag       (0x00800000)
               | CALL_NOT tag   (0x00C00000)
               | return_skip_bytes << 24
```

The low 22 bits are the header IP. Bit `0x00800000` identifies a function
frame; `0x00400000` is the CALL_NOT modifier. The high byte is the number of
parameter bytes from immediately after the 3-byte CALL header to the output
destination list. The latter is populated by the shared CALL argument path;
the return core consumes it as `encoded_return >> 24`.

The three header bytes are re-read by `0x0005F09C` as:

```text
input_count, output_count, locals_offset
```

## Proven `RETURN` algorithm

For a function frame, `0x0005F09C` performs:

```text
frame = stack[--thread.call_depth]

if frame has CALL/CALL_NOT tag:
    if frame has CALL_NOT modifier:
        thread.condition_result = !thread.condition_result   // PSP +0x209

    ip = frame.call_header_ip
    in, out, locals_offset = read 3 header bytes

    ScriptParams[0..out) = locals[thread.locals_pointer + in .. + in + out)
    ip += frame.return_skip_bytes
    thread.locals_pointer -= locals_offset
    StoreParameters(ip, out)       // outputs into the caller destinations
```

The source output range is exact: `0x0005F09C` constructs it as
`thread + 0x54 + 4 * (locals_pointer + input_count)` before copying `out`
dwords to the shared parameter buffer. It then calls the confirmed
`StoreParameters` primitive at `0x0005E890`.

## Portable reconstruction and test

`vcs_vm_core` now implements opcode `0026 RETURN` for the confirmed function
frames. The unit test builds a real byte layout containing a `CALL` or
`CALL_NOT`, two inputs, one output destination and a callee `RETURN`. It
verifies all of these simultaneously:

- input locals are addressed from the new locals pointer;
- the callee output at `locals[LP + input_count]` is copied to the caller's
  variable destination;
- `locals_pointer` rolls back by the header's `locals_offset`;
- IP resumes after the output destination encoding;
- `CALL_NOT` flips the condition result while `CALL` does not;
- native status remains `CONTINUE`.

## Boundary

This document remains limited to the tagged function-frame path. The tagless
GOSUB frame is deliberately documented separately, because its return behavior
is simpler and must not be conflated with CALL output copying.
