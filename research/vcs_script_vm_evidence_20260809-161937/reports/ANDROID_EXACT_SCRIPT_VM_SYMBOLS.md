# Android LCS exact Script VM symbols

Validated **28** address + namespace + C++ symbol anchors against SHA-256 `c46507f92e1e02bc7e79f476aba02fa731682fc5ca3c320624fa8a99e854adb1`. Source is reLCS commit `snapshot (commit metadata unavailable)`; `ReLCS.dll` is not an input.

| Symbol | Android address | Role | Confidence |
| --- | --- | --- | --- |
| `CRunningScript::CollectParameters` | `0x002fb6f4` | parameter_decode | EXACT_SYMBOL |
| `CRunningScript::CollectNextParameterWithoutIncreasingPC` | `0x002fb8b0` | parameter_decode | EXACT_SYMBOL |
| `CRunningScript::StoreParameters` | `0x002fb9e4` | parameter_encode | EXACT_SYMBOL |
| `CTheScripts::Init` | `0x002fbea4` | script_loader | EXACT_SYMBOL |
| `CTheScripts::StartNewScript` | `0x002fc784` | script_spawn | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands200To299` | `0x002fc858` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ReturnFromGosubOrFunction` | `0x002fe4e4` | script_vm_support | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands0To99` | `0x002fe70c` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands100To199` | `0x00301a4c` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessOneCommand` | `0x00304a14` | opcode_dispatcher | EXACT_SYMBOL |
| `CRunningScript::Process` | `0x00304bd4` | running_scheduler | EXACT_SYMBOL |
| `CTheScripts::Process` | `0x00304d68` | global_scheduler | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands300To399` | `0x00305160` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands400To499` | `0x00307420` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands600To699` | `0x0030a0a0` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands700To799` | `0x0030b888` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands500To599` | `0x0030e940` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands900To999` | `0x0031180c` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands800To899` | `0x003144e4` | command_partition | EXACT_SYMBOL |
| `CTheScripts::ReadObjectNamesFromScript` | `0x0031e8c8` | bytecode_reader | EXACT_SYMBOL |
| `CTheScripts::ReadMultiScriptFileOffsetsFromScript` | `0x0031eac0` | bytecode_reader | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1100To1199` | `0x0031ed84` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1000To1099` | `0x00321ca4` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1200To1299` | `0x00324118` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1300To1399` | `0x00327498` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1400To1499` | `0x0032a420` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1500To1599` | `0x0032d398` | command_partition | EXACT_SYMBOL |
| `CRunningScript::ProcessCommands1600To1699` | `0x00330490` | command_partition | EXACT_SYMBOL |

No Ghidra names were changed.
