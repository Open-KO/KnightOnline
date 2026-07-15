[CmdletBinding()]
param([Parameter(Mandatory)][string] $GameDbPassword)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Local-Tooling.ps1')

function ConvertTo-YamlDoubleQuotedScalar {
    param([Parameter(Mandatory)][AllowEmptyString()][string] $Value)

    if ($Value -match '[\x00-\x1F\x7F]') {
        throw 'GameDbPassword must not contain control characters.'
    }

    return '"' + $Value.Replace('\', '\\').Replace('"', '\"') + '"'
}

function Invoke-CheckedGit {
    param(
        [Parameter(Mandatory)][string] $RepositoryPath,
        [Parameter(Mandatory)][string[]] $Arguments,
        [Parameter(Mandatory)][string] $FailureMessage
    )

    & git -C $RepositoryPath @Arguments
    if ($LASTEXITCODE -ne 0) { throw $FailureMessage }
}

function Apply-IdempotentGitPatch {
    param(
        [Parameter(Mandatory)][string] $RepositoryPath,
        [Parameter(Mandatory)][string] $PatchPath
    )

    & git -C $RepositoryPath apply --unidiff-zero --check -- $PatchPath
    if ($LASTEXITCODE -eq 0) {
        & git -C $RepositoryPath apply --unidiff-zero -- $PatchPath
        if ($LASTEXITCODE -ne 0) { throw 'Failed to apply the pinned OpenKO-db KnightsIndex patch.' }
        return
    }

    & git -C $RepositoryPath apply --unidiff-zero --reverse --check -- $PatchPath
    if ($LASTEXITCODE -ne 0) {
        throw 'The pinned OpenKO-db KnightsIndex patch is neither applicable nor already applied.'
    }
}

function Assert-KodbUtilImportSucceeded {
    param(
        [Parameter(Mandatory)][int] $ExitCode,
        [Parameter(Mandatory)][AllowEmptyCollection()][string[]] $OutputLines
    )

    if ($ExitCode -ne 0) { throw ('kodb-util database clean/import exited with code {0}.' -f $ExitCode) }
    $outputText = $OutputLines -join [Environment]::NewLine
    foreach ($marker in @('Recovered from panic','error executing batch')) {
        if ($outputText.IndexOf($marker, [System.StringComparison]::OrdinalIgnoreCase) -ge 0) {
            throw ('kodb-util reported a false-success marker: {0}' -f $marker)
        }
    }
}

function Assert-KnOnlineObjectInventory {
    param(
        [AllowNull()][string] $CollationName,
        [Parameter(Mandatory)][string[]] $Tables,
        [Parameter(Mandatory)][string[]] $Procedures
    )

    if ([string]::IsNullOrWhiteSpace($CollationName)) { throw 'KN_online has no database collation.' }
    $missing = [System.Collections.Generic.List[string]]::new()
    foreach ($table in @('USERDATA','ITEM','MAGIC')) {
        if ($Tables -notcontains $table) { $missing.Add(('table:{0}' -f $table)) }
    }
    foreach ($procedure in @('ACCOUNT_LOGIN','LOAD_USER_DATA','UPDATE_USER_DATA','CHECK_KNIGHTS')) {
        if ($Procedures -notcontains $procedure) { $missing.Add(('procedure:{0}' -f $procedure)) }
    }
    if ($missing.Count -gt 0) { throw ('KN_online import validation failed; missing {0}.' -f ($missing -join ', ')) }
}

function Assert-KnOnlineDatabaseReady {
    $connection = [System.Data.SqlClient.SqlConnection]::new('Server=.\SQLEXPRESS;Database=KN_online;Integrated Security=True;Encrypt=False;TrustServerCertificate=True')
    try {
        $connection.Open()
        $command = $connection.CreateCommand()
        $command.CommandText = "SELECT CONVERT(nvarchar(128), DATABASEPROPERTYEX(DB_NAME(), 'Collation'))"
        $collationName = [string]$command.ExecuteScalar()

        $command.CommandText = "SELECT [name] FROM sys.tables WHERE [name] IN (N'USERDATA',N'ITEM',N'MAGIC')"
        $tables = [System.Collections.Generic.List[string]]::new()
        $reader = $command.ExecuteReader()
        try { while ($reader.Read()) { $tables.Add([string]$reader.GetString(0)) } } finally { $reader.Close() }

        $command.CommandText = "SELECT [name] FROM sys.procedures WHERE [name] IN (N'ACCOUNT_LOGIN',N'LOAD_USER_DATA',N'UPDATE_USER_DATA',N'CHECK_KNIGHTS')"
        $procedures = [System.Collections.Generic.List[string]]::new()
        $reader = $command.ExecuteReader()
        try { while ($reader.Read()) { $procedures.Add([string]$reader.GetString(0)) } } finally { $reader.Close() }

        Assert-KnOnlineObjectInventory -CollationName $collationName -Tables $tables.ToArray() -Procedures $procedures.ToArray()
    } finally {
        $connection.Dispose()
    }
    Write-Host 'KN_online schema validation: OK'
}

$runtime = Join-Path $PSScriptRoot 'runtime'
$util = Join-Path $runtime 'kodb-util'
$openKoDb = Join-Path $util 'OpenKO-db'
$patchPath = Join-Path $PSScriptRoot 'patches\kodb-util-openko-db-knightsindex.patch'
$kodbUtilCommit = 'aa62573a9eb4594b369e1ffa2df18327669c3feb'
$openKoDbCommit = 'bec619466938d278339c30e5b1a4bff3c9413bab'
$goExecutable = Resolve-GoExecutable
if (-not $goExecutable) { throw 'Go 1.24+ executable not found.' }
New-Item -ItemType Directory -Force -Path $runtime | Out-Null
if (-not (Test-Path -LiteralPath (Join-Path $util '.git'))) {
    git clone https://github.com/Open-KO/kodb-util.git $util
    if ($LASTEXITCODE -ne 0) { throw 'git clone failed while fetching Open-KO/kodb-util.' }
}
& git -C $util cat-file -e (('{0}^{{commit}}' -f $kodbUtilCommit)) 2>$null
if ($LASTEXITCODE -ne 0) {
    Invoke-CheckedGit -RepositoryPath $util -Arguments @('fetch','--no-tags','origin',$kodbUtilCommit) -FailureMessage 'Failed to fetch the pinned kodb-util commit.'
}
Invoke-CheckedGit -RepositoryPath $util -Arguments @('checkout','--detach','--force',$kodbUtilCommit) -FailureMessage 'Failed to check out the pinned kodb-util commit.'
$actualKodbUtilCommit = (& git -C $util rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $actualKodbUtilCommit -ne $kodbUtilCommit) { throw 'kodb-util commit verification failed.' }

Invoke-GitBashSubmodule -RepositoryPath $util -Arguments @('sync','--recursive')
Invoke-GitBashSubmodule -RepositoryPath $util -Arguments @('update','--init','--recursive','--force')
$actualOpenKoDbCommit = (& git -C $openKoDb rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $actualOpenKoDbCommit -ne $openKoDbCommit) { throw 'OpenKO-db submodule commit verification failed.' }
Apply-IdempotentGitPatch -RepositoryPath $openKoDb -PatchPath $patchPath

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
    & $goExecutable mod download
    if ($LASTEXITCODE -ne 0) { throw 'go mod download failed for kodb-util.' }
    $importOutput = [System.Collections.Generic.List[string]]::new()
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $goExecutable run kodb-util.go -clean -import 2>&1 | ForEach-Object {
            $safeLine = ([string]$_).Replace($GameDbPassword, '[REDACTED]')
            $importOutput.Add($safeLine)
            Write-Host $safeLine
        }
        $importExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    Assert-KodbUtilImportSucceeded -ExitCode $importExitCode -OutputLines $importOutput.ToArray()
} finally {
    Pop-Location
}

Assert-KnOnlineDatabaseReady
Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue | Remove-OdbcDsn
Add-OdbcDsn -Name 'KN_online' -DriverName 'ODBC Driver 18 for SQL Server' -DsnType User -Platform '64-bit' -SetPropertyValue @('Server=.\SQLEXPRESS','Database=KN_online','Trusted_Connection=No','Encrypt=Optional','AutoTranslate=No')
Write-Host 'KN_online import and 64-bit DSN setup: OK'
