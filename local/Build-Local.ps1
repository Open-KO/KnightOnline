[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string] $Configuration = 'Debug',
    [string] $MSBuildPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if ($PSBoundParameters.ContainsKey('MSBuildPath')) {
    if (-not (Test-Path -LiteralPath $MSBuildPath -PathType Leaf)) {
        throw ('MSBuild path not found: {0}' -f $MSBuildPath)
    }
    $msbuild = (Resolve-Path -LiteralPath $MSBuildPath).ProviderPath
} else {
    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    $msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) { throw 'MSBuild 17.x not found.' }
}

Push-Location $repoRoot
try {
    foreach ($project in @(
        'Server.slnx',
        'Client.slnx',
        'deps\googletest-msvc\googletest-distribution.slnx',
        'Tests.slnx')) {
        & $msbuild /m /p:Configuration=$Configuration /p:Platform=x64 $project
        if ($LASTEXITCODE -ne 0) { throw ('Build failed: {0}' -f $project) }
    }
} finally {
    Pop-Location
}
