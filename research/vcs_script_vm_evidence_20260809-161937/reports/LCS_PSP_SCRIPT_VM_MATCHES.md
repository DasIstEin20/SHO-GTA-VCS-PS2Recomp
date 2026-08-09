# LCS PSP Script VM candidates

These are bounded review candidates derived from exact Android anchors. None is a symbol mapping or an automatic rename.

| Android exact anchor | Best LCS PSP candidate | Score | Confidence | Disposition |
| --- | --- | --- | --- | --- |
| `CRunningScript::CollectParameters` | `0x00153d28` | 0.7622 | REVIEW | manual validation required |
| `CRunningScript::CollectNextParameterWithoutIncreasingPC` | `0x0010d8f8` | 0.6515 | REVIEW | manual validation required |
| `CRunningScript::StoreParameters` | `0x0011853c` | 0.8612 | REVIEW | manual validation required |
| `CTheScripts::Init` | `0x0002448c` | 0.7662 | REVIEW | manual validation required |
| `CTheScripts::StartNewScript` | `0x001cb754` | 0.8879 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands200To299` | `0x002a19f4` | 0.7188 | REVIEW | manual validation required |
| `CRunningScript::ReturnFromGosubOrFunction` | `0x002f41e0` | 0.7568 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands0To99` | `0x002a19f4` | 0.6866 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands100To199` | `0x0001278c` | 0.7371 | REVIEW | manual validation required |
| `CRunningScript::ProcessOneCommand` | `0x00153a90` | None | CALLGRAPH_CONFIRMED | confirmed by manual semantic review |
| `CRunningScript::Process` | `0x00117a44` | 0.7747 | REVIEW | manual validation required |
| `CTheScripts::Process` | `0x0017f2cc` | 0.7943 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands300To399` | `0x002348b4` | 0.6912 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands400To499` | `0x00184940` | 0.7599 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands600To699` | `0x000ced78` | 0.7144 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands700To799` | `0x0001278c` | 0.7485 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands500To599` | `0x0001278c` | 0.7408 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands900To999` | `0x00184940` | 0.7586 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands800To899` | `0x00184940` | 0.7696 | REVIEW | manual validation required |
| `CTheScripts::ReadObjectNamesFromScript` | `0x0015228c` | 0.8185 | REVIEW | manual validation required |
| `CTheScripts::ReadMultiScriptFileOffsetsFromScript` | `0x00079028` | 0.8051 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1100To1199` | `0x002dbc40` | 0.7013 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1000To1099` | `0x00184940` | 0.6715 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1200To1299` | `0x00184940` | 0.7413 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1300To1399` | `0x00184940` | 0.7392 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1400To1499` | `0x00184940` | 0.761 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1500To1599` | `0x00184940` | 0.7241 | REVIEW | manual validation required |
| `CRunningScript::ProcessCommands1600To1699` | `0x000ced78` | 0.7405 | REVIEW | manual validation required |
