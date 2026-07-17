[CmdletBinding()]
param([switch] $LibraryOnly)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$statePath = Join-Path $PSScriptRoot 'pids.json'
$allowedNames = @('Aujard','ItemManager','VersionManager','AIServer','Ebenezer','KnightOnLine')

function Write-Utf8File {
    param([Parameter(Mandatory)][string] $Path, [Parameter(Mandatory)][AllowEmptyString()][string] $Content)
    [System.IO.File]::WriteAllText($Path, $Content, [System.Text.UTF8Encoding]::new($false))
}

function Write-RemainingState {
    param([Parameter(Mandatory)][AllowEmptyCollection()][object[]] $Records)
    $temporaryPath = '{0}.{1}.tmp' -f $statePath, ([Guid]::NewGuid().ToString('N'))
    $backupPath = $statePath + '.bak'
    try {
        Write-Utf8File $temporaryPath (ConvertTo-Json -InputObject @($Records) -Depth 3)
        Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
        [System.IO.File]::Replace($temporaryPath, $statePath, $backupPath)
    } finally {
        Remove-Item -LiteralPath $temporaryPath -Force -ErrorAction SilentlyContinue
        if (Test-Path -LiteralPath $statePath) {
            Remove-Item -LiteralPath $backupPath -Force -ErrorAction SilentlyContinue
        }
    }
}

function Test-ExactPath {
    param([Parameter(Mandatory)][string] $Left, [Parameter(Mandatory)][string] $Right)
    [string]::Equals([System.IO.Path]::GetFullPath($Left), [System.IO.Path]::GetFullPath($Right),
        [System.StringComparison]::OrdinalIgnoreCase)
}

function Assert-RecordSchema {
    param(
        [Parameter(Mandatory)] $Record,
        [Parameter(Mandatory)][AllowEmptyCollection()][System.Collections.Generic.HashSet[int]] $SeenIds,
        [Parameter(Mandatory)][AllowEmptyCollection()][System.Collections.Generic.HashSet[string]] $SeenNames,
        [Parameter(Mandatory)][AllowEmptyCollection()][System.Collections.Generic.HashSet[string]] $SeenPaths
    )
    if ($Record -is [System.Array] -or $Record -isnot [pscustomobject]) {
        throw 'Invalid PID state: every top-level array member must be one object.'
    }
    foreach ($property in @('Name','Id','Path','StartTimeUtcTicks')) {
        if ($null -eq $Record.PSObject.Properties[$property]) {
            throw ('Invalid PID state: missing property {0}.' -f $property)
        }
    }
    $name = [string]$Record.Name
    if ($name -notin $allowedNames) { throw ('Invalid PID state: unsupported executable name {0}.' -f $name) }
    if (-not $SeenNames.Add($name)) { throw ('Invalid PID state: duplicate executable name {0}.' -f $name) }

    $id = 0
    if (-not [int]::TryParse([string]$Record.Id, [ref]$id) -or $id -le 0) {
        throw 'Invalid PID state: Id must be a positive integer.'
    }
    if (-not $SeenIds.Add($id)) { throw ('Invalid PID state: duplicate process Id {0}.' -f $id) }

    $ticks = [int64]0
    if (-not [int64]::TryParse([string]$Record.StartTimeUtcTicks, [ref]$ticks) -or $ticks -le 0) {
        throw ('Invalid PID state: StartTimeUtcTicks must be a positive Int64 for Id {0}.' -f $id)
    }

    $recordedPath = [string]$Record.Path
    if ([string]::IsNullOrWhiteSpace($recordedPath) -or -not [System.IO.Path]::IsPathRooted($recordedPath)) {
        throw 'Invalid PID state: Path must be an absolute path.'
    }
    $recordedPath = [System.IO.Path]::GetFullPath($recordedPath)
    if ([System.IO.Path]::GetFileNameWithoutExtension($recordedPath) -cne $name) {
        throw ('Invalid PID state: Name does not match Path for Id {0}.' -f $id)
    }
    if (-not $SeenPaths.Add($recordedPath)) { throw ('Invalid PID state: duplicate Path {0}.' -f $recordedPath) }
    $allowedPaths = @(
        (Join-Path $repoRoot ('bin\Debug-x64\{0}.exe' -f $name)),
        (Join-Path $repoRoot ('bin\Release-x64\{0}.exe' -f $name)))
    if (-not ($allowedPaths | Where-Object { Test-ExactPath $_ $recordedPath })) {
        throw ('Invalid PID state: Path is outside the supported local build for Id {0}.' -f $id)
    }
}

function Read-OwnedState {
    if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) { return @() }
    $raw = Get-Content -Raw -LiteralPath $statePath
    $trimmed = $raw.Trim()
    if (-not $trimmed.StartsWith('[') -or -not $trimmed.EndsWith(']')) {
        throw 'Owned PID state must be an explicit top-level JSON array.'
    }
    # Windows PowerShell 5.1 enumerates a top-level JSON array on the pipeline. The raw
    # bracket check above preserves the top-level shape contract; this pipeline form then
    # intentionally flattens exactly that one level while nested arrays remain Object[].
    try {
        $parsed = $raw | ConvertFrom-Json -ErrorAction Stop
        $records = @($parsed)
    }
    catch { throw ('Owned PID state JSON is invalid: {0}' -f $_.Exception.Message) }

    $seenIds = [System.Collections.Generic.HashSet[int]]::new()
    $seenNames = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    $seenPaths = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($record in $records) {
        Assert-RecordSchema $record $seenIds $seenNames $seenPaths
    }
    return $records
}

function Test-OwnedProcessIdentity {
    param([Parameter(Mandatory)] $Record, [Parameter(Mandatory)] $Process)
    try {
        $Process.Refresh()
        if ($Process.HasExited -or $Process.ProcessName -cne [string]$Record.Name) { return $false }
        if (-not (Test-ExactPath $Process.Path ([string]$Record.Path))) { return $false }
        return [int64]($Process.StartTime.ToUniversalTime().Ticks) -eq
            [int64]($Record.StartTimeUtcTicks)
    } catch { return $false }
}

function Get-ValidatedOwnedProcess {
    param([Parameter(Mandatory)] $Record)
    $process = Get-Process -Id ([int]$Record.Id) -ErrorAction SilentlyContinue
    if ($null -eq $process -or -not (Test-OwnedProcessIdentity $Record $process)) { return $null }
    $process.Refresh()
    if (-not (Test-OwnedProcessIdentity $Record $process)) { return $null }
    return $process
}

function Invoke-StopLocal {
    param([ValidateRange(1,10000)][int] $TimeoutMilliseconds = 10000)
    if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) {
        Write-Host 'No owned OpenKO process state was found.'
        return
    }
    try { $records = @(Read-OwnedState) }
    catch { throw ('Owned PID state is invalid; no process was stopped and the file was preserved: {0}' -f $_.Exception.Message) }

    $remaining = [System.Collections.Generic.List[object]]::new()
    foreach ($record in $records) { $remaining.Add($record) }
    $failures = [System.Collections.Generic.List[string]]::new()
    $stoppedCount = 0
    $deadlineUtc = [datetime]::UtcNow.AddMilliseconds($TimeoutMilliseconds)

    for ($index = $records.Count - 1; $index -ge 0; $index--) {
        $record = $records[$index]
        $existing = Get-Process -Id ([int]$record.Id) -ErrorAction SilentlyContinue
        if ($null -eq $existing) {
            [void]$remaining.Remove($record)
            Write-RemainingState $remaining.ToArray()
            continue
        }
        $process = Get-ValidatedOwnedProcess $record
        if ($null -eq $process) {
            $failures.Add(('{0} ({1}): exact process identity mismatch' -f $record.Name, $record.Id))
            continue
        }
        try {
            $process.Refresh()
            if (-not (Test-OwnedProcessIdentity $record $process)) { throw 'identity changed immediately before stop' }
            Stop-Process -InputObject $process -Force -ErrorAction Stop
            $remainingMilliseconds = [int][Math]::Max(0,
                [Math]::Floor(($deadlineUtc - [datetime]::UtcNow).TotalMilliseconds))
            [void]$process.WaitForExit($remainingMilliseconds)
            $process.Refresh()
            if (-not $process.HasExited) { throw 'captured process did not exit within ten seconds' }
            [void]$remaining.Remove($record)
            Write-RemainingState $remaining.ToArray()
            $stoppedCount++
        } catch { $failures.Add(('{0} ({1}): {2}' -f $record.Name, $record.Id, $_.Exception.Message)) }
    }

    if ($remaining.Count -eq 0) { Remove-Item -LiteralPath $statePath -Force }
    if ($failures.Count -gt 0) {
        throw ('Some owned processes were not stopped safely; remaining state was preserved. {0}' -f
            ($failures -join '; '))
    }
    Write-Host ('Stopped {0} owned OpenKO process(es).' -f $stoppedCount)
}

if (-not $LibraryOnly) { Invoke-StopLocal }
