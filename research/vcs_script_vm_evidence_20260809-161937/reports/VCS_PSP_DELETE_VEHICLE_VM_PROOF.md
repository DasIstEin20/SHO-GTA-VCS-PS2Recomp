# VCS PSP/PS2 opcode `0049`: vehicle-delete host bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x002E03C8` | `script_vm_matcher/artifacts/review/vcs_delete_car/002e03c8_linear_160.txt` |
| PS2 | `0x003D0B60` | `script_vm_matcher/artifacts/review/vcs_delete_car/003d0b60_FUN_003d0b60.c` |

Both native handlers first call their platform `CollectParameters` primitive
with a count of one and use the resulting raw dword as input to an
engine-owned vehicle-pool lookup. The control flow then agrees:

1. If native `ScriptThread +0x20A` is nonzero, enter a separate native cleanup
   path using the same collected dword.
2. If the pool lookup returned an entity, remove that entity, remove references
   to it, and invoke its final virtual destruction path.
3. Return `v0 = 0`, the VM's `CONTINUE` status.

The PSP forced-linear window gives the complete handler despite the automatic
function boundary ending after its collector call. The PS2 decompilation shows
the equivalent full CFG directly. The reLCS `COMMAND_DELETE_CAR` source is
only a structural cross-check for the pool/remove/references/delete sequence;
the VCS contract above follows from the two native instruction paths.

The portable core therefore decodes exactly one raw vehicle handle and calls
`VehicleDeleteAdapter(handle, native_context_flag)`. Pool lookup, the exact
meaning and ownership of the `+0x20A` cleanup context, cleanup-list operations,
reference removal and entity lifetime deliberately remain host integration.
Missing host integration faults as `Opcode0049RequiresVehicleDeleteAdapter`.
