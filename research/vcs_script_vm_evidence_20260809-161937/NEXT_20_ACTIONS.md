# Next 20 actions

1. Treat the eight-byte VCS SCM header as outside ScriptSpace in every loader, route trace and control-flow target calculation.
2. Preserve `VcsDiscFS` as read-only: directory and ISO9660 backends may read a user-supplied disc source but must never mount, patch, unpack or persist its data.
3. Preserve `VcsMissionHost`'s exact `0289` order: selector remap, offset range, stream scope, active-buffer copy, `StartNewScript`, scope leave, marker/global writes, re-entrant child `Process()`.
4. Do not collapse `0289` into `LaunchMission(id)` or an opaque callback. A child host/VM fault must propagate to the bootstrap runner with its opcode visible.
5. Treat `gta-vcs-bootstrap` as a vertical-slice/handoff proof, not a game launcher: source → `MAIN.SCM` → main thread → first child → explicit boundary.
6. The next blocker on both verified selector-zero routes is `013F CREATE_PICKUP`: recover only the model-object resolver first, then terrain-Z and pickup-manager adapters if the real route demands them.
7. Keep selector 1..9 as a static host census, not a reachability claim. Re-run them only after the real selector-zero child reaches a later wall.
8. Use PPSSPP only to capture naturally reached `0289` behavior after a safe save-state route is available; do not inject bytecode or patch game memory.
9. Extend a route harness with another entry/thread only after proving its initial ScriptSpace base and header/container convention.
10. Use confirmed VCS PSP `ReadMultiScriptFileOffsetsFromScript` (`0x0006041C`) to keep the PC mission-offset table direct and bounded; next prove any extra table/sentinel behavior from assets and consumers.
11. Keep `CTheScripts::StartNewScript` as a direct scheduler contract (idle removal, process ID, Init, IP, active insertion and `+0x208`); model list ownership without inventing native link layouts.
12. Continue per-handler work only when a reached route, a host boundary, or a high-frequency real-SCM opcode supplies the prioritization evidence.
13. Use `0113` as the vehicle-release-mark bridge only; do not merge it with destructive `0049` or propagate public SCM wording to internal callees.
14. Re-run bounded static coverage after every evidence status change and keep coupled host boundaries visible in coverage/hand-off status.
15. Validate real route IPs against native coordinates before interpreting adjacent bytes as code or data.
16. Add confirmed symbols only as a byproduct of instruction-level semantic proof; never promote structural table equivalence or a fuzzy match.
17. Keep PSP runtime captures paired with PS2 static evidence and state their input-path limits explicitly.
18. Maintain a no-game-payload handoff: source, reports, static addresses, route metadata and traces only.
19. Rebuild `vcs_vm_core`, run CTest and re-run the PS2-extracted + ISO9660 bootstrap proofs before every handoff.
20. Export a new ZIP only after rebuilding and checking its manifest/archive for game executables, ISOs, APKs, OBBs and raw SCM payloads.
