[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()
$repoRoot = Split-Path -Parent $PSScriptRoot
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere)) {
    $failures.Add('Visual Studio Installer/vswhere is missing.')
} else {
    $msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) { $failures.Add('MSBuild 17.x is missing.') }
    foreach ($component in @('Microsoft.VisualStudio.Workload.VCTools','Microsoft.VisualStudio.Component.VC.ATL','Microsoft.VisualStudio.Component.VC.ATLMFC')) {
        $match = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires $component -property installationPath
        if (-not $match) { $failures.Add(('Visual Studio component missing: {0}' -f $component)) }
    }
}

if (-not (Get-Command go -ErrorAction SilentlyContinue)) {
    $failures.Add('Go 1.24+ is missing.')
} elseif ((go version) -notmatch 'go1\.(2[4-9]|[3-9][0-9])') {
    $failures.Add(('Go is too old: {0}' -f (go version)))
}

$sql = Get-Service -Name 'MSSQL$SQLEXPRESS' -ErrorAction SilentlyContinue
if ($null -eq $sql) { $failures.Add('SQL Server Express instance SQLEXPRESS is missing.') }

$dsn = Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue
if ($null -eq $dsn) { $failures.Add('64-bit user DSN KN_online is missing.') }

foreach ($relative in @('assets\Client\Server.ini.default','deps\googletest\CMakeLists.txt','deps\db-models\CMakeLists.txt')) {
    if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $relative))) {
        $failures.Add(('Submodule content missing: {0}' -f $relative))
    }
}

if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Error $_ -ErrorAction Continue }
    exit 1
}

Write-Host 'OpenKO prerequisites: OK'
exit 0
