# VCS PSP ↔ PS2 COMMON_SLOT semantic-shape comparison

This is a normalized MIPS-level triage of all table-common handlers. It compares parameter-decoder calls, direct `CRunningScript` offsets, global read/write shape, small constants, normalized callee kinds, approximate CFG and native handler-status constants. A status constant is interpreter ABI control flow, not a script-command output. It does **not** infer command identity or automatically emit `DIFFERENT_HANDLER_SEMANTICS`.

| Triage | Slots |
| --- | ---: |
| `LIMITED_RECOVERY` | 57 |
| `MANUAL_PARTIAL` | 2 |
| `MANUAL_SAME` | 4 |
| `STRUCTURAL_HIGH` | 135 |
| `STRUCTURAL_LOW` | 1047 |
| `STRUCTURAL_MEDIUM` | 83 |

## Highest-priority manual-review candidates

| Opcode | Triage | Decoder / thread fields | VM dispatch-status constants | Mean signal similarity |
| --- | --- | --- | --- | ---: |
| `0x026E` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.020 |
| `0x04F8` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.023 |
| `0x04F9` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.023 |
| `0x0437` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.024 |
| `0x01C5` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.025 |
| `0x0555` | `STRUCTURAL_LOW` | script_param_decoder:type=12 / — | — / constant:0:1 | 0.025 |
| `0x04DF` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.027 |
| `0x01C6` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.028 |
| `0x04D8` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.028 |
| `0x02F1` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.028 |
| `0x044C` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.029 |
| `0x012D` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.029 |
| `0x02D5` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.029 |
| `0x01C1` | `STRUCTURAL_LOW` | script_param_decoder:type=6 / — | — / constant:0:1 | 0.029 |
| `0x02AF` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.029 |
| `0x01B7` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.029 |
| `0x01B8` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.029 |
| `0x01B9` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.029 |
| `0x02AE` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.029 |
| `0x0237` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.030 |
| `0x02D8` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.030 |
| `0x0054` | `STRUCTURAL_LOW` | script_param_decoder:type=8 / — | — / constant:0:1 | 0.030 |
| `0x013D` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.030 |
| `0x02EF` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.030 |
| `0x046E` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.030 |
| `0x0446` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.031 |
| `0x02A6` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.031 |
| `0x037C` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.031 |
| `0x0053` | `STRUCTURAL_LOW` | script_param_decoder:type=6 / — | — / constant:0:1 | 0.031 |
| `0x0485` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.031 |
| `0x01B3` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x021A` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x0252` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x02E1` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x0306` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x034B` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x0438` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x02DE` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x02DF` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x02E0` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x036B` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x02A7` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x0349` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x032B` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.032 |
| `0x0436` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x0439` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.032 |
| `0x00A3` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x0149` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x0251` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0361` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0397` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0398` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x03A6` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x01F0` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.033 |
| `0x0326` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.033 |
| `0x02A8` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0327` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.033 |
| `0x0317` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x039C` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x0322` | `STRUCTURAL_LOW` | script_param_decoder:type=5 / — | — / constant:0:1 | 0.033 |
| `0x0329` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.033 |
| `0x004F` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x00A0` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x009F` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x01AB` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.033 |
| `0x01C7` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0320` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x034D` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x035D` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x037D` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x037E` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x037F` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x0424` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.033 |
| `0x007D` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.033 |
| `0x014F` | `STRUCTURAL_LOW` | script_param_decoder:type=2 / — | — / constant:0:1 | 0.034 |
| `0x0441` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.034 |
| `0x036C` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.034 |
| `0x007E` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.034 |
| `0x012C` | `STRUCTURAL_LOW` | script_param_decoder:type=1 / — | — / constant:0:1 | 0.034 |
| `0x04E8` | `STRUCTURAL_LOW` | script_param_decoder / — | — / constant:0:1 | 0.034 |

The complete per-opcode signal payload is in the companion JSON. A `COMMON_SLOT` remains a membership classification until manual decompilation establishes either semantic equivalence or a semantic difference.
