[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string] $Configuration = 'Debug',
    [string] $MSBuildPath,
    [string] $VsWherePath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if ($PSBoundParameters.ContainsKey('MSBuildPath') -and $PSBoundParameters.ContainsKey('VsWherePath')) {
    throw 'Specify either MSBuildPath or VsWherePath, not both.'
}
if ($PSBoundParameters.ContainsKey('MSBuildPath')) {
    if (-not (Test-Path -LiteralPath $MSBuildPath -PathType Leaf)) {
        throw ('MSBuild path not found: {0}' -f $MSBuildPath)
    }
    $msbuild = (Resolve-Path -LiteralPath $MSBuildPath).ProviderPath
} else {
    if ($PSBoundParameters.ContainsKey('VsWherePath')) {
        if (-not (Test-Path -LiteralPath $VsWherePath -PathType Leaf)) {
            throw ('vswhere path not found: {0}' -f $VsWherePath)
        }
        $vswhere = (Resolve-Path -LiteralPath $VsWherePath).ProviderPath
    } else {
        $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
        $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    }
    $msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    $vswhereExitCode = $LASTEXITCODE
    if ($vswhereExitCode -ne 0) { throw ('vswhere failed with exit code {0}.' -f $vswhereExitCode) }
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
