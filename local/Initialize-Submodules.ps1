[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Local-Tooling.ps1')

$repoRoot = Split-Path -Parent $PSScriptRoot
Invoke-GitBashSubmodule -RepositoryPath $repoRoot -Arguments @('sync','--recursive')
Invoke-GitBashSubmodule -RepositoryPath $repoRoot -Arguments @('update','--init','--recursive')
Write-Host 'OpenKO submodules: OK'
