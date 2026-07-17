[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string] $Configuration = 'Debug',
    [ValidateRange(1,60)][int] $ReadinessTimeoutSeconds = 60,
    [switch] $LibraryOnly
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$configurationDir = '{0}-x64' -f $Configuration
$binDir = Join-Path $repoRoot ('bin\{0}' -f $configurationDir)
$clientDir = Join-Path $repoRoot 'assets\Client'
$mapDir = Join-Path $repoRoot 'assets\Server\MAP'
$questsDir = Join-Path $repoRoot 'assets\Server\QUESTS'
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
        if (Test-Path -LiteralPath $statePath) {
            Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
        }
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
    if ([string]::IsNullOrEmpty($value) -or $value -cne $value.Trim()) {
        throw 'GAME_DB_PASSWORD must be non-empty and have no leading or trailing whitespace.'
    }
    foreach ($character in $value.ToCharArray()) {
        if ([char]::IsControl($character)) {
            throw 'GAME_DB_PASSWORD contains a control character.'
        }
    }
    return $value
}

function Assert-AssetDirectories {
    foreach ($asset in @($clientDir, $mapDir, $questsDir)) {
        if (-not (Test-Path -LiteralPath $asset -PathType Container)) {
            throw ('Required asset directory is missing: {0}' -f $asset)
        }
    }
}

function Write-LocalConfiguration {
    param([Parameter(Mandatory)][string] $GamePassword)

    Assert-AssetDirectories
    $absoluteMapDir = [System.IO.Path]::GetFullPath($mapDir)
    $absoluteQuestsDir = [System.IO.Path]::GetFullPath($questsDir)

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
[NETWORK]
LISTEN_IP=127.0.0.1

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
[NETWORK]
LISTEN_IP=127.0.0.1

[SERVER]
ZONE=1

[ODBC]
GAME_DSN=KN_online
GAME_UID=knight
GAME_PWD=$GamePassword

[PATH]
MAP_DIR=$absoluteMapDir
EVENT_DIR=$absoluteMapDir
"@
    Write-Utf8File -Path (Join-Path $repoRoot 'server.ini') -Content $aiServer

    $gameServer = @"
[NETWORK]
LISTEN_IP=127.0.0.1

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
MAP_DIR=$absoluteMapDir
QUESTS_DIR=$absoluteQuestsDir

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
            if ($line -cnotmatch '^\s*([A-Za-z][A-Za-z0-9_]*)\s*=\s*(.*?)\s*$') { continue }
            $key = $Matches[1]
            $value = $Matches[2]
            $normalizedKey = $key.ToUpperInvariant()
            $isAddressKey = $normalizedKey -cmatch '^(IP\d*|LISTEN_IP|SERVER_IP_\d+|GSERVER_IP_\d+|URL)$' -or
                ((Split-Path -Leaf $path) -eq 'Version.ini' -and $normalizedKey -cmatch '^SERVER_\d+$')
            if ($isAddressKey -and $value -cne '127.0.0.1') {
                throw ('Non-loopback address rejected in {0}: key {1}' -f $path, $key)
            }
        }
    }
}

function Test-TcpPort {
    param(
        [Parameter(Mandatory)][int] $Port,
        [ValidateRange(1,1000)][int] $TimeoutMilliseconds = 250
    )

    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        $async = $client.BeginConnect('127.0.0.1', $Port, $null, $null)
        if (-not $async.AsyncWaitHandle.WaitOne($TimeoutMilliseconds)) { return $false }
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

function Wait-OwnedProcessIdentity {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)] $Process,
        [ValidateRange(1,2000)][int] $TimeoutMilliseconds = 2000,
        [ValidateRange(1,50)][int] $PollMilliseconds = 50
    )

    $deadlineUtc = [datetime]::UtcNow.AddMilliseconds($TimeoutMilliseconds)
    while ($true) {
        if (Test-OwnedProcessIdentity -Record $Record -Process $Process) { return $true }
        try {
            $Process.Refresh()
            if ($Process.HasExited) { return $false }
        } catch {
            # Process metadata may be temporarily unavailable immediately after Start-Process.
        }

        $remainingMilliseconds = [Math]::Floor(($deadlineUtc - [datetime]::UtcNow).TotalMilliseconds)
        if ($remainingMilliseconds -le 0) { return $false }
        [System.Threading.Thread]::Sleep([int][Math]::Min($PollMilliseconds, $remainingMilliseconds))
    }
}

function Start-OwnedProcess {
    param(
        [Parameter(Mandatory)][string] $Name,
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][string] $WorkingDirectory,
        [ValidateSet('Hidden','Normal')][string] $WindowStyle = 'Hidden'
    )

    $stdout = Join-Path $logDir ($Name + '.out.log')
    $stderr = Join-Path $logDir ($Name + '.err.log')
    $process = Start-Process -FilePath $Path -WorkingDirectory $WorkingDirectory -WindowStyle $WindowStyle -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $record = [pscustomobject]@{
        Name = $Name
        Id = [int]$process.Id
        Path = [System.IO.Path]::GetFullPath($Path)
        StartTimeUtcTicks = [int64]$process.StartTime.ToUniversalTime().Ticks
    }
    $owned.Add($record)
    Write-OwnedState
    if (-not (Wait-OwnedProcessIdentity -Record $record -Process $process)) {
        $cleanupFailure = $null
        try {
            $process.Refresh()
            if (-not $process.HasExited) {
                Stop-Process -InputObject $process -Force -ErrorAction Stop
                if (-not $process.WaitForExit(2000)) {
                    throw 'captured process did not exit before cleanup deadline'
                }
                $process.Refresh()
                if (-not $process.HasExited) { throw 'captured process remained alive after cleanup' }
            }
        } catch {
            $cleanupError = $_.Exception.Message
            try {
                $process.Refresh()
                if (-not $process.HasExited) { $cleanupFailure = $cleanupError }
            } catch {
                $cleanupFailure = $_.Exception.Message
            }
        }

        if ($null -eq $cleanupFailure) {
            Remove-OwnedRecord -Id $record.Id
            Write-OwnedState
        } else {
            throw ('{0} exited or changed identity during startup, and captured process cleanup failed: {1}. Ownership state preserved at {2}. Logs: {3}, {4}' -f
                $Name, $cleanupFailure, $statePath, $stdout, $stderr)
        }
        throw ('{0} exited or changed identity during startup. Logs: {1}, {2}' -f $Name, $stdout, $stderr)
    }
    return $record
}

function Test-OwnedProcessIdentity {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)] $Process
    )

    try {
        $Process.Refresh()
        if ($Process.HasExited) { return $false }
        if ($Process.ProcessName -cne [string]$Record.Name) { return $false }
        if (-not [string]::Equals(
            [System.IO.Path]::GetFullPath($Process.Path),
            [System.IO.Path]::GetFullPath([string]$Record.Path),
            [System.StringComparison]::OrdinalIgnoreCase)) { return $false }
        return [int64]($Process.StartTime.ToUniversalTime().Ticks) -eq
            [int64]($Record.StartTimeUtcTicks)
    } catch {
        return $false
    }
}

function Get-ValidatedOwnedProcess {
    param([Parameter(Mandatory)] $Record)
    $process = Get-Process -Id ([int]$Record.Id) -ErrorAction SilentlyContinue
    if ($null -eq $process -or -not (Test-OwnedProcessIdentity -Record $Record -Process $process)) {
        return $null
    }
    $process.Refresh()
    if (-not (Test-OwnedProcessIdentity -Record $Record -Process $process)) { return $null }
    return $process
}

function Test-ReadyMarker {
    param([Parameter(Mandatory)] $Record, [Parameter(Mandatory)][string] $Marker)
    $stdout = Join-Path $logDir ([string]$Record.Name + '.out.log')
    return (Test-Path -LiteralPath $stdout -PathType Leaf) -and
        [bool](Select-String -LiteralPath $stdout -SimpleMatch $Marker -Quiet -ErrorAction SilentlyContinue)
}

function Test-ServiceReady {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)][int] $Port,
        [Parameter(Mandatory)][string] $Marker,
        [Parameter(Mandatory)][int] $TimeoutMilliseconds
    )
    return $null -ne (Get-ValidatedOwnedProcess -Record $Record) -and
        (Test-TcpPort -Port $Port -TimeoutMilliseconds $TimeoutMilliseconds) -and
        (Test-ReadyMarker -Record $Record -Marker $Marker)
}

function Wait-OwnedServicesReady {
    param(
        [Parameter(Mandatory)][datetime] $DeadlineUtc,
        [Parameter(Mandatory)][object[]] $Services
    )

    while ($true) {
        $missing = [System.Collections.Generic.List[string]]::new()
        foreach ($service in $Services) {
            $remainingMilliseconds = [Math]::Floor(($DeadlineUtc - [datetime]::UtcNow).TotalMilliseconds)
            if ($remainingMilliseconds -le 0) {
                throw ('Readiness timed out before every requested service was checked. Inspect logs in {0}.' -f $logDir)
            }
            $attemptMilliseconds = [int][Math]::Max(1, [Math]::Min(250, $remainingMilliseconds))
            $ready = Test-ServiceReady -Record $service.Record -Port $service.Port `
                -Marker $service.Marker -TimeoutMilliseconds $attemptMilliseconds
            if (-not $ready) {
                if ($null -eq (Get-ValidatedOwnedProcess -Record $service.Record)) {
                    throw ('{0} exited or changed identity before readiness. Inspect logs in {1}.' -f
                        $service.Record.Name, $logDir)
                }
                $missing.Add(('{0}=127.0.0.1:{1} marker={2}' -f
                    $service.Record.Name, $service.Port, $service.Marker))
            }
        }
        if ($missing.Count -eq 0) { return $true }
        $remainingMilliseconds = [Math]::Floor(($DeadlineUtc - [datetime]::UtcNow).TotalMilliseconds)
        if ($remainingMilliseconds -le 0) {
            throw ('Readiness timed out. Missing: {0}. Inspect logs in {1}.' -f
                ($missing -join ', '), $logDir)
        }
        [System.Threading.Thread]::Sleep([int][Math]::Min(250, $remainingMilliseconds))
    }
}

function Remove-OwnedRecord {
    param([Parameter(Mandatory)][int] $Id)
    for ($index = $owned.Count - 1; $index -ge 0; $index--) {
        if ($owned[$index].Id -eq $Id) { $owned.RemoveAt($index) }
    }
}

function Undo-NewProcesses {
    param([ValidateRange(1,10000)][int] $TimeoutMilliseconds = 10000)
    $rollbackFailures = [System.Collections.Generic.List[string]]::new()
    $deadlineUtc = [datetime]::UtcNow.AddMilliseconds($TimeoutMilliseconds)
    for ($index = $owned.Count - 1; $index -ge 0; $index--) {
        $record = $owned[$index]
        $existing = Get-Process -Id ([int]$record.Id) -ErrorAction SilentlyContinue
        if ($null -eq $existing) {
            Remove-OwnedRecord -Id $record.Id
            Write-OwnedState
            continue
        }
        $process = Get-ValidatedOwnedProcess -Record $record
        if ($null -eq $process) {
            $rollbackFailures.Add(('{0} ({1}, identity mismatch)' -f $record.Name, $record.Id))
            continue
        }
        try {
            $process.Refresh()
            if (-not (Test-OwnedProcessIdentity -Record $record -Process $process)) {
                throw 'identity changed before rollback stop'
            }
            Stop-Process -InputObject $process -Force -ErrorAction Stop
            $remainingMilliseconds = [int][Math]::Max(0,
                [Math]::Floor(($deadlineUtc - [datetime]::UtcNow).TotalMilliseconds))
            [void]$process.WaitForExit($remainingMilliseconds)
            $process.Refresh()
            if (-not $process.HasExited) { throw 'captured process did not exit before rollback deadline' }
            Remove-OwnedRecord -Id $record.Id
            Write-OwnedState
        } catch {
            $rollbackFailures.Add(('{0} ({1}): {2}' -f $record.Name, $record.Id, $_.Exception.Message))
        }
    }

    if ($owned.Count -eq 0) {
        Remove-Item -LiteralPath $statePath -Force -ErrorAction SilentlyContinue
    }
    return $rollbackFailures
}

function Assert-NoOwnedState {
    if (Test-Path -LiteralPath $statePath) {
        throw ('Owned or stale process state already exists: {0}. Run Stop-Local.ps1 first.' -f $statePath)
    }
}

function Assert-LocalPrerequisites {
    & (Join-Path $PSScriptRoot 'Test-Prerequisites.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Local prerequisite validation failed.' }
}

function Assert-SqlRunning {
    $sql = Get-Service -Name 'MSSQL$SQLEXPRESS' -ErrorAction SilentlyContinue
    if ($null -eq $sql -or $sql.Status -ne 'Running') {
        throw 'SQL Server Express service MSSQL$SQLEXPRESS must be Running.'
    }
}

function Invoke-StartLocal {
    Assert-NoOwnedState
    Assert-LocalPrerequisites
    Assert-SqlRunning
    Assert-AssetDirectories

    $executables = [ordered]@{
        Aujard = Join-Path $binDir 'Aujard.exe'
        ItemManager = Join-Path $binDir 'ItemManager.exe'
        VersionManager = Join-Path $binDir 'VersionManager.exe'
        AIServer = Join-Path $binDir 'AIServer.exe'
        Ebenezer = Join-Path $binDir 'Ebenezer.exe'
        KnightOnLine = Join-Path $binDir 'KnightOnLine.exe'
    }
    foreach ($entry in $executables.GetEnumerator()) {
        if (-not (Test-Path -LiteralPath $entry.Value -PathType Leaf)) {
            throw ('Required {0} executable is missing: {1}' -f $Configuration, $entry.Value)
        }
    }
    foreach ($port in @(15100, 10020, 15001)) { Assert-PortFree -Port $port }

    $password = Get-GameDatabasePassword
    try { Write-LocalConfiguration -GamePassword $password } finally { $password = $null }
    Assert-LoopbackConfiguration -Paths @(
        (Join-Path $repoRoot 'Aujard.ini'), (Join-Path $repoRoot 'ItemManager.ini'),
        (Join-Path $repoRoot 'Version.ini'), (Join-Path $repoRoot 'server.ini'),
        (Join-Path $repoRoot 'gameserver.ini'), (Join-Path $clientDir 'Server.ini'))

    New-Item -ItemType Directory -Path $logDir -Force | Out-Null
    New-OwnedState
    $deadlineUtc = [datetime]::UtcNow.AddSeconds($ReadinessTimeoutSeconds)
    try {
        [void](Start-OwnedProcess -Name 'Aujard' -Path $executables.Aujard -WorkingDirectory $repoRoot)
        [void](Start-OwnedProcess -Name 'ItemManager' -Path $executables.ItemManager -WorkingDirectory $repoRoot)
        $version = Start-OwnedProcess -Name 'VersionManager' -Path $executables.VersionManager -WorkingDirectory $repoRoot
        $ai = Start-OwnedProcess -Name 'AIServer' -Path $executables.AIServer -WorkingDirectory $repoRoot
        [void](Wait-OwnedServicesReady -DeadlineUtc $deadlineUtc -Services @(
            [pscustomobject]@{ Record=$version; Port=15100; Marker='OPENKO_READY VersionManager 127.0.0.1:15100' },
            [pscustomobject]@{ Record=$ai; Port=10020; Marker='OPENKO_READY AIServer 127.0.0.1:10020' }))

        $ebenezer = Start-OwnedProcess -Name 'Ebenezer' -Path $executables.Ebenezer -WorkingDirectory $repoRoot
        [void](Wait-OwnedServicesReady -DeadlineUtc $deadlineUtc -Services @(
            [pscustomobject]@{ Record=$version; Port=15100; Marker='OPENKO_READY VersionManager 127.0.0.1:15100' },
            [pscustomobject]@{ Record=$ai; Port=10020; Marker='OPENKO_READY AIServer 127.0.0.1:10020' },
            [pscustomobject]@{ Record=$ebenezer; Port=15001; Marker='OPENKO_READY Ebenezer 127.0.0.1:15001' }))

        [void](Start-OwnedProcess -Name 'KnightOnLine' -Path $executables.KnightOnLine -WorkingDirectory $clientDir -WindowStyle Normal)
    } catch {
        $startupError = $_.Exception.Message
        $rollbackFailures = @(Undo-NewProcesses)
        if ($rollbackFailures.Count -gt 0) {
            throw ('Startup failed: {0} Rollback incomplete: {1}. State preserved at {2}.' -f
                $startupError, ($rollbackFailures -join '; '), $statePath)
        }
        throw ('Startup failed and newly started processes were rolled back: {0}' -f $startupError)
    }

    Write-Host ('OpenKO local stack started ({0}-x64). Owned PID state: {1}' -f $Configuration, $statePath)
    Write-Host ('Logs: {0}' -f $logDir)
}

if (-not $LibraryOnly) { Invoke-StartLocal }
