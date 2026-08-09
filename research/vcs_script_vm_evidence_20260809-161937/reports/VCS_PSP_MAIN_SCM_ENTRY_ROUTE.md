# VCS MAIN.SCM observed portable-VM route

This is a read-only route through original SCM bytes in `vcs_vm_core`, not PPSSPP runtime telemetry and not a whole-program CFG claim.

- Entry IP: `0x7860`
- Commands before terminal result: `216`
- Distinct opcode IDs: `12`
- Terminal: index `215`, opcode `0x289`, IP `0x7889 → 0x788C`, `Opcode0289RequiresMissionStreamingSubsystem`

## Observed VM-proven control transfers

| Index | Role | Opcode | IP before | IP after |
| ---: | --- | --- | --- | --- |
| 3 | GOSUB_FILE | `0x01BA` | `0x7871` | `0x7BFF` |
| 41 | GOTO_IF_FALSE | `0x0022` | `0x7D20` | `0x7D27` |
| 211 | GOTO | `0x0002` | `0x80B4` | `0x8101` |
| 212 | RETURN | `0x0026` | `0x8101` | `0x787D` |
| 213 | GOSUB_FILE | `0x01BA` | `0x787D` | `0x8103` |
| 214 | RETURN | `0x0026` | `0x8103` | `0x7889` |

## Opcode frequency on this route

| Opcode | Count |
| --- | ---: |
| `0x0004` | 192 |
| `0x0006` | 12 |
| `0x0026` | 2 |
| `0x01BA` | 2 |
| `0x0002` | 1 |
| `0x0005` | 1 |
| `0x001B` | 1 |
| `0x0022` | 1 |
| `0x00C9` | 1 |
| `0x0238` | 1 |
| `0x0289` | 1 |
| `0x0401` | 1 |

The terminal `0289` is a direct PSP/PS2-proved mission-streaming subsystem boundary. Its selector is decoded before the explicit fault; no generic mission-launch adapter is substituted.
