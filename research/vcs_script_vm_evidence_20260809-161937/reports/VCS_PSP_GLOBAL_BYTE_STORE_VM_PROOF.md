# VCS PSP/PS2 opcode `04E5`: global-byte-store bridge

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x0008273C` | `script_vm_matcher/artifacts/review/vcs_empire_business_count/0008273c_linear_72.txt` |
| PS2 | `0x00185408` | `script_vm_matcher/artifacts/review/vcs_empire_business_count/00185408_FUN_00185408.c` |

Both handlers call `CollectParameters(..., 2, shared_parameter_buffer)`, then
perform one byte store and return `v0 = 0` (`CONTINUE`):

```text
address = engine_global_byte_region + raw_parameter[0]
*address = low_uint8(raw_parameter[1])
```

The PSP sequence is direct: it loads parameter zero, adds the platform's
global-base register and fixed byte-region displacement, then uses `sb` with
the second collected dword. The PS2 decompilation expresses the same operation
as `(&DAT_004cd4e7)[DAT_0050e1f8] = (undefined1)DAT_0050e1fc`.

No branch, condition reduction, output-store call or other parameter use is
present. The portable VM models the exact two-raw-parameter and low-byte
narrowing contract with `GlobalByteStoreAdapter(offset, value)`. The source
name/lifetime of that engine-global region is not asserted, so a public command
library label is deliberately not used as a native symbol name. Missing host
integration faults as `Opcode04E5RequiresGlobalByteStoreAdapter`.
