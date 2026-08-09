# VCS PSP/PS2 `0377` one-input/one-output pad-state proof

| Opcode | Public label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `0377` | `GET_PAD_STATE` | `0x002ACDEC` | `0x003A7E38` |

## Recovered parameter ABI

The public command library lists two parameters, but both native handlers show
the actual split ABI:

```text
selector = CollectParameters(1)[0]
ScriptParams[0] = native_pad_value(selector)  // only for selector 0..44
StoreParameters(1)                            // consumes one output destination
return VM_CONTINUE (0)
```

This is one input encoding followed by one output-destination encoding. It is
not a two-input handler.

## Direct PSP and PS2 evidence

PSP forced-linear window
`script_vm_matcher/artifacts/review/vcs_pad_state/002acdec_linear_80.txt`
calls the confirmed collector with count one, executes an unsigned range check
`sltiu selector, 0x2D`, and uses a 45-entry jump table for selector values
`0x00..0x2C`. Each branch calls a specific pad accessor and overwrites shared
`ScriptParams[0]`; the common tail invokes `StoreParameters(1)`.

PS2 decompilation
`script_vm_matcher/artifacts/review/vcs_pad_state/003a7e38_FUN_003a7e38.c`
independently recovers the same switch over `0..0x2C`, one fixed pad object and
one `StoreParameters` call after the switch.

## Non-obvious default path

For an unsigned selector outside `0..44` (including any negative raw int32),
the native handler skips the switch. Since `CollectParameters(1)` left the
selector in `ScriptParams[0]`, the common `StoreParameters(1)` writes that
unchanged selector to the requested output destination. This is intentionally
preserved by the portable model.

## Host boundary

The 45 individual accessors are input-system behavior, not Script VM layout.
`vcs_vm_core` therefore exposes `PadStateAdapter(selector) -> raw int32` only
for the native range `0..44`. It owns decoding, unsigned range behavior and
the `StoreParameters` output transfer. A missing adapter for an in-range
selector raises `Opcode0377RequiresPadStateAdapter` rather than fabricating
button state.

## Portable tests

Tests prove an in-range selector reaches the host adapter and one output
destination, an out-of-range `-1` selector is stored unchanged without any
adapter, and an in-range selector with no adapter faults explicitly.
