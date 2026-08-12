[CmdletBinding()]
param(
    [ValidateSet('down', 'install', 'redo', 'seed', 'skip', 'status', 'up')]
    [string]$Command = 'status'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$workspaceRoot = (Resolve-Path (Join-Path $projectRoot '..')).Path
$toolPath = Join-Path $workspaceRoot (
    'Tools\evedbtool-0.0.6\evedbtool.exe')
$password = [Environment]::GetEnvironmentVariable(
    'EVEMU_DB_PASSWORD',
    'Process')

if ([string]::IsNullOrWhiteSpace($password)) {
    throw 'EVEMU_DB_PASSWORD is not set for this process.'
}
if ($password.Contains("`r") -or $password.Contains("`n")) {
    throw 'EVEMU_DB_PASSWORD contains a line break.'
}
if (-not (Test-Path -LiteralPath $toolPath -PathType Leaf)) {
    throw 'EVEDBTool was not found.'
}

$baseDirectory = (Resolve-Path (Join-Path $projectRoot 'sql\base')).Path
$migrationDirectory = (
    Resolve-Path (Join-Path $projectRoot 'sql\migrations')).Path
$dungeonDirectory = (
    Resolve-Path (Join-Path $projectRoot 'sql\dungeons')).Path
$temporaryDirectory = Join-Path (
    [IO.Path]::GetTempPath()) "evemu-evedbtool-$PID"

if (Test-Path -LiteralPath $temporaryDirectory) {
    throw 'The temporary EVEDBTool directory already exists.'
}

New-Item -ItemType Directory -Path $temporaryDirectory | Out-Null
$configPath = Join-Path $temporaryDirectory 'evedb.yaml'
$config = @"
base-dir: $($baseDirectory.Replace('\', '/'))
db-database: evemu
db-host: 127.0.0.1
db-pass: $password
db-port: "3306"
db-user: evemu
dungeons-dir: $($dungeonDirectory.Replace('\', '/'))
log-level: Info
migrations-dir: $($migrationDirectory.Replace('\', '/'))
"@

$encoding = [System.Text.UTF8Encoding]::new($false)
$exitCode = 0

try {
    [IO.File]::WriteAllText($configPath, $config, $encoding)
    Push-Location $temporaryDirectory
    try {
        $previousErrorAction = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        try {
            $output = & $toolPath $Command 2>&1
        }
        finally {
            $ErrorActionPreference = $previousErrorAction
        }
        $exitCode = $LASTEXITCODE
        $output
    }
    finally {
        Pop-Location
    }
}
finally {
    Remove-Item -LiteralPath $temporaryDirectory -Recurse -Force
}

if ($exitCode -ne 0) {
    exit $exitCode
}
