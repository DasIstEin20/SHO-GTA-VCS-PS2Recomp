[CmdletBinding()]
param(
    [string]$Workspace = (Split-Path $PSScriptRoot -Parent),
    [string]$GhidraHome = (Join-Path (Split-Path $PSScriptRoot -Parent) 'tools\ghidra_12.1.2_PUBLIC'),
    [string]$ProjectDirectory = (Join-Path $PSScriptRoot 'work\ghidra-20260809-012231')
)

$ErrorActionPreference = 'Stop'
$headless = Join-Path $GhidraHome 'support\analyzeHeadless.bat'
$scriptDir = Join-Path $PSScriptRoot 'ghidra'
$inputs = Join-Path $PSScriptRoot 'artifacts\semantic_inputs'
$exports = Join-Path $PSScriptRoot 'artifacts\exports'
$comparison = Join-Path $Workspace 'VCS_PSP_PS2_OPCODE_COMPARE.json'
$required = @($headless, $comparison, (Join-Path $scriptDir 'ExportOpcodeHandlerSemanticFeatures.java'))
$missing = $required | Where-Object { -not (Test-Path -LiteralPath $_) }
if ($missing) { throw "Missing required semantic comparison input(s): $($missing -join '; ')" }

& python (Join-Path $PSScriptRoot 'src\prepare_common_opcode_semantic_inputs.py') --comparison $comparison --psp-output (Join-Path $inputs 'vcs_psp_common_handlers.tsv') --ps2-output (Join-Path $inputs 'vcs_ps2_common_handlers.tsv') --manifest (Join-Path $inputs 'vcs_common_handler_targets.json')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$previousAppData, $previousLocalAppData = $env:APPDATA, $env:LOCALAPPDATA
try {
    $env:APPDATA = Join-Path $PSScriptRoot 'work\ghidra-user\AppData\Roaming'
    $env:LOCALAPPDATA = Join-Path $PSScriptRoot 'work\ghidra-user\AppData\Local'
    & $headless $ProjectDirectory 'scriptvm_vcs_psp' -process 'GTA_VCS_Debug_EBOOT.ELF' -noanalysis -scriptPath $scriptDir -postScript 'ExportOpcodeHandlerSemanticFeatures.java' (Join-Path $inputs 'vcs_psp_common_handlers.tsv') (Join-Path $exports 'vcs_psp_opcode_handler_semantics.json')
    if ($LASTEXITCODE -ne 0) { throw "PSP semantic-feature export failed with exit code $LASTEXITCODE" }
    & $headless $ProjectDirectory 'scriptvm_vcs_ps2' -process 'SLES_546.22' -noanalysis -scriptPath $scriptDir -postScript 'ExportOpcodeHandlerSemanticFeatures.java' (Join-Path $inputs 'vcs_ps2_common_handlers.tsv') (Join-Path $exports 'vcs_ps2_opcode_handler_semantics.json')
    if ($LASTEXITCODE -ne 0) { throw "PS2 semantic-feature export failed with exit code $LASTEXITCODE" }
} finally {
    $env:APPDATA = $previousAppData
    $env:LOCALAPPDATA = $previousLocalAppData
}

& python (Join-Path $PSScriptRoot 'src\compare_common_opcode_handler_semantics.py') --comparison $comparison --psp-features (Join-Path $exports 'vcs_psp_opcode_handler_semantics.json') --ps2-features (Join-Path $exports 'vcs_ps2_opcode_handler_semantics.json') --output (Join-Path $Workspace 'VCS_PSP_PS2_COMMON_SEMANTICS.json') --markdown (Join-Path $Workspace 'reports\VCS_PSP_PS2_COMMON_SEMANTICS.md')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Wrote VCS PSP ↔ PS2 COMMON_SLOT semantic-shape comparison to $Workspace"
