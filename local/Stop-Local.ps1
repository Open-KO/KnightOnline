[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$statePath = Join-Path $PSScriptRoot 'pids.json'
$allowedNames = @('Aujard','ItemManager','VersionManager','AIServer','Ebenezer','KnightOnLine')

function Write-Utf8File {
    param(
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][AllowEmptyString()][string] $Content
    )

    $encoding = [System.Text.UTF8Encoding]::new($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Write-RemainingState {
    param([Parameter(Mandatory)][AllowEmptyCollection()][object[]] $Records)

    $temporaryPath = '{0}.{1}.tmp' -f $statePath, ([Guid]::NewGuid().ToString('N'))
    $backupPath = $statePath + '.bak'
    try {
        $json = ConvertTo-Json -InputObject @($Records) -Depth 3
        Write-Utf8File -Path $temporaryPath -Content $json
        Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
        [System.IO.File]::Replace($temporaryPath, $statePath, $backupPath)
    } finally {
        Remove-Item -LiteralPath $temporaryPath -Force -ErrorAction SilentlyContinue
        Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
    }
}

function Test-ExactPath {
    param(
        [Parameter(Mandatory)][string] $Left,
        [Parameter(Mandatory)][string] $Right
    )
    return [string]::Equals(
        [System.IO.Path]::GetFullPath($Left),
        [System.IO.Path]::GetFullPath($Right),
        [System.StringComparison]::OrdinalIgnoreCase)
}

function Assert-RecordSchema {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)][AllowEmptyCollection()][System.Collections.Generic.HashSet[int]] $SeenIds
    )

    foreach ($property in @('Name','Id','Path','StartedAtUtc')) {
        if ($null -eq $Record.PSObject.Properties[$property]) {
            throw ('Invalid PID state: missing property {0}.' -f $property)
        }
    }

    if ([string]$Record.Name -notin $allowedNames) {
        throw ('Invalid PID state: unsupported executable name {0}.' -f $Record.Name)
    }

    $id = 0
    if (-not [int]::TryParse([string]$Record.Id, [ref]$id) -or $id -le 0) {
        throw 'Invalid PID state: Id must be a positive integer.'
    }
    if (-not $SeenIds.Add($id)) {
        throw ('Invalid PID state: duplicate process Id {0}.' -f $id)
    }

    $recordedPath = [string]$Record.Path
    if ([string]::IsNullOrWhiteSpace($recordedPath) -or -not [System.IO.Path]::IsPathRooted($recordedPath)) {
        throw 'Invalid PID state: Path must be an absolute path.'
    }
    if ([System.IO.Path]::GetFileNameWithoutExtension($recordedPath) -ne [string]$Record.Name) {
        throw ('Invalid PID state: Name does not match Path for Id {0}.' -f $id)
    }

    $allowedPaths = @(
        (Join-Path $repoRoot ('bin\Debug-x64\{0}.exe' -f $Record.Name)),
        (Join-Path $repoRoot ('bin\Release-x64\{0}.exe' -f $Record.Name)))
    if (-not ($allowedPaths | Where-Object { Test-ExactPath -Left $_ -Right $recordedPath })) {
        throw ('Invalid PID state: Path is outside the supported local build for Id {0}.' -f $id)
    }

    $startedAt = [datetime]::MinValue
    if (-not [datetime]::TryParse(
        [string]$Record.StartedAtUtc,
        [System.Globalization.CultureInfo]::InvariantCulture,
        [System.Globalization.DateTimeStyles]::RoundtripKind,
        [ref]$startedAt)) {
        throw ('Invalid PID state: StartedAtUtc is not a round-trip timestamp for Id {0}.' -f $id)
    }
}

if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) {
    Write-Host 'No owned OpenKO process state was found.'
    exit 0
}

try {
    $rawState = Get-Content -Raw -LiteralPath $statePath
    $records = @($rawState | ConvertFrom-Json -ErrorAction Stop)
} catch {
    throw ('Owned PID state is invalid; no process was stopped and the file was preserved: {0}' -f $_.Exception.Message)
}

$seenIds = [System.Collections.Generic.HashSet[int]]::new()
foreach ($record in $records) { Assert-RecordSchema -Record $record -SeenIds $seenIds }

$remaining = [System.Collections.Generic.List[object]]::new()
foreach ($record in $records) { $remaining.Add($record) }
$failures = [System.Collections.Generic.List[string]]::new()
$stoppedCount = 0
$deadlineUtc = [datetime]::UtcNow.AddSeconds(10)

for ($index = $records.Count - 1; $index -ge 0; $index--) {
    $record = $records[$index]
    $id = [int]$record.Id
    $process = Get-Process -Id $id -ErrorAction SilentlyContinue

    if ($null -eq $process) {
        [void]$remaining.Remove($record)
        Write-RemainingState -Records $remaining.ToArray()
        continue
    }

    try {
        if ($process.ProcessName -ne [string]$record.Name) {
            throw ('process name is {0}, expected {1}' -f $process.ProcessName, $record.Name)
        }

        $actualPath = $process.Path
        if ([string]::IsNullOrWhiteSpace($actualPath) -or -not (Test-ExactPath -Left $actualPath -Right ([string]$record.Path))) {
            throw 'executable path does not match the recorded path'
        }

        $recordedStart = [datetime]::Parse(
            [string]$record.StartedAtUtc,
            [System.Globalization.CultureInfo]::InvariantCulture,
            [System.Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime()
        $actualStart = $process.StartTime.ToUniversalTime()
        if ([Math]::Abs(($actualStart - $recordedStart).TotalSeconds) -gt 2) {
            throw 'process start time does not match the recorded process identity'
        }

        Stop-Process -Id $id -Force -ErrorAction Stop
        $secondsLeft = [Math]::Floor(($deadlineUtc - [datetime]::UtcNow).TotalSeconds)
        if ($secondsLeft -gt 0) {
            Wait-Process -Id $id -Timeout ([int]$secondsLeft) -ErrorAction SilentlyContinue
        }
        if ($null -ne (Get-Process -Id $id -ErrorAction SilentlyContinue)) {
            throw 'process did not exit within the shared ten-second stop deadline'
        }

        [void]$remaining.Remove($record)
        Write-RemainingState -Records $remaining.ToArray()
        $stoppedCount++
    } catch {
        $failures.Add(('{0} ({1}): {2}' -f $record.Name, $id, $_.Exception.Message))
    }
}

if ($remaining.Count -eq 0) {
    Remove-Item -LiteralPath $statePath -Force
}

if ($failures.Count -gt 0) {
    throw ('Some owned processes were not stopped safely; remaining ownership state was preserved. {0}' -f ($failures -join '; '))
}

Write-Host ('Stopped {0} owned OpenKO process(es).' -f $stoppedCount)
