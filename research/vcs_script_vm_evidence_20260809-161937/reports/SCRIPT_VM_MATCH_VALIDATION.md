# Script VM match validation

## Result

No automatic Ghidra rename was made. reLCS source → LCS Android is exact; LCS PSP `ProcessOneCommand` is manually confirmed at `0x00153a90`; VCS PSP `ProcessOneCommand` is manually confirmed at `0x0005e5e8` using the full Script-IP → opcode → NOT → table dispatch → return-status sequence.

## Input hashes

| Input | SHA-256 |
| --- | --- |
| lcs_android | `c46507f92e1e02bc7e79f476aba02fa731682fc5ca3c320624fa8a99e854adb1` |
| lcs_psp | `85fb68879359dbbc3f85c0747ef8dd61ff62a9b8219c12a5078f7d8867a7a160` |
| vcs_psp | `a722be366b2090676f225941235e7a01cb4a42b242b14e785180162f46376b12` |

## Policy

- `EXACT_SYMBOL` anchors must agree on address, namespace and name.
- Android → LCS PSP structural/p-code metrics produce `REVIEW` only.
- LCS PSP → VCS PSP uses relocation-neutral MIPS hashes, opcode profile, CFG/data/call measurements. A MIPS hash alone is not enough without an independently confirmed LCS semantic anchor.
- Old fuzzy four-way results are preserved separately as `UNCONFIRMED_REVIEW`.
