# VCS PSP/PS2 timed float arithmetic proof (`0031..0034`)

## Scope and rule

This is a direct handler proof, not a name inferred from a whole-function
histogram.  The public SCM spellings are used only as labels; the semantics
below come from the actual PSP and PS2 dispatch targets and their forced-linear
instruction windows.

| Opcode | Public label | PSP target | PS2 target |
|---|---|---:|---:|
| `0031` | `ADD_TIMED_VAL_TO_FLOAT_VAR` | `0x002B7EA8` | `0x003B1D20` |
| `0032` | `ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR` | `0x002B7EA8` | `0x003B1D20` |
| `0033` | `SUB_TIMED_VAL_FROM_FLOAT_VAR` | `0x002B7F18` | `0x003B1DA8` |
| `0034` | `SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR` | `0x002B7F18` | `0x003B1DA8` |

Thus each add/sub pair is an exact native-target alias on **both** platforms.
The evidence does not claim a distinct source-kind ABI for the two public
spellings: each target uses the ordinary parameter decoder and therefore
accepts whichever encoded parameter form that decoder resolves.

## PSP direct path

The reviewed PSP windows are:

- `script_vm_matcher/artifacts/review/vcs_timed_arithmetic/002b7ea8_linear_42.txt`
- `script_vm_matcher/artifacts/review/vcs_timed_arithmetic/002b7f18_linear_42.txt`

Both start by forwarding the destination encoding through the confirmed
`GetPointerToScriptVariable` primitive (`0x0005DA7C`), then invoke the
one-parameter collector.  They load the current destination binary32 value and
the one collected raw binary32 parameter.  `0x002B7EA8` emits scalar `mul.s`
followed by `add.s`; `0x002B7F18` emits scalar `mul.s` followed by `sub.s`.
The multiplier is read from `gp + 0x1E1C`.  The computed binary32 result is
stored through the resolved destination and the epilogue sets `v0 = 0`.

The direct VM contract is consequently:

```text
destination = resolve_variable(next encoded parameter)
value       = decode_one_parameter_as_raw_binary32()
delta       = value * engine_time_step
*destination = *destination +/- delta
return VM_CONTINUE (0)
```

## Independent PS2 cross-check

The paired PS2 decompilation/instruction windows are:

- `script_vm_matcher/artifacts/review/vcs_timed_arithmetic/003b1d20_FUN_003b1d20.c`
- `script_vm_matcher/artifacts/review/vcs_timed_arithmetic/003b1da8_FUN_003b1da8.c`

Their normal paths reduce directly to:

```cpp
*destination = *destination + ScriptParams[0] * DAT_004CD410; // 0031/0032
*destination = *destination - ScriptParams[0] * DAT_004CD410; // 0033/0034
return 0;
```

This independently confirms the PSP `mul.s` scalar is a shared engine
time-step value, not an extra SCM parameter or an accidental intermediate.

## Portable-core boundary

`DAT_004CD410` / PSP `gp+0x1E1C` belongs to the engine timing subsystem.  The
portable core therefore exposes a required `TimeStepProvider`; it neither
guesses units nor samples a wall clock.  With a provider, the core preserves
binary32 arithmetic and returns `CONTINUE`.  Without one it raises
`TimedArithmeticRequiresTimeStep` after consuming the VM-owned destination and
parameter bytes, rather than silently using a made-up frame delta.

This is a confirmed common PSP/PS2 semantic core with an explicit host timing
adapter boundary.
