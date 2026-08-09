# VCS PSP/PS2 script-thread factory proof

## Confirmed identity

| Platform | Address | Confirmed role |
| --- | ---: | --- |
| VCS PSP | `0x0005F470` | `CTheScripts::StartNewScript` |
| VCS PS2 | `0x00255AD8` | `CTheScripts::StartNewScript` |

The name is evidence-gated. Both VCS functions have the same ordered
thread-pool transition as the exact reLCS implementation of
`CTheScripts::StartNewScript`, not merely a similar aggregate instruction
shape. The directly reviewed source reference is
`research_sources/reLCS_origin_lcs/src/control/Script.cpp`.

## Direct native sequence

The PSP forced-linear window at
`script_vm_matcher/artifacts/review/vcs_start_new_script_factory/0005f470_linear_100.txt`
and the PS2 decompilation at
`script_vm_matcher/artifacts/review/vcs_start_new_script_factory/00255ad8_FUN_00255ad8.c`
give the following equivalent sequence:

```text
thread = IdleScripts.head
RemoveScriptFromList(thread, &IdleScripts)
thread->scheduler_id = NextProcessId
NextProcessId += 1
thread->Init()
thread->IP = requested_ip
AddScriptToList(thread, &ActiveScripts)
thread->active_byte_0x208 = 1
return thread
```

The exact native writes make the minimum contract concrete:

| Thread field | PSP | PS2 | Fact proved |
| --- | ---: | ---: | --- |
| scheduler identifier | `+0x08` | `+0x08` | receives the prior global counter value before that counter increments |
| instruction pointer | `+0x10` | `+0x10` | receives the sole factory argument after initialization |
| active flag | `+0x208` | `+0x208` | becomes one after active-list insertion |

`scheduler_id` is a portable descriptive name for the directly observed
counter assignment; it is not a claim about a Rockstar member spelling.
Likewise, the three invoked list/init routines are named here by their
verified effects. They have now been independently reviewed and promoted only
because both platforms match the exact reLCS member-routine semantics:

| Role | VCS PSP | VCS PS2 | Direct behavior |
| --- | ---: | ---: | --- |
| `CRunningScript::Init` | `0x0005E3EC` | `0x00255358` | resets stack/control fields, locals and the observed marker bytes, including `+0x20A` and `+0x217` to zero |
| `CRunningScript::RemoveScriptFromList` | `0x0005E6C0` | `0x00255A78` | fixes previous/next links and replaces the passed list head when predecessor is null |
| `CRunningScript::AddScriptToList` | `0x0005E6F8` | `0x00255AB0` | inserts the thread at the passed list head, fixes the prior head's back link and updates that head |

The artifacts are in
`script_vm_matcher/artifacts/review/vcs_script_scheduler_primitives/`.
This also establishes the ordering meaningfully: `0289`'s later writes of
one to `+0x20A` and `+0x217` are deliberate post-`Init` markers rather than
stale values from a recycled idle thread. Their source member spellings still
remain unclaimed.

## Relation to opcode `0289`

Both `Opcode0289_MissionStreamAndImmediateProcess` implementations call this
factory after preparing/copying the selected mission slice and before the
separate stream-finalization call. The factory therefore consumes the already
prepared active script buffer through its `requested_ip` argument; it is not
an arbitrary `new ScriptThread` allocation. `0289` then writes its two
additional request markers (`+0x20A`, `+0x217`), updates mission-global state,
clears the parsed global range and calls `CRunningScript::Process(target)`
re-entrantly.

The portable `vcs_vm_core` still keeps scheduler ownership outside the VM.
That is now an intentional host boundary with a recovered contract: a PC host
must maintain preallocated/available thread objects, active/idle membership,
the monotonic process counter and `Init` semantics before it can reproduce
this factory exactly. The supplied read-only mission harness stages and
processes a child only as a semantic model; it does not claim native pool or
list ownership.
