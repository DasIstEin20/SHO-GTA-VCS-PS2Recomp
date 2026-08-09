[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string] $PpssppHeadless,

    [string] $Game = (Join-Path $PSScriptRoot "..\..\GTA VCS [Debug Build]"),

    [int] $DebuggerPort = 16482,

    [string[]] $AdditionalArgument = @(),

    [switch] $Wait
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $PpssppHeadless -PathType Leaf)) {
    throw "PPSSPPHeadless executable not found: $PpssppHeadless"
}

if ((Split-Path -Leaf $PpssppHeadless) -notmatch 'Headless') {
    throw 'Refusing to start a GUI PPSSPP binary. Supply a stock PPSSPPHeadless.exe build.'
}

if (-not (Test-Path -LiteralPath $Game)) {
    throw "Game image/path not found: $Game"
}

Write-Host "Starting stock PPSSPPHeadless with: $Game"
Write-Host "The PPSSPP config must already enable RemoteDebuggerOnStartup=True, RemoteDebuggerLocal=True and RemoteISOPort=$DebuggerPort."
Write-Host "Debugger endpoint expected by capture scripts: ws://127.0.0.1:$DebuggerPort/debugger"

$arguments = @($Game) + $AdditionalArgument
$start = @{
    FilePath = $PpssppHeadless
    ArgumentList = $arguments
    WorkingDirectory = (Split-Path -Parent $PpssppHeadless)
    PassThru = $true
    WindowStyle = 'Hidden'
}
if ($Wait) {
    $start.Wait = $true
}

$process = Start-Process @start
Write-Host "PPSSPPHeadless PID: $($process.Id)"
