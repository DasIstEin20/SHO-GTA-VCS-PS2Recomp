# VCS recovered PS2 symbols

This inventory applies to the supplied **SLES_546.22** build. The names are
descriptive semantic labels derived from static control flow plus, where
noted, live PS2Recomp or preserved-recorder traces. They are not claims of
original Rockstar source identifiers.

`gta.csv` is the code-generation map. This document records why selected
anonymous entries were renamed and whether they are candidates for a Hybrid
HLE boundary.

## Bootstrap, core runtime and I/O

| Address | Label | Confidence | Recompile policy |
| --- | --- | --- | --- |
| `0x0017C308` | `StoreU16AtOffset2` | confirmed | keep recompiled |
| `0x00220B78` | `EnumerateDirectoryAndBuildEntries` | strong inference | keep recompiled |
| `0x00346BD0` | `GetCurrentLookupOwner` | confirmed | keep recompiled |
| `0x00346BE0` | `SetCurrentLookupOwner` | confirmed | keep recompiled |
| `0x00346C60` | `FindLinkedListEntryByCaseInsensitiveKey` | confirmed | keep recompiled |
| `0x00346E60` | `FindLinkedListEntryAndAddRef` | partial | keep recompiled |
| `0x003A5CA8` | `SelectCurrentLookupOwnerByIndex` | confirmed | keep recompiled |
| `0x003A5D10` | `SaveCurrentLookupOwnerForRestore` | confirmed | keep recompiled |
| `0x003E60A0` | `FindConfigurationRecordById` | confirmed | keep recompiled |
| `0x003F3D88` | `ApplyBootstrapConfigurationRecordModes` | strong inference | keep recompiled |
| `0x00461E60` | `CaseInsensitiveByteStringCompare` | confirmed | keep recompiled |
| `0x00465460` | `CaseFoldByteForComparison` | confirmed | keep recompiled |
| `0x0046BCC0` | `GetOrCreateGlobalSema48DAC4` | confirmed | keep recompiled |
| `0x0046BD18` | `WaitOnGlobalSema48DAC4` | confirmed | keep recompiled |

The live bootstrap trace proves a missing configuration child pointer can
cause `FindConfigurationRecordById` to return zero; the subsequent 16-bit
store then aliases the physical first RDRAM word. This is a diagnostic result,
not authorisation to patch the later linked-list consumer.

## RenderWare and game/render glue

| Address | Label | Confidence | Hybrid guidance |
| --- | --- | --- | --- |
| `0x002C03D0` | `RpClumpForAllAtomics` | confirmed | investigate a RenderWare cut |
| `0x00394488` | `CVehicleModelInfo_SetAtomicRendererCB` | confirmed | investigate a RenderWare cut |
| `0x00394C68` | `CVehicleModelInfo_SetAtomicRenderCallbacks` | confirmed | investigate a RenderWare cut |
| `0x002F5180` | `InitializePrimaryRenderStateConfiguration` | confirmed | investigate an HLE cut |
| `0x002F5EB8` | `SelectRenderState` | confirmed | investigate an HLE cut |
| `0x002F5F48` | `SetRenderStateSlot` | confirmed | investigate an HLE cut |
| `0x002F88F0` | `ApplyRenderStateCallbacksAndPacketTemplates` | confirmed | investigate an HLE cut |
| `0x002FB0A8` | `InitializePs2RenderBackendState` | strong inference | investigate an HLE cut |
| `0x002FC188` | `PrepareIndexedPrimitiveRenderState` | strong inference | investigate an HLE cut |

The callback dispatcher at `0x002F88F0` is above packet construction and is
therefore a better boundary candidate than a VIF/VU helper. Its indirect
callback target set still requires dynamic capture before any interception is
implemented.

## PS2-specific graphics backend

| Address | Label | Confidence | Hybrid guidance |
| --- | --- | --- | --- |
| `0x001A9910` | `SubmitVif0Dma` | confirmed | PS2 platform backend |
| `0x002F8430` | `BuildVif1MicroprogramPacket` | confirmed | PS2 platform backend |
| `0x002FC4C8` | `BuildVifPacketForIndexedPrimitiveList` | confirmed | PS2 platform backend |
| `0x002FCA18` | `BuildVifPacketForAlternateIndexedPrimitiveList` | strong inference | PS2 platform backend |
| `0x003708A8` | `ProgramGsDisplayState` | strong inference | PS2 platform backend |
| `0x00447850` | `ResetVif1AndVuState` | confirmed | PS2 platform backend |
| `0x004478F0` | `PollVif1GifVuIdle` | confirmed | PS2 platform backend |

In particular, `BuildVifPacketForIndexedPrimitiveList` accepts indexed
primitive data and produces VIF-oriented work. It is too low in the pipeline
to be treated as an RSL/librw geometry hook.

## Evidence rule

Add a new name only after recording its address, control-flow contract,
observable inputs/outputs and confidence. A convenient-looking match or a
single histogram similarity is not sufficient. When dynamic diagnostics prove
a contract, update both this document and `gta.csv` in the same change.
