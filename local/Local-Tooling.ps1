function Resolve-GoExecutable {
    $goCommand = Get-Command go -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -ne $goCommand) { return [string]$goCommand.Source }

    $candidates = [System.Collections.Generic.List[string]]::new()
    if ($env:ProgramFiles) { $candidates.Add((Join-Path $env:ProgramFiles 'Go\bin\go.exe')) }
    if (${env:ProgramFiles(x86)}) { $candidates.Add((Join-Path ${env:ProgramFiles(x86)} 'Go\bin\go.exe')) }

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }

    return $null
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
