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

## Native script-storage boundary

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x001790D0` | `vcs_CFileMgr_SetDir` | confirmed semantic | map to scoped VCS data-root resolution |
| `0x00179168` | `vcs_CFileMgr_OpenFile` | confirmed semantic | map to directory/ISO open |
| `0x00179188` | `vcs_CFileMgr_Read` | confirmed semantic | preserve exact SCM byte ranges |
| `0x001791B8` | `vcs_CFileMgr_Seek` | confirmed semantic | preserve mission offset plus 8 |
| `0x001791E8` | `vcs_CFileMgr_CloseFile` | confirmed semantic | retain Init/Shutdown ownership |

PS2 bootstrap opens `RUNDATA/main.scm` in `rb` mode, retains the handle for
mission streaming and closes it during `CTheScripts::Shutdown`. These five
wrappers are the appropriate filesystem/ISO host cut; full contracts and PSP
pairs are recorded in `reports/VCS_PS2_SCRIPT_FILEMGR_BOUNDARY.md`.

## Script VM parameter ABI

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x00254D18` | `vcs_CRunningScript_CollectParameters` | confirmed semantic | preserve tag dispatch and shared parameters |
| `0x00254F50` | `vcs_CRunningScript_CollectNextParameterWithoutIncreasingPC` | confirmed semantic | keep private-cursor peek semantics |
| `0x002550F8` | `vcs_CRunningScript_StoreParameters` | confirmed semantic | resolve and store raw dwords |
| `0x00255180` | `vcs_GetPointerToScriptVariable` | confirmed semantic | preserve VCS local/global tag ranges |
| `0x002552F0` | `vcs_CRunningScript_GetPointerToScriptVariable` | confirmed semantic | retain member-wrapper ABI |

Tag `0x00` in plural collection is now instruction-proven `ARGUMENT_END`: it
consumes one byte and leaves destination values unchanged. Zero literals are
tags `0x01`/`0x02`; the standalone peek still routes `0x00` through its low
local-variable resolver. The corrected contract and PSP jump-table proof are
in `reports/VCS_PS2_PARAMETER_ABI_BOUNDARY.md`.

## Native gameplay/VM bridge candidates

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x001A7C30` | `vcs_CModelInfo_GetModelInfoByName` | confirmed semantic | bridge SCM names to native model registry |
| `0x0018F8D8` | `vcs_CWorld_Add` | strong inference | bridge to reVC world registration |
| `0x00196758` | `vcs_CWorld_FindGroundZForCoord` | confirmed semantic | bridge to reVC collision/world query |
| `0x001DDB18` | `vcs_CPickups_GenerateNewOne` | confirmed semantic | VCS-aware native pickup manager |
| `0x001E2378` | `vcs_CPickup_GiveUsAPickUpObject` | confirmed semantic | native pickup object construction |
| `0x001E2C70` | `vcs_CPickups_GetNewUniquePickupIndex` | confirmed semantic | preserve VCS handle generation |
| `0x001E2CC0` | `vcs_CPickups_GetActualPickupIndex` | confirmed semantic | preserve VCS handle validation |
| `0x00255418` | `vcs_CTheScripts_Shutdown` | confirmed semantic | release native SCM resources |
| `0x00255490` | `vcs_CTheScripts_Init` | confirmed semantic | central native SCM bootstrap boundary |
| `0x0025B3E8` | `vcs_CTheScripts_ReadObjectNamesFromScript` | confirmed semantic | parse VCS used-object names |
| `0x0025B508` | `vcs_CTheScripts_UpdateObjectIndices` | confirmed semantic | populate native model indices |
| `0x0025B5A8` | `vcs_CTheScripts_ReadMultiScriptFileOffsetsFromScript` | confirmed semantic | populate VCS mission stream table |
| `0x002D7CE0` | `vcs_Opcode013F_CreatePickup` | confirmed semantic | semantic VM to native host |
| `0x002D8050` | `vcs_Opcode01F9_CreatePickupWithAmmo` | confirmed semantic | semantic VM to native host |
| `0x00366660` | `vcs_CKeyGen_GetUppercaseKey` | confirmed semantic | preserve model-name hash compatibility |

The negative model-object translation in both opcodes is inline, not a hidden
engine function: records have stride `0x1C` and their resolved model index is
at `+0x18`. This is the `UsedObjectArray[-model].index` contract a reVC-derived
host should populate during SCM bootstrap. The full ABI and PSP/PS2 address
pairs are recorded in `reports/VCS_PS2_PICKUP_PORT_BOUNDARY.md`.

The producer side is now recovered too: script bootstrap copies 24-byte names
into `0x1C` records, then resolves records 1..N-1 with an uppercase CRC model
lookup. Preserve the `-1` missing-model sentinel when adapting this path to
reVC. Exact PS2/PSP pairs and the table layout are recorded in
`reports/VCS_PS2_USED_OBJECT_BOOTSTRAP.md`.

`vcs_CTheScripts_Init` is the wider integration cut containing those parsers.
It owns the script-thread pool/list bootstrap, `main.scm` allocation and load,
mission/UI array resets and optional save-state load. Replace its PS2 platform
services at the host edge, but keep VCS structure counts, sentinels and call
order inside the compatibility layer.

## Car-generator and garage gameplay boundaries

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x003D1BE8` | `vcs_Opcode00A9_CreateCarGenerator` | confirmed semantic | semantic VM-to-host bridge |
| `0x003D1C88` | `vcs_Opcode00AA_SwitchCarGenerator` | confirmed semantic | semantic VM-to-host bridge |
| `0x003D9718` | `vcs_CCarGenerator_SwitchOff` | confirmed semantic | preserve VCS active-count behavior |
| `0x003D9730` | `vcs_CCarGenerator_SwitchOn` | confirmed semantic | preserve VCS timer initialization |
| `0x003D9770` | `vcs_CCarGenerator_CalcNextGen` | strong inference | map to reVC timing only after parity tests |
| `0x003DA0B0` | `vcs_CCarGenerator_Setup` | confirmed semantic | preserve VCS byte/uint16 narrowing |
| `0x003DA660` | `vcs_CTheCarGenerators_CreateCarGenerator` | confirmed semantic | adapt the VCS 200-slot, `0x30`-byte record contract |
| `0x003DA700` | `vcs_CTheCarGenerators_Init` | confirmed semantic | retain VCS pool size and aggregate reset |
| `0x002BF950` | `vcs_Opcode0145_SetGarage` | confirmed semantic | semantic VM-to-host bridge |
| `0x0025CA80` | `vcs_CGarages_AddOne` | confirmed semantic | adapt the VCS 32-slot, `0xD0`-byte record contract |
| `0x002C0130` | `vcs_Opcode0353_ChangeGarageType` | confirmed semantic | preserve low-byte type write |
| `0x002BFEF8` | `vcs_Opcode0247_OpenGarage` | confirmed semantic | preserve signed-16 index narrowing |
| `0x0025D650` | `vcs_CGarages_OpenGarage` | strong inference | bridge the state transition, door and collision update |

The diagnostic PC registries can now consume the real PSP bootstrap route:
194 generator requests produce 194 records (169 active), and ten garage
requests produce ten records. They deliberately do not materialize vehicles,
doors or collision. These are compatibility fixtures for wiring the VCS
script ABI into reVC-owned gameplay managers, not substitute game systems.

## Bootstrap vertical-slice gameplay symbols

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x00184A60` | `vcs_Opcode01E0_SetProgressTotal` | confirmed semantic | map to `CStats::TotalProgressInGame` |
| `0x00184E58` | `vcs_Opcode0296_SetTotalNumberOfMissionsNoOp` | confirmed semantic | keep the VCS parameter-consuming no-op |
| `0x00185138` | `vcs_Opcode03AC_SetUniqueJumpsFound` | confirmed semantic | map to the found-count field, not progress total |
| `0x00272DC0` | `vcs_Opcode00CB_AddHospitalRestart` | confirmed semantic | restart-manager host bridge |
| `0x00272E80` | `vcs_Opcode00CC_AddPoliceRestart` | confirmed semantic | restart-manager host bridge |
| `0x00287FE8` | `vcs_Opcode03B7_SetCameraPedZoomIndicator` | confirmed semantic | update only in FOLLOWPED mode |
| `0x002D2788` | `vcs_Opcode0028_CreatePlayer` | confirmed semantic | player/ped construction bridge; linear dispatcher target |
| `0x002D2E80` | `vcs_Opcode009C_SetDeatharrestState` | confirmed semantic | preserve VCS `parameter != 0` rule |
| `0x002D3318` | `vcs_Opcode0128_SetMaxWantedLevel` | confirmed semantic | wanted-system host bridge |
| `0x002D5070` | `vcs_Opcode0363_SetShortcutPickupPoint` | confirmed semantic | game-logic endpoint bridge |
| `0x002D50D8` | `vcs_Opcode0364_SetShortcutDropoffPointForMission` | confirmed semantic | game-logic endpoint bridge |
| `0x002D5868` | `vcs_Opcode03F8_StorePlayerOutfit` | confirmed semantic | preserve the exact eight-byte copy |
| `0x0030D3C0` | `vcs_Opcode00D3_SetObjectHeading` | confirmed semantic | world remove/rotate/re-add bridge |
| `0x0030D728` | `vcs_Opcode0117_DontRemoveObject` | confirmed semantic | clear VCS mission-cleanup ownership |
| `0x0030D798` | `vcs_Opcode01AA_CreateObject` | confirmed semantic | model/world/object-pool bridge |
| `0x004062B0` | `vcs_Opcode0403_SetClockEventWarning` | confirmed semantic | HUD clock-warning bridge |
| `0x004142A8` | `vcs_Opcode00CF_SetCharHeading` | confirmed semantic | on-foot ped rotation bridge |

## Extended PSP bootstrap-route opcode symbols

| Address | Label | Confidence | Native-port guidance |
| --- | --- | --- | --- |
| `0x0010B6F0` | `vcs_Opcode00A2_ClearCharObjective` | confirmed semantic | bind to the full `CPed` objective-reset path; invalid handles are not native no-ops |
| `0x00183470` | `vcs_Opcode03F1_SwitchEmergencyServices` | confirmed semantic | normalize the raw dword with `!= 0`, then update emergency traffic state |
| `0x0021B900` | `vcs_Opcode0493_GetInteriorRadius` | confirmed semantic | return the collision-model radius as a raw binary32 dword; missing IDs yield `+0.0f` |
| `0x002D2C70` | `vcs_Opcode0098_AlterWantedLevel` | confirmed semantic | player-table plus wanted-system bridge |
| `0x002D3C20` | `vcs_Opcode026E_CanPlayerStartMission` | confirmed semantic | bind to `CPed::CanStartMission` plus the driving-state allowance |
| `0x002D4940` | `vcs_Opcode0318_IsPlayerWearing` | confirmed semantic | preserve lowercase-in-place name hashing and `CModelInfo` comparison |
| `0x002D4F08` | `vcs_Opcode035B_IsPlayerInInfoZone` | confirmed semantic | select the containing info zone first, then compare its exact eight-byte label |
| `0x003166D0` | `vcs_Opcode0080To0085_LocateChar2D` | confirmed semantic | shared six-opcode 2D locate family; preserve on-foot/in-car and stopped modes |
| `0x00316710` | `vcs_Opcode0089To008E_LocateChar3D` | confirmed semantic | shared six-opcode 3D locate family; preserve on-foot/in-car and stopped modes |
| `0x00390C40` | `vcs_Opcode0202_IsAreaOccupied` | confirmed semantic | collision/world-sector query with six bounds and five filter flags |
| `0x00392008` | `vcs_Opcode0130_ReadKillFrenzyStatus` | confirmed semantic | store the raw global rampage status through one SCM output |
| `0x003D3BF0` | `vcs_Opcode01F6_GetRandomCarOfTypeInArea` | confirmed semantic | reverse vehicle-pool scan, model/area filters and `-1` miss sentinel |
| `0x004128E8` | `vcs_Opcode0043_GetCharCoordinates` | confirmed semantic | use the current vehicle transform when present; store three raw binary32 dwords |
| `0x00413828` | `vcs_Opcode007E_IsCharInAnyCar` | confirmed semantic | test the general vehicle pointer/predicate, distinct from seated-state opcode `02A7` |
| `0x00413FD8` | `vcs_Opcode00B0_IsCharInZone` | confirmed semantic | resolve the named zone before testing the ped/vehicle position |
| `0x00415340` | `vcs_Opcode014B_SetCharHealth` | confirmed semantic | preserve the special zero-health death/vehicle-exit branch |
| `0x004173A0` | `vcs_Opcode0220_SetCharWaitState` | confirmed semantic | negative duration maps to `CPed::SetWaitState(..., nullptr)` |
| `0x00417BD0` | `vcs_Opcode026A_IsCharInControl` | confirmed semantic | use ped state, flags and health through the native control predicate |
| `0x004186A0` | `vcs_Opcode02A7_IsCharSittingInAnyCar` | confirmed semantic | preserve the exact seated-state and objective exclusion checks |
| `0x0041B3E8` | `vcs_Opcode0352_ClearAllCharAnims` | confirmed semantic | retain state exclusions and the complete `CPed` animation reset sequence |
| `0x00100470` | `vcs_Opcode031D_GetDistanceBetweenCoords2D` | confirmed semantic | preserve four binary32 inputs and single-precision `sqrt(dx*dx + dy*dy)` |
| `0x0012BCB0` | `vcs_Opcode0160_RequestModel` | confirmed semantic | resolve `USED_OBJECTS`, preserve mission flags, script owner and the `0x16A→0x16E` pair |
| `0x0012BE70` | `vcs_Opcode0161_HasModelLoaded` | confirmed semantic | query both normal streaming and supplemental model cache before condition reduction |
| `0x0012BFF0` | `vcs_Opcode0162_MarkModelAsNoLongerNeeded` | confirmed semantic | retain distinct mission/non-mission release paths and clear streaming ownership |
| `0x0017E798` | `vcs_Opcode047B_GetEmpireBuildingTypedPoolHandle` | confirmed semantic | resolve empire entity, return `-1` on miss, otherwise OR the pool handle with `0x00800000` |
| `0x001CCB18` | `vcs_Opcode0427_GetNameOfInfoZone` | confirmed semantic | use the distinct info-zone selector and store its ScriptSpace label offset |
| `0x001CCBB8` | `vcs_Opcode0428_GetZonePedInfoTimeOfDay` | confirmed semantic | narrow time selector to byte and store exactly eleven population values |
| `0x001E61F0` | `vcs_Opcode047D_GetCountInteriorElementsByType` | confirmed semantic | count `0x30`-byte interior records by leading type byte; missing data stores zero |
| `0x002D5688` | `vcs_Opcode03C0_IsPlayerMadeSafe` | confirmed semantic | test player-zero state bit `0x20` through the shared condition reducer |
| `0x002D4E00` | `vcs_Opcode0354_AreWantedStarsFlashing` | confirmed binary contract | test whether the current player's secondary wanted-star value exceeds its primary value |
| `0x002D2D40` | `vcs_Opcode009A_IsWantedLevelGreater` | confirmed semantic | resolve `CPlayerInfo`/`CWanted` and compare the integer wanted level with strict greater-than |
| `0x00287E20` | `vcs_Opcode0221_SetCameraBehindPlayer` | confirmed semantic | make the single zero-parameter call into the camera manager |
| `0x00287A80` | `vcs_Opcode00B9_SetFixedCameraPosition` | confirmed semantic | forward two raw xyz vectors to the fixed-camera manager |
| `0x00287B08` | `vcs_Opcode00BA_PointCameraAtPoint` | confirmed semantic | resolve ground for VCS z sentinels, narrow the mode and take scripted camera control |
| `0x00287D70` | `vcs_Opcode01CF_RestoreCameraJumpCut` | confirmed semantic | make the single zero-parameter restore-with-jump-cut call |
| `0x002C01E0` | `vcs_Opcode01F7_HasResprayHappened` | confirmed semantic | narrow one garage selector and reduce the garage-manager respray result as a condition |
| `0x002D6360` | `vcs_Opcode0549_SetRadarBlipByRange` | confirmed semantic | update the 75-slot radar manager using the raw binary32 range |
| `0x00311BA0` | `vcs_Opcode0559_SetObjectFlag10000000` | confirmed binary contract | resolve `CObject` and OR bit `0x10000000` at native flag word `+0x1E0`; public bit name remains unknown |
| `0x00361B48` | `vcs_Opcode0474_IsPagerOn` | confirmed semantic | query the active pager/text-manager entry through the shared condition reducer |
| `0x00390BB8` | `vcs_Opcode01BB_GetGroundZFor3DCoord` | confirmed semantic | bind to the vertical world ray; a miss stores raw `+0.0f` |
| `0x003D06B0` | `vcs_Opcode0048_CreateCar` | confirmed semantic | select concrete vehicle class, place it, initialize script state, world-add and return pool handle |
| `0x003D0EC8` | `vcs_Opcode004D_GetCarCoordinates` | confirmed semantic | resolve one `CVehicle`, copy raw transform xyz and store three SCM outputs |
| `0x003D1348` | `vcs_Opcode004F_IsCarStillAlive` | confirmed semantic | require a valid vehicle and the shared native dead/wrecked predicate to be false |
| `0x003D18C8` | `vcs_Opcode00A0_IsCarDead` | confirmed semantic | treat a failed pool lookup as dead; otherwise use the shared dead/wrecked predicate |
| `0x003D1E28` | `vcs_Opcode00D1_SetCarHeading` | confirmed semantic | rotate the resolved vehicle by degrees times `0.017453294` |
| `0x003D4330` | `vcs_Opcode0246_ClearAreaOfCars` | confirmed semantic | normalize all three AABB axes before clearing vehicles from the world |
| `0x003D9290` | `vcs_Opcode04DD_MakeVehicleInvisible` | confirmed semantic | resolve the vehicle render object and set native visibility to false |
| `0x003DAC80` | `vcs_Opcode0057_PrintNow` | confirmed semantic | resolve one GXT key, collect duration/style and add a jump-queue message |
| `0x003DAF50` | `vcs_Opcode011E_PrintWithNumber` | confirmed semantic | resolve GXT, preserve number/duration, narrow style and use fixed unused substitutions |
| `0x003DC610` | `vcs_Opcode0267_PrintHelp` | confirmed semantic | resolve one GXT key and install non-persistent HUD help with fixed VCS flags |
| `0x001E2F68` | `vcs_Opcode01AE_SwitchWidescreen` | confirmed semantic | select camera widescreen on/off by exact zero/nonzero input |
| `0x003DBD28` | `vcs_Opcode0204_SetTextScale` | confirmed binary contract | store two raw binary32 scale words in the next intro-text record |
| `0x003DBDA0` | `vcs_Opcode0205_SetTextColour` | confirmed semantic | truncate four parameters to RGBA bytes for the next intro-text record |
| `0x003DBF80` | `vcs_Opcode0208_SetTextWrapX` | confirmed semantic | store raw wrap X and clamp only ordered values above `480.0f` |
| `0x003DAAC8` | `vcs_Opcode0055_PrintBig` | confirmed semantic | resolve GXT, preserve duration and pass wrapped `int16(style)-1` to big-message queue |
| `0x00416938` | `vcs_Opcode0200_SetCharVisible` | confirmed binary contract | toggle mutually exclusive `CPed` visibility bits by exact zero/nonzero input |
| `0x003D1428` | `vcs_Opcode0050_SetCarCruiseSpeed` | confirmed semantic | narrow requested speed to signed byte and clamp against handling max cruise velocity |
| `0x002D3358` | `vcs_Opcode012E_SetPoliceIgnorePlayer` | confirmed semantic | toggle wanted ignore bit and stop active law enforcers only on enable |
| `0x0012BFC0` | `vcs_Opcode0228_LoadAllModelsNow` | confirmed semantic | suspend timer, synchronously load requested models with false, then resume timer |
| `0x00265C88` | `vcs_Opcode044E_SetEmpireHudElementVisibleState` | confirmed semantic | normalize state to bool and set visibility for one VCS empire-HUD element |
| `0x00265D40` | `vcs_Opcode0450_SetEmpireHudElementStartPosition` | confirmed binary contract | forward element ID plus two raw position words |
| `0x00265DB8` | `vcs_Opcode0451_SetEmpireHudElementSize` | confirmed binary contract | forward element ID plus two raw size words |
| `0x00265E30` | `vcs_Opcode0452_SetEmpireHudElementBackgroundColour` | confirmed semantic | truncate four channels and set element background RGBA |
| `0x00265ED8` | `vcs_Opcode0453_SetEmpireHudElementBorderColour` | confirmed semantic | truncate four channels and set element border RGBA |
| `0x00265F80` | `vcs_Opcode0454_AddEmpireHudTextElement` | confirmed semantic | resolve GXT, collect three inputs, create text and store its returned ID |
| `0x00266270` | `vcs_Opcode0457_SetEmpireHudElementColour` | confirmed semantic | truncate four channels and set element foreground RGBA |
| `0x00266318` | `vcs_Opcode0458_SetEmpireHudTextElementSize` | confirmed binary contract | forward text-element ID and one raw size word |
| `0x003DCAF0` | `vcs_Opcode0323_PrintHelpForever` | confirmed semantic | resolve one ScriptSpace GXT key and install it as the persistent HUD help message |
| `0x00405978` | `vcs_Opcode0269_FlashHudObject` | confirmed semantic | lazily initialize HUD state and forward one narrowed object selector |
| `0x00415D00` | `vcs_Opcode01C2_IsCurrentCharWeapon` | confirmed semantic | resolve `CPed`, select its current weapon slot and compare the requested weapon type exactly |
| `0x00413728` | `vcs_Opcode007D_IsCharInModel` | confirmed semantic | resolve `CPed`, require a current vehicle and compare its signed 16-bit model index |
| `0x0041CDA0` | `vcs_Opcode042C_IsArmourHealthGreaterThan` | confirmed semantic | resolve `CPed`, convert the integer threshold to float and strictly compare armour at `+0x4E8` |
| `0x004144B8` | `vcs_Opcode00D7_IsCharHealthGreater` | confirmed semantic | resolve `CPed`, convert the integer threshold to float and compare health with strict greater-than |
| `0x004145D0` | `vcs_Opcode0106_GiveWeaponToChar` | confirmed semantic | give weapon/ammo, select returned slot and perform native ped/vehicle weapon-model transitions |
| `0x00414790` | `vcs_Opcode010B_SetCurrentCharWeapon` | confirmed semantic | use the distinct unarmed path or scan ten weapon slots and select each matching type |
| `0x00414A20` | `vcs_Opcode0112_MarkCharAsNoLongerNeeded` | confirmed semantic | release a valid pool ped and conditionally unregister mission cleanup from thread context |
| `0x00415420` | `vcs_Opcode014D_GetCharHealth` | confirmed semantic | resolve `CPed`, truncate its float health to int32 and store one SCM output |
| `0x004197D8` | `vcs_Opcode02E1_IsCharInWater` | confirmed binary contract | resolve `CPed` and test bit 40 of its 64-bit flag word at `+0xE8` |
| `0x00419110` | `vcs_Opcode02C8_IsCharOnAnyBike` | confirmed semantic | resolve the ped's vehicle and require native vehicle-class discriminator `2` |
| `0x00418788` | `vcs_Opcode02A8_IsCharOnFoot` | confirmed semantic | reject the native occupancy predicate and ped states `0x11`/`0x12` |
| `0x0012BD80` | `vcs_Opcode041A_RequestModelName` | confirmed semantic | resolve model name, retain owner/script-name metadata and request both `0x16A`/`0x16E` for the special model |
| `0x001E3188` | `vcs_Opcode02EA_SetAreaVisible` | confirmed semantic | store one raw area selector and rebuild renderer visibility |
| `0x001E31D0` | `vcs_Opcode0313_SetExtraColours` | confirmed semantic | preserve colour selector and normalize only the fade flag |
| `0x002D2978` | `vcs_Opcode0094_AddScore` | confirmed semantic | add signed money delta and clamp a negative result to zero |
| `0x003DDA10` | `vcs_Opcode04CD_SetAreaName` | confirmed semantic | resolve GXT key and select visible/hidden HUD area-name state |
| `0x004063B0` | `vcs_Opcode0479_CreateUser3DMarker` | confirmed semantic | reuse the generation-tagged script-marker pool with marker type 7 |
| `0x00406500` | `vcs_Opcode047A_RemoveUser3DMarker` | confirmed semantic | remove a generation-tagged user marker; stale handles are no-ops |
| `0x00412848` | `vcs_Opcode003F_DeleteChar` | confirmed semantic | delete a valid non-protected ped and conditionally unregister mission cleanup |
| `0x00419968` | `vcs_Opcode02EE_GetOffsetFromCharInWorldCoords` | confirmed semantic | multiply local xyz by the ped transform, add translation and store world xyz |

These labels were accepted only after direct PS2 decompilation and a paired
PSP table/handler review. The portable VM exposes engine-owned work through
host adapters so a reVC-derived PC host can supply `CPed`, `CWanted`, world,
interior and traffic managers without baking diagnostic registry behavior into
SCM execution.

Two source divergences are now instruction-proven. VCS `0296` consumes its
parameter without updating a statistic, unlike reLCS. VCS `009C` treats every
nonzero value as enabled, whereas the reLCS implementation checks for exactly
one. The VCS compatibility layer must retain those semantics when reusing
reVC/reLCS managers.

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
