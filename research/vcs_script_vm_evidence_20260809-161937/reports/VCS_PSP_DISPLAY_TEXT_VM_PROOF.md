# VCS PSP/PS2 opcode `0203`: display-text VM/host boundary

| Platform | Handler target | Evidence |
| --- | --- | --- |
| PSP | `0x002EC548` | `script_vm_matcher/artifacts/review/vcs_display_text/002ec548_linear_160.txt` |
| PS2 | `0x003DBB10` | `script_vm_matcher/artifacts/review/vcs_display_text/003dbb10_FUN_003dbb10.c` |

Both handlers first collect two raw parameter dwords into the shared parameter
area, then separately collect one raw text-key/offset dword. The first two are
loaded as binary32 values into a bounded native display record; the third is
converted through an engine-owned text/GXT lookup path. The handlers copy at
most 63 UTF-16 code units into one of at most 20 queued text records, then
return `v0 = 0` (`CONTINUE`).

The real `MAIN.SCM` route confirms the parameter extent:

```text
0x007BFF: 0203
  00                    -> VCS plural-collector zero coordinate payload
  06 F3 7B 00 00        -> second raw coordinate payload
  06 00 D0 01 0A        -> separately collected text-key/offset payload
next opcode: 0x0006 at 0x007C0C
```

The PSP collector's tag-`0x00` table case writes zero and continues its count;
it is not imported from reLCS as an argument terminator. Its standalone
`CollectNextParameterWithoutIncreasingPC` path still routes tag `0x00` through
the low variable resolver range, so the portable API keeps those two entry
points distinct.

`DisplayTextAdapter(left_raw, top_raw, text_key_raw)` is therefore the correct
portable boundary. It receives the recovered three raw dwords in order; GXT
lookup, UTF-16 queue storage, text styling and rendering remain host work.
Missing adapters fault explicitly.
