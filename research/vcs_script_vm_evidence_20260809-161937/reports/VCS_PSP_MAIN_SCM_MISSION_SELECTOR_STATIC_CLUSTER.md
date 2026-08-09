# VCS PSP `MAIN.SCM` static `0289` selector cluster

This is a bounded **static linear** scan of the original `MAIN.SCM` main
segment. It does not claim that any site is reachable, that it executes in
this order at runtime, or that native mission mode takes the streaming branch.

The scanner uses the directly proved one-parameter `0289` ABI and records a
selector only for VCS immediate integer forms. It stops at the pre-existing
unknown `0x120D` boundary rather than resynchronising through data.

## Result

All 11 decoded `0289` occurrences have a direct integer literal; none uses a
local/global/array/float-like parameter encoding.

| File offset | Tag | Literal selector |
| ---: | --- | ---: |
| `0x007891` | `0x01` | 0 |
| `0x007894` | `0x07` | 1 |
| `0x007898` | `0x07` | 2 |
| `0x00789C` | `0x07` | 3 |
| `0x0078A0` | `0x07` | 4 |
| `0x0078A4` | `0x07` | 5 |
| `0x0078A8` | `0x07` | 6 |
| `0x007A51` | `0x07` | 7 |
| `0x007AF6` | `0x07` | 8 |
| `0x007B8E` | `0x07` | 8 |
| `0x007BB8` | `0x07` | 9 |

The file offsets include the eight-byte container header. Thus the first row
corresponds to the already observed portable VM instruction IP `0x007889`.
The read-only VM route independently reaches selectors 0 and then 1 before
stopping at the explicit host boundary, which is consistent with the first
two static rows but does not promote the remaining rows to runtime evidence.

## Consequence

The next host-census batch can start deterministically with selector indices
0 through 9 from the parsed 99-dword offset table. It must keep each child
result separately and retain the source/buffer/scheduler contract; this is not
license to replace `0289` with a generic `LaunchMission(index)` callback.

Generated evidence is retained in `VCS_PSP_MAIN_SCM_OPCODE_USAGE.json` under
`scan.mission_stream_selector_sites`, and the scanner implementation is
`script_vm_matcher/src/analyze_vcs_scm_usage.py`.
