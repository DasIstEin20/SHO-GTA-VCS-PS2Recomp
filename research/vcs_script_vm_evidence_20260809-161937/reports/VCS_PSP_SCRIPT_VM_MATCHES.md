# VCS PSP Script VM candidates

`CRunningScript::ProcessOneCommand` is confirmed at `0x0005e5e8` through the instruction-level fetch/NOT/table-dispatch sequence and its caller's return-status loop. VCS uses a data-driven table rather than direct LCS-style page callees.

| Android exact anchor | LCS PSP anchor | VCS PSP candidate | Confidence | Disposition |
| --- | --- | --- | --- | --- |
| `CRunningScript::CollectParameters` | `0x00153d28` | `0x0005e71c` | REVIEW | review only |
| `CRunningScript::CollectNextParameterWithoutIncreasingPC` | `0x0010d8f8` | `0x00306110` | REVIEW | review only |
| `CRunningScript::StoreParameters` | `0x0011853c` | `0x00212e28` | REVIEW | review only |
| `CTheScripts::Init` | `0x0002448c` | `0x000a9d94` | REVIEW | review only |
| `CTheScripts::StartNewScript` | `0x001cb754` | `0x001380f8` | REVIEW | review only |
| `CRunningScript::ProcessCommands200To299` | `0x002a19f4` | `0x001f9f58` | REVIEW | review only |
| `CRunningScript::ReturnFromGosubOrFunction` | `0x002f41e0` | `0x0036038c` | REVIEW | review only |
| `CRunningScript::ProcessCommands0To99` | `0x002a19f4` | `0x001f9f58` | REVIEW | review only |
| `CRunningScript::ProcessCommands100To199` | `0x0001278c` | `0x00039320` | REVIEW | review only |
| `CRunningScript::ProcessOneCommand` | `0x00153a90` | `0x0005e5e8` | CALLGRAPH_CONFIRMED | confirmed by manual semantic review |
| `CRunningScript::Process` | `0x00117a44` | `0x00212330` | REVIEW | review only |
| `CTheScripts::Process` | `0x0017f2cc` | `0x001d1c84` | REVIEW | review only |
| `CRunningScript::ProcessCommands300To399` | `0x002348b4` | `0x00257b14` | REVIEW | review only |
| `CRunningScript::ProcessCommands400To499` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands600To699` | `0x000ced78` | `0x000c0228` | REVIEW | review only |
| `CRunningScript::ProcessCommands700To799` | `0x0001278c` | `0x00039320` | REVIEW | review only |
| `CRunningScript::ProcessCommands500To599` | `0x0001278c` | `0x00039320` | REVIEW | review only |
| `CRunningScript::ProcessCommands900To999` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands800To899` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CTheScripts::ReadObjectNamesFromScript` | `0x0015228c` | `0x00156a98` | REVIEW | review only |
| `CTheScripts::ReadMultiScriptFileOffsetsFromScript` | `0x00079028` | `0x0006041c` | REVIEW | review only |
| `CRunningScript::ProcessCommands1100To1199` | `0x002dbc40` | `0x001c12c4` | REVIEW | review only |
| `CRunningScript::ProcessCommands1000To1099` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands1200To1299` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands1300To1399` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands1400To1499` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands1500To1599` | `0x00184940` | `0x001ba264` | REVIEW | review only |
| `CRunningScript::ProcessCommands1600To1699` | `0x000ced78` | `0x000c0228` | REVIEW | review only |
