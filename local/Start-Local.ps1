[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string] $Configuration = 'Debug',
    [ValidateRange(1,60)][int] $ReadinessTimeoutSeconds = 60
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$configurationDir = '{0}-x64' -f $Configuration
$binDir = Join-Path $repoRoot ('bin\{0}' -f $configurationDir)
$clientDir = Join-Path $repoRoot 'assets\Client'
$logDir = Join-Path $PSScriptRoot 'logs'
$statePath = Join-Path $PSScriptRoot 'pids.json'
$envPath = Join-Path $PSScriptRoot '.env.local'
$owned = [System.Collections.Generic.List[object]]::new()

function Write-Utf8File {
    param(
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][AllowEmptyString()][string] $Content
    )

    $encoding = [System.Text.UTF8Encoding]::new($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function New-OwnedState {
    if (Test-Path -LiteralPath $statePath) {
        throw ('Owned process state already exists: {0}. Run Stop-Local.ps1 first.' -f $statePath)
    }

    $stream = [System.IO.FileStream]::new(
        $statePath,
        [System.IO.FileMode]::CreateNew,
        [System.IO.FileAccess]::Write,
        [System.IO.FileShare]::None)
    try {
        $bytes = [System.Text.Encoding]::UTF8.GetBytes('[]')
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush($true)
    } finally {
        $stream.Dispose()
    }
}

function Write-OwnedState {
    $temporaryPath = '{0}.{1}.tmp' -f $statePath, ([Guid]::NewGuid().ToString('N'))
    $backupPath = $statePath + '.bak'
    try {
        $json = ConvertTo-Json -InputObject @($owned.ToArray()) -Depth 3
        Write-Utf8File -Path $temporaryPath -Content $json
        Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
        [System.IO.File]::Replace($temporaryPath, $statePath, $backupPath)
    } finally {
        Remove-Item -LiteralPath $temporaryPath -Force -ErrorAction SilentlyContinue
        Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
    }
}

function Get-GameDatabasePassword {
    if (-not (Test-Path -LiteralPath $envPath -PathType Leaf)) {
        throw ('Missing local secret file: {0}' -f $envPath)
    }

    $matches = @(Get-Content -LiteralPath $envPath | Where-Object { $_ -match '^GAME_DB_PASSWORD=' })
    if ($matches.Count -ne 1) {
        throw 'local/.env.local must contain exactly one GAME_DB_PASSWORD entry.'
    }

    $value = $matches[0].Substring('GAME_DB_PASSWORD='.Length)
    if ([string]::IsNullOrWhiteSpace($value) -or $value.IndexOfAny([char[]]"`r`n") -ge 0) {
        throw 'GAME_DB_PASSWORD is empty or contains a line break.'
    }
    return $value
}

function Write-LocalConfiguration {
    param([Parameter(Mandatory)][string] $GamePassword)

    $aujard = @"
[ODBC]
ACCOUNT_DSN=KN_online
ACCOUNT_UID=knight
ACCOUNT_PWD=$GamePassword
GAME_DSN=KN_online
GAME_UID=knight
GAME_PWD=$GamePassword

[ZONE_INFO]
GROUP_INFO=1
ZONE_INFO=1
"@
    Write-Utf8File -Path (Join-Path $repoRoot 'Aujard.ini') -Content $aujard

    Write-Utf8File -Path (Join-Path $repoRoot 'ItemManager.ini') -Content @"
[LOCAL]
IP=127.0.0.1
"@

    $version = @"
[DOWNLOAD]
URL=127.0.0.1
PATH=/

[ODBC]
DSN=KN_online
UID=knight
PWD=$GamePassword

[SERVER_LIST]
COUNT=1
SERVER_00=127.0.0.1
NAME_00=LOCAL|OpenKO
ID_00=1
USER_LIMIT_00=3000
"@
    Write-Utf8File -Path (Join-Path $repoRoot 'Version.ini') -Content $version

    $aiServer = @"
[SERVER]
ZONE=1

[ODBC]
GAME_DSN=KN_online
GAME_UID=knight
GAME_PWD=$GamePassword

[PATH]
MAP_DIR=MAP
EVENT_DIR=MAP
"@
    Write-Utf8File -Path (Join-Path $repoRoot 'server.ini') -Content $aiServer

    $gameServer = @"
[AI_SERVER]
IP=127.0.0.1

[ODBC]
GAME_DSN=KN_online
GAME_UID=knight
GAME_PWD=$GamePassword

[BOTS]
Enabled=1
Count=10
TickMilliseconds=200
RespawnSeconds=15
Zone=201
AttackRange=2.5
MoveStep=1.5

[PATH]
MAP_DIR=MAP
QUESTS_DIR=QUESTS

[SG_INFO]
SERVER_INDEX=1

[ZONE_INFO]
MY_INFO=1
SERVER_NUM=0
SERVER_COUNT=1
SERVER_00=1
SERVER_IP_00=127.0.0.1
"@
    Write-Utf8File -Path (Join-Path $repoRoot 'gameserver.ini') -Content $gameServer

    $clientTemplate = Join-Path $clientDir 'Server.ini.default'
    $clientConfig = Join-Path $clientDir 'Server.ini'
    if (-not (Test-Path -LiteralPath $clientTemplate -PathType Leaf)) {
        throw ('Missing client configuration template: {0}' -f $clientTemplate)
    }
    if (-not (Test-Path -LiteralPath $clientConfig -PathType Leaf)) {
        Copy-Item -LiteralPath $clientTemplate -Destination $clientConfig
    }
    $clientContent = Get-Content -Raw -LiteralPath $clientConfig
    if ($clientContent -notmatch '(?im)^\s*IP0\s*=') {
        throw 'Client Server.ini does not contain IP0.'
    }
    $clientContent = $clientContent -replace '(?im)^(\s*IP\d+\s*=\s*).+$', '${1}127.0.0.1'
    Write-Utf8File -Path $clientConfig -Content $clientContent
}

function Assert-LoopbackConfiguration {
    param([Parameter(Mandatory)][string[]] $Paths)

    foreach ($path in $Paths) {
        foreach ($line in Get-Content -LiteralPath $path) {
            if ($line -match '^\s*(IP(?:\d+)?|SERVER_IP_\d+|GSERVER_IP_\d+|URL)\s*=\s*(.*?)\s*$') {
                if ($Matches[2] -ne '127.0.0.1') {
                    throw ('Non-loopback address rejected in {0}: key {1}' -f $path, $Matches[1])
                }
            }
            if ((Split-Path -Leaf $path) -eq 'Version.ini' -and $line -match '^\s*(SERVER_\d+)\s*=\s*(.*?)\s*$') {
                if ($Matches[2] -ne '127.0.0.1') {
                    throw ('Non-loopback address rejected in {0}: key {1}' -f $path, $Matches[1])
                }
            }
            if ($line -match '(?i)localhost') {
                throw ('Hostname localhost is not permitted in {0}; use literal 127.0.0.1.' -f $path)
            }
        }
    }
}

function Test-TcpPort {
    param([Parameter(Mandatory)][int] $Port)

    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        $async = $client.BeginConnect('127.0.0.1', $Port, $null, $null)
        if (-not $async.AsyncWaitHandle.WaitOne(250)) { return $false }
        $client.EndConnect($async)
        return $true
    } catch {
        return $false
    } finally {
        $client.Dispose()
    }
}

function Assert-PortFree {
    param([Parameter(Mandatory)][int] $Port)
    if (Test-TcpPort -Port $Port) {
        throw ('Required localhost port {0} is already in use.' -f $Port)
    }
}

function Start-OwnedProcess {
    param(
        [Parameter(Mandatory)][string] $Name,
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][string] $WorkingDirectory
    )

    $stdout = Join-Path $logDir ($Name + '.out.log')
    $stderr = Join-Path $logDir ($Name + '.err.log')
    $process = Start-Process -FilePath $Path -WorkingDirectory $WorkingDirectory -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $process.Refresh()
    if ($process.HasExited) {
        throw ('{0} exited during startup (exit code {1}). Logs: {2}, {3}' -f $Name, $process.ExitCode, $stdout, $stderr)
    }

    $record = [pscustomobject]@{
        Name = $Name
        Id = $process.Id
        Path = [System.IO.Path]::GetFullPath($Path)
        StartedAtUtc = $process.StartTime.ToUniversalTime().ToString('o')
    }
    $owned.Add($record)
    Write-OwnedState
    return $record
}

function Test-OwnedProcessIdentity {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)] $Process
    )

    try {
        if ($Process.ProcessName -ne [string]$Record.Name) { return $false }
        if (-not [string]::Equals(
            [System.IO.Path]::GetFullPath($Process.Path),
            [System.IO.Path]::GetFullPath([string]$Record.Path),
            [System.StringComparison]::OrdinalIgnoreCase)) { return $false }
        $recordedStart = [datetime]::Parse(
            [string]$Record.StartedAtUtc,
            [System.Globalization.CultureInfo]::InvariantCulture,
            [System.Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime()
        return [Math]::Abs(($Process.StartTime.ToUniversalTime() - $recordedStart).TotalSeconds) -le 2
    } catch {
        return $false
    }
}

function Assert-OwnedAlive {
    foreach ($record in $owned) {
        $process = Get-Process -Id $record.Id -ErrorAction SilentlyContinue
        if ($null -eq $process -or -not (Test-OwnedProcessIdentity -Record $record -Process $process)) {
            $stdout = Join-Path $logDir ($record.Name + '.out.log')
            $stderr = Join-Path $logDir ($record.Name + '.err.log')
            throw ('{0} exited or changed identity before readiness. Logs: {1}, {2}' -f $record.Name, $stdout, $stderr)
        }
    }
}

function Wait-LocalReadiness {
    param(
        [Parameter(Mandatory)][datetime] $DeadlineUtc,
        [Parameter(Mandatory)][hashtable] $RequiredPorts
    )

    do {
        Assert-OwnedAlive
        $missing = @($RequiredPorts.GetEnumerator() | Where-Object { -not (Test-TcpPort -Port $_.Value) })
        if ($missing.Count -eq 0) { return }
        Start-Sleep -Milliseconds 250
    } while ([datetime]::UtcNow -lt $DeadlineUtc)

    $summary = $missing | Sort-Object Name | ForEach-Object { '{0}=127.0.0.1:{1}' -f $_.Name, $_.Value }
    throw ('Readiness timed out. Missing: {0}. Inspect logs in {1}.' -f ($summary -join ', '), $logDir)
}

function Remove-OwnedRecord {
    param([Parameter(Mandatory)][int] $Id)
    for ($index = $owned.Count - 1; $index -ge 0; $index--) {
        if ($owned[$index].Id -eq $Id) { $owned.RemoveAt($index) }
    }
}

function Undo-NewProcesses {
    $rollbackFailures = [System.Collections.Generic.List[string]]::new()
    for ($index = $owned.Count - 1; $index -ge 0; $index--) {
        $record = $owned[$index]
        $process = Get-Process -Id $record.Id -ErrorAction SilentlyContinue
        if ($null -ne $process) {
            if (-not (Test-OwnedProcessIdentity -Record $record -Process $process)) {
                $rollbackFailures.Add(('{0} ({1}, identity mismatch)' -f $record.Name, $record.Id))
                continue
            }
            try {
                Stop-Process -Id $record.Id -Force -ErrorAction Stop
                Wait-Process -Id $record.Id -Timeout 10 -ErrorAction SilentlyContinue
            } catch {
                $rollbackFailures.Add(('{0} ({1})' -f $record.Name, $record.Id))
                continue
            }
        }
        Remove-OwnedRecord -Id $record.Id
        Write-OwnedState
    }

    if ($owned.Count -eq 0) {
        Remove-Item -LiteralPath $statePath -Force -ErrorAction SilentlyContinue
    }
    return $rollbackFailures
}

$executables = [ordered]@{
    Aujard = Join-Path $binDir 'Aujard.exe'
    ItemManager = Join-Path $binDir 'ItemManager.exe'
    VersionManager = Join-Path $binDir 'VersionManager.exe'
    AIServer = Join-Path $binDir 'AIServer.exe'
    Ebenezer = Join-Path $binDir 'Ebenezer.exe'
    KnightOnLine = Join-Path $binDir 'KnightOnLine.exe'
}

& (Join-Path $PSScriptRoot 'Test-Prerequisites.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Local prerequisite validation failed.' }

$sql = Get-Service -Name 'MSSQL$SQLEXPRESS' -ErrorAction SilentlyContinue
if ($null -eq $sql -or $sql.Status -ne 'Running') {
    throw 'SQL Server Express service MSSQL$SQLEXPRESS must be Running.'
}

foreach ($entry in $executables.GetEnumerator()) {
    if (-not (Test-Path -LiteralPath $entry.Value -PathType Leaf)) {
        throw ('Required {0} executable is missing: {1}' -f $Configuration, $entry.Value)
    }
}
foreach ($port in @(15100, 10020, 15001)) { Assert-PortFree -Port $port }

$password = Get-GameDatabasePassword
Write-LocalConfiguration -GamePassword $password
Assert-LoopbackConfiguration -Paths @(
    (Join-Path $repoRoot 'Aujard.ini'),
    (Join-Path $repoRoot 'ItemManager.ini'),
    (Join-Path $repoRoot 'Version.ini'),
    (Join-Path $repoRoot 'server.ini'),
    (Join-Path $repoRoot 'gameserver.ini'),
    (Join-Path $clientDir 'Server.ini'))

New-Item -ItemType Directory -Path $logDir -Force | Out-Null
New-OwnedState
$deadlineUtc = [datetime]::UtcNow.AddSeconds($ReadinessTimeoutSeconds)

try {
    [void](Start-OwnedProcess -Name 'Aujard' -Path $executables.Aujard -WorkingDirectory $repoRoot)
    [void](Start-OwnedProcess -Name 'ItemManager' -Path $executables.ItemManager -WorkingDirectory $repoRoot)
    [void](Start-OwnedProcess -Name 'VersionManager' -Path $executables.VersionManager -WorkingDirectory $repoRoot)
    [void](Start-OwnedProcess -Name 'AIServer' -Path $executables.AIServer -WorkingDirectory $repoRoot)
    Wait-LocalReadiness -DeadlineUtc $deadlineUtc -RequiredPorts @{ VersionManager = 15100; AIServer = 10020 }

    [void](Start-OwnedProcess -Name 'Ebenezer' -Path $executables.Ebenezer -WorkingDirectory $repoRoot)
    Wait-LocalReadiness -DeadlineUtc $deadlineUtc -RequiredPorts @{ VersionManager = 15100; AIServer = 10020; Ebenezer = 15001 }

    [void](Start-OwnedProcess -Name 'KnightOnLine' -Path $executables.KnightOnLine -WorkingDirectory $clientDir)
    Assert-OwnedAlive
} catch {
    $startupError = $_.Exception.Message
    $rollbackFailures = Undo-NewProcesses
    if ($rollbackFailures.Count -gt 0) {
        throw ('Startup failed: {0} Rollback incomplete for: {1}. Ownership state preserved at {2}.' -f $startupError, ($rollbackFailures -join ', '), $statePath)
    }
    throw ('Startup failed and newly started processes were rolled back: {0}' -f $startupError)
} finally {
    $password = $null
}

Write-Host ('OpenKO local stack started ({0}-x64). Owned PID state: {1}' -f $Configuration, $statePath)
Write-Host ('Logs: {0}' -f $logDir)
