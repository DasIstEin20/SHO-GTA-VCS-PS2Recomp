# VCS PSP ↔ PS2 opcode table comparison

PSP range is `0x0000..0x055A` (1,371 slots); PS2 range is `0x0000..0x0564` (1,381 slots). `COMMON_SLOT` proves table membership on both platforms only. It is not a command-name or ABI claim.

| Classification | Handler slots |
| --- | ---: |
| `COMMON_SLOT` | 1328 |
| `PSP_ONLY` | 0 |
| `PS2_ONLY` | 10 |
| `DIFFERENT_HANDLER_SEMANTICS` | 0 |

## Manual semantic review started

| Opcode | Disposition | Review status | Finding |
| --- | --- | --- | --- |
| `0x0000` | `COMMON_SLOT` | `MANUAL_REVIEW_COMPLETE` | Both handlers are exactly `jr ra` with delay-slot `v0 = 0`. This is native VM dispatch status `CONTINUE`, not a script-level output; neither handler reads a parameter nor mutates thread state. |
| `0x0001` | `COMMON_SLOT` | `PARTIAL_MANUAL_REVIEW` | PS2 reads one script parameter, clears thread+0x20b, stores a derived wake value at thread+0x200 and reports native VM dispatch status 1 (`YIELD`). PSP enters its corresponding table target with the same one-parameter decoder setup, but its continuation lies outside the recovered PSP function boundary; no cross-platform semantic equality is claimed yet. |
| `0x0002` | `COMMON_SLOT` | `PARTIAL_MANUAL_REVIEW` | PS2 reads one script parameter and writes the resulting script position to thread+0x10 before reporting native VM dispatch status 0 (`CONTINUE`). PSP reaches the analogous table target with the same one-parameter decoder setup, but its continuation is outside the recovered PSP function boundary; no cross-platform semantic equality is claimed yet. |
| `0x037A` | `COMMON_SLOT` | `MANUAL_REVIEW_COMPLETE` | Both platforms enter the common CALL handler, compare the preserved opcode with 0x037B, push a thread+0x204 call-frame entry, consume call metadata/arguments and report native VM dispatch status 0 (`CONTINUE`). The non-0x037B branch uses the CALL_NOT frame tag (0x00c00000). |
| `0x037B` | `COMMON_SLOT` | `MANUAL_REVIEW_COMPLETE` | Both platforms use the same handler as 0x037A, but the `opcode == 0x037B` branch writes the distinct CALL frame tag (0x00800000) before collecting call metadata/arguments and reports native VM dispatch status 0 (`CONTINUE`). This proves the shared target is not an accidental table alias. |
| `0x055A` | `COMMON_SLOT` | `MANUAL_REVIEW_COMPLETE` | Both zero-parameter handlers reduce/update Script VM condition state: they read thread+0x20c, thread+0x206 and thread+0x209, write the aggregate state, clear thread+0x206 at the terminal condition and report native VM dispatch status 0 (`CONTINUE`). The public opcode list provides no semantic name, so a generic name is retained. |

## `0x055B..0x0564`: PS2-only

All ten slots have no PSP table record; each was manually inspected from the listed PS2 instruction windows. Their original command names remain intentionally withheld.

| Opcode | Behavior proved from PS2 instruction window |
| --- | --- |
| `0x055B` | PS2-only: reads one parameter and updates a global with a max-style comparison; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x055C` | PS2-only: reads one parameter and updates a separate global through the inverse comparison; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x055D` | PS2-only: reads one parameter and updates another global through the inverse comparison; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x055E` | PS2-only: reads one parameter and updates another global through the inverse comparison; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x055F` | PS2-only: reads one parameter, calls 0x0028A5E0 with that value and reports native VM dispatch status 0 (`CONTINUE`). |
| `0x0560` | PS2-only: reads one parameter, calls 0x0028A5E0 with that value and reports native VM dispatch status 0 (`CONTINUE`). |
| `0x0561` | PS2-only: reads one parameter, applies floating-point scale/rounding and stores a max-clamped result to a global; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x0562` | PS2-only: reads one parameter, calls 0x0028A608 with that value and reports native VM dispatch status 0 (`CONTINUE`). |
| `0x0563` | PS2-only: reads two parameters, resolves an object and sets or clears bit 0 in its +0x1e0 field; reports native VM dispatch status 0 (`CONTINUE`). |
| `0x0564` | PS2-only: reads one parameter and sets a global flag at 0x00487724 to zero/nonzero; reports native VM dispatch status 0 (`CONTINUE`). |

`DIFFERENT_HANDLER_SEMANTICS` remains zero: no such classification is emitted from target-address or descriptor differences alone. It requires a manual proof of divergent behavior.
