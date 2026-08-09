# VCS PSP/PS2 `010E GET_GAME_TIMER` VM proof

| Opcode | Public label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `010E` | `GET_GAME_TIMER` | `0x002F0118` | `0x003DF8C8` |

## Direct shared sequence

PSP `002f0118_linear_60.txt` loads the game timer from `gp + 0x1E0C`, writes
that raw dword to the shared parameter buffer and calls the confirmed
`StoreParameters` primitive (`0x0005E890`) with count one. It then writes
`v0 = 0`.

The independent PS2 decompilation
`003df8c8_FUN_003df8c8.c` expresses the same sequence:

```cpp
ScriptParams[0] = DAT_004CD404;
StoreParameters(thread, &thread.IP, 1);
return 0;
```

Thus the VM contract is:

```text
StoreParameters([game_clock_raw_dword])
return VM_CONTINUE (0)
```

There is one **output destination encoding** consumed by `StoreParameters`; it
is not an opcode return value. The portable core intentionally reuses its
existing injected game-clock provider — already needed by `WAIT` — and copies
the clock bits into the script variable unchanged.

## Portable test

The core test injects clock bits `0xFEDCBA98`, executes an `010E` bytecode
sequence with one local destination and verifies the local receives exactly
those raw 32 bits while the native VM status remains `CONTINUE`.
