[CmdletBinding()]
param([ValidateSet('Debug','Release')][string] $Configuration = 'Debug')

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { throw 'MSBuild 17.x not found.' }

Push-Location $repoRoot
try {
    foreach ($solution in @('Server.slnx','Client.slnx','Tests.slnx')) {
        & $msbuild /m /p:Configuration=$Configuration /p:Platform=x64 $solution
        if ($LASTEXITCODE -ne 0) { throw ('Build failed: {0}' -f $solution) }
    }
} finally {
    Pop-Location
}
