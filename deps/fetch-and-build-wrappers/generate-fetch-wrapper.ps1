param(
    [Parameter(Mandatory = $true)]
    [string]$DependencyName,

    [bool]$HasBuildProject
)

if ($PSBoundParameters.ContainsKey('HasBuildProject') -eq $false) {
	do {
		$input = Read-Host "Does this dependency have a corresponding MSVC build project? (y/n)"
	} while ($input -notmatch '^[yY1nN0]$')

	$HasBuildProject = $input -match '^[yY1]$'
}

$LocalDependencyDir = 'deps\' + $DependencyName

if ($HasBuildProject) {
	$RealProjectPath = '$(RootDir)' + $LocalDependencyDir + '-msvc\' + $DependencyName + '.vcxproj'
} else {
	$RealProjectPath = ''
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$OutputFile = Join-Path $ScriptDir ("fetch-" + $DependencyName + ".vcxproj")

# Generate a new GUID for the project
$guid = [guid]::NewGuid().ToString().ToUpper()

# Template for the .vcxproj file
$projectXml = @"
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{$guid}</ProjectGuid>
  </PropertyGroup>
  <Import Project="..\..\props\platform_root.props" />
  <PropertyGroup Label="UserMacros">
    <DependencyName>$DependencyName</DependencyName>
    <LocalDependencyDir>$LocalDependencyDir</LocalDependencyDir>
    <RealProjectPath>$RealProjectPath</RealProjectPath>
  </PropertyGroup>
  <Import Project="`$(PropsDir)fetch_wrapper.props" />
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@

$projectXml | Out-File -FilePath $OutputFile -Encoding Ascii -NoNewline

Write-Host "Generated project file $OutputFile with GUID $guid"
