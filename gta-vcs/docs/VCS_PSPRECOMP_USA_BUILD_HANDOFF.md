# VCS PSPRecomp — USA build handoff

## Verified target

- Image: *Grand Theft Auto: Vice City Stories (USA) (v1.03)*.
- PSP disc ID: `ULUS-10160`.
- Region: `NTSC-U`.
- Expected decrypted `EBOOT.ELF` SHA-256:
  `85c5e172fa0207086b72a37649e9a10d47845f0866b0dee179c48d930e43d17f`.

The image contains standard 2048-byte ISO9660 sectors. The game-data helper
extracts its `PSP_GAME` tree locally, then `prepare_game.ps1` copies it to the
VCS profile and writes the verified ELF as
`PSP_GAME/SYSDIR/EBOOT_DECRYPTED.ELF`. It does not implement EBOOT decryption:
the ELF must come from the user's legally obtained game image.

## Native build status

The current executable target is `VCSNative`, produced by the PSPRecomp VCS
profile. It is a native Windows DX12 host, not a PSP emulator. The host already
has PC keyboard, raw-mouse, and XInput input paths; PC-control refinement is a
separate mapping/UI task, not an emulator dependency.

For a 16-logical-CPU machine while preserving two threads for the shell, launch
the build with processor affinity `0x3FFF` (14 low logical CPUs), for example:

```bat
start "PSPRecomp VCS build" /b /wait /affinity 3FFF cmake --build out\vcs-fast --config Release --parallel 14 --target VCSNative -- /m:14
```

This is preferable to only lowering `/m`: MSVC may still create more compiler
processes, but the affinity mask prevents them from using the reserved CPUs.

## Architecture note for reVCS

`reVC` cannot directly run VCS data because it is the Vice City codebase. A
future reVCS port should use PSPRecomp's VCS behavior and generated code as a
semantic oracle while migrating VCS systems into reVC-style world, model, and
renderer interfaces. Do not run both implementations as independent world
owners. That path keeps room for PC input, high-resolution assets, a mod API,
and future DLAA/DLSS work; the latter also needs depth, motion vectors, and
camera jitter, not merely vector geometry.

## Provenance

The implementation-side source change is retained locally in PSPRecomp commit
`89b3b20` (`fix(vcs): prepare verified US game data`), which adds the generic
ISO9660 extractor and corrects the USA ELF hash. This handoff records the
portable findings in the PS2Recomp fork's own history.
