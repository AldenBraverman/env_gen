# Open env_gen_test.RPP (Reaper project) with the default application.
# Usage: .\open_env_gen_test.ps1

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectFile = Join-Path $ScriptDir "env_gen_test.RPP"

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    Write-Error "Project file not found: $ProjectFile"
    exit 1
}

Start-Process -FilePath $ProjectFile
