[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()
$repoRoot = Split-Path -Parent $PSScriptRoot
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
. (Join-Path $PSScriptRoot 'Local-Tooling.ps1')

function Get-OdbcDsnPropertyValue {
    param(
        [Parameter(Mandatory)] $Dsn,
        [Parameter(Mandatory)][string] $Name
    )

    $candidates = [System.Collections.Generic.List[object]]::new()
    foreach ($propertyName in @('Attribute','PropertyValue')) {
        $property = $Dsn.PSObject.Properties[$propertyName]
        if ($null -eq $property) { continue }
        foreach ($value in @($property.Value)) { $candidates.Add($value) }
    }

    foreach ($attribute in $candidates) {
        if ($null -eq $attribute) { continue }

        $keywordProperty = $attribute.PSObject.Properties['Keyword']
        $valueProperty = $attribute.PSObject.Properties['Value']
        if ($null -ne $keywordProperty -and $null -ne $valueProperty -and [string]$keywordProperty.Value -ieq $Name) {
            return [string]$valueProperty.Value
        }

        $text = [string]$attribute
        if ($text -match '^\s*([^=]+)=(.*)$' -and $Matches[1].Trim() -ieq $Name) {
            return $Matches[2].Trim()
        }
    }

    return $null
}

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

$goCommand = Resolve-GoExecutable
if ($null -eq $goCommand) {
    $failures.Add('Go 1.24+ is missing.')
} else {
    $goVersion = & $goCommand version
    if ($LASTEXITCODE -ne 0) {
        $failures.Add('Go version check failed.')
    } elseif ($goVersion -notmatch 'go1\.(2[4-9]|[3-9][0-9])') {
        $failures.Add(('Go is too old: {0}' -f $goVersion))
    }
}

$sql = Get-Service -Name 'MSSQL$SQLEXPRESS' -ErrorAction SilentlyContinue
if ($null -eq $sql) {
    $failures.Add('SQL Server Express instance SQLEXPRESS is missing.')
} elseif ($sql.Status -ne 'Running') {
    $failures.Add('SQL Server Express instance SQLEXPRESS is not running.')
}

$dsn = Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue
if ($null -eq $dsn) {
    $failures.Add('64-bit user DSN KN_online is missing.')
} else {
    if ($dsn.DriverName -ne 'ODBC Driver 18 for SQL Server') {
        $failures.Add('KN_online DSN must use ODBC Driver 18 for SQL Server.')
    }
    if ((Get-OdbcDsnPropertyValue -Dsn $dsn -Name 'AutoTranslate') -ne 'No') {
        $failures.Add('KN_online DSN must set AutoTranslate=No.')
    }
    if ((Get-OdbcDsnPropertyValue -Dsn $dsn -Name 'Database') -ne 'KN_online') {
        $failures.Add('KN_online DSN must set Database=KN_online.')
    }
    if ((Get-OdbcDsnPropertyValue -Dsn $dsn -Name 'Server') -ne '.\SQLEXPRESS') {
        $failures.Add('KN_online DSN must set Server=.\SQLEXPRESS.')
    }
}

foreach ($relative in @('assets\Client\Server.ini.default','deps\googletest\CMakeLists.txt','deps\db-models\Ebenezer\model\EbenezerModel.h')) {
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
