@echo off

ECHO.
ECHO ** OpenKO repository setup **
ECHO.
ECHO Preparing to initialise and update submodules
ECHO Finding git.exe...

set "GitDir="
where /q git
if not errorlevel 1 (
 	ECHO Found git.exe in PATH
 	goto init_and_update_submodules
)

ECHO git.exe not found in PATH, checking for GitHub Desktop directories...

set GithubDir=%LOCALAPPDATA%\GitHubDesktop
set "LocalGithubGitDir32=resources\app\git\mingw32\bin\"
set "LocalGithubGitDir64=resources\app\git\mingw64\bin\"
for /f "delims=" %%a in ('dir /b /ad /oN "%GithubDir%\app-*"') do (
	if exist "%GithubDir%\%%a\%LocalGithubGitDir64%git.exe" (
		set "GitDir=%GithubDir%\%%a\%LocalGithubGitDir64%"
	) else (
		if exist "%GithubDir%\%%a\%LocalGithubGitDir32%git.exe" (
			set "GitDir=%GithubDir%\%%a\%LocalGithubGitDir32%"
		)
	)
)

if "%GitDir%"=="" (
	ECHO Failed to update submodules: git.exe not found.
	ECHO Ensure that git is installed and placed in your PATH environment variable.
	PAUSE
	EXIT /B
)

ECHO Found git.exe in GitHub Desktop directory (%GitDir%)

:init_and_update_submodules
ECHO Initialising and updating submodules...
"%GitDir%git.exe" submodule update --init --recursive

:: Find latest Visual Studio with MSBuild
set "MSBUILD="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
	set "MSBUILD=%%i"
)

if "%MSBUILD%"=="" (
	ECHO MSBuild not found! Please ensure that Visual Studio 2022 is installed.
	EXIT /B
)

:: Build all third party dependencies for Debug and Release configurations
"%MSBUILD%" ThirdParty.sln /t:Rebuild /p:Configuration=Debug
"%MSBUILD%" ThirdParty.sln /t:Rebuild /p:Configuration=Release

ECHO.
ECHO All done! You can close this window now.
PAUSE
