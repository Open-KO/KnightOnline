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

$runtime = Join-Path $PSScriptRoot 'runtime'
$util = Join-Path $runtime 'kodb-util'
New-Item -ItemType Directory -Force -Path $runtime | Out-Null
if (-not (Test-Path -LiteralPath (Join-Path $util '.git'))) {
    git clone https://github.com/Open-KO/kodb-util.git $util
    if ($LASTEXITCODE -ne 0) { throw 'git clone failed while fetching Open-KO/kodb-util.' }
}
git -C $util submodule update --init --recursive
if ($LASTEXITCODE -ne 0) { throw 'git submodule update failed for kodb-util.' }

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
