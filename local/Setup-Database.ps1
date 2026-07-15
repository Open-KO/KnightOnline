[CmdletBinding()]
param([Parameter(Mandatory)][string] $GameDbPassword)

$ErrorActionPreference = 'Stop'

function ConvertTo-YamlDoubleQuotedScalar {
    param([Parameter(Mandatory)][AllowEmptyString()][string] $Value)

    if ($Value -match '[\x00-\x1F\x7F]') {
        throw 'GameDbPassword must not contain control characters.'
    }

    return '"' + $Value.Replace('\', '\\').Replace('"', '\"') + '"'
}

function Get-GitBashPath {
    $candidates = [System.Collections.Generic.List[string]]::new()
    if ($env:ProgramFiles) { $candidates.Add((Join-Path $env:ProgramFiles 'Git\bin\bash.exe')) }
    if (${env:ProgramFiles(x86)}) { $candidates.Add((Join-Path ${env:ProgramFiles(x86)} 'Git\bin\bash.exe')) }
    if ($env:LOCALAPPDATA) { $candidates.Add((Join-Path $env:LOCALAPPDATA 'Programs\Git\bin\bash.exe')) }

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }

    throw 'Git Bash was not found in a standard Git for Windows installation.'
}

function Invoke-GitBashSubmodule {
    param(
        [Parameter(Mandatory)][string] $RepositoryPath,
        [Parameter(Mandatory)][ValidateNotNullOrEmpty()][string[]] $Arguments
    )

    $gitBash = Get-GitBashPath
    $bashCommand = 'repo_path=$(cygpath -u -- "$1") || exit $?; shift; git -C "$repo_path" submodule "$@"'
    & $gitBash -lc $bashCommand -- $RepositoryPath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw ('Git Bash git submodule {0} failed with exit code {1}.' -f ($Arguments -join ' '), $LASTEXITCODE)
    }
}

$runtime = Join-Path $PSScriptRoot 'runtime'
$util = Join-Path $runtime 'kodb-util'
New-Item -ItemType Directory -Force -Path $runtime | Out-Null
if (-not (Test-Path -LiteralPath (Join-Path $util '.git'))) {
    git clone https://github.com/Open-KO/kodb-util.git $util
    if ($LASTEXITCODE -ne 0) { throw 'git clone failed while fetching Open-KO/kodb-util.' }
}
Invoke-GitBashSubmodule -RepositoryPath $util -Arguments @('update','--init','--recursive')

$configPath = Join-Path $util 'kodb-util-config.yaml'
$gameDbPasswordYaml = ConvertTo-YamlDoubleQuotedScalar $GameDbPassword
$yaml = @"
databaseConfig:
  host: localhost
  instance: SQLEXPRESS
  port: 1433
genConfig:
  schemaDir: ./OpenKO-db
  gameDb:
    - name: KN_online
      isForbidClean: false
      isForbidImport: false
      isForbidExport: true
      schemas:
        - knight
      logins:
        - name: knight
          pass: $gameDbPasswordYaml
      users:
        - name: knight
          schema: knight
"@
Set-Content -LiteralPath $configPath -Value $yaml -Encoding UTF8

Push-Location $util
try {
    go mod download
    if ($LASTEXITCODE -ne 0) { throw 'go mod download failed for kodb-util.' }
    go run kodb-util.go -clean -import
    if ($LASTEXITCODE -ne 0) { throw 'kodb-util database clean/import failed.' }
} finally {
    Pop-Location
}

Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue | Remove-OdbcDsn
Add-OdbcDsn -Name 'KN_online' -DriverName 'ODBC Driver 18 for SQL Server' -DsnType User -Platform '64-bit' -SetPropertyValue @('Server=.\SQLEXPRESS','Database=KN_online','Trusted_Connection=No','Encrypt=Optional','AutoTranslate=No')
Write-Host 'KN_online import and 64-bit DSN setup: OK'
