# VCS PSP/PS2 `01E7` total-unique-jumps VM proof

| Platform | Handler |
| --- | ---: |
| VCS PSP | `0x00081FBC` |
| VCS PS2 | `0x00184C40` |

Both handlers call `CollectParameters(thread + 0x10, 1)`, copy the one raw
int32 to their corresponding stat-global location and return native
`CONTINUE` (`0`). PSP forced-linear instructions explicitly show the global
store after the collector; PS2 decompilation shows the identical sequence.
Artifacts: `script_vm_matcher/artifacts/review/vcs_mission_child_01e7/`.

Exact reLCS source in `src/control/Script3.cpp` names the corresponding
single-parameter aggregate `CStats::TotalNumberOfUniqueJumps`. Therefore the
portable core implements `01E7` as a narrow host adapter with the public
navigation label `SET_UNIQUE_JUMPS_TOTAL`; save persistence and UI remain host
owned. It does not fabricate a stat store when the adapter is absent.
