# VCS PSP/PS2 `0079` mission-launch request proof

| Opcode | Public label | VCS PSP target | VCS PS2 target |
|---|---|---:|---:|
| `0079` | `LAUNCH_MISSION` | `0x002B8250` | `0x003B2320` |

## Recovered VM contract

Both versions have the same narrow Script VM contribution:

```text
mission_id = CollectParameters(1)[0]
target = engine_resolve_mission_script(mission_id)
target->byte_0x20A = 1
return VM_CONTINUE (0)
```

The returned object is a different script-thread-like object.  The native
write at offset `+0x20A` is directly observed, but the source-level name of
that byte is not asserted here.  The portable analogue is the explicitly
named `ScriptThread::mission_launch_requested` state, documenting the observed
request rather than claiming the original field spelling.

## Direct PSP and PS2 evidence

The PSP forced-linear window
`script_vm_matcher/artifacts/review/vcs_mission_launch/002b8250_linear_90.txt`
shows a one-parameter call to confirmed `CollectParameters` at `0x0006189C`,
then passes `ScriptParams[0]` to `0x0005F470`.  It writes literal one to byte
`0x20A` of that call's returned pointer and sets `v0` to zero before return.

The PS2 decompilation
`script_vm_matcher/artifacts/review/vcs_mission_launch/003b2320_FUN_003b2320.c`
independently recovers the identical shape: `CollectParameters(..., 1, ...)`,
an engine resolver called with the collected dword, a byte-one write at
`returned_pointer + 0x20A`, then `return 0`.

Therefore the cross-platform result proves the input count, resolver handoff,
target-side request write and `CONTINUE` status.  It does not infer what a
mission ID means outside the resolver, how the target is allocated, or when
the engine services the request.

## Host boundary and portable implementation

`vcs_vm_core` supplies a narrow
`MissionLaunchAdapter(ScriptVm&, int32_t) -> ScriptThread*`.  The adapter owns
mission-index lookup, target-thread lifetime and engine scheduling.  After a
non-null result, the VM sets `mission_launch_requested` and returns
`ScriptResult::Continue`.  A missing adapter and a null adapter result fail
loudly (`Opcode0079RequiresMissionLaunchAdapter` and
`MissionLaunchAdapterReturnedNull`) rather than fabricating a mission thread.

## Portable tests

The C++ tests prove that the single decoded mission ID reaches the adapter,
the returned target receives the request flag, the source thread continues,
and both unavailable-host cases fault explicitly.
