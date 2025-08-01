@ECHO OFF

CALL "%~dp0env_find_git.cmd"
IF NOT ERRORLEVEL 0 (
	ECHO ERROR: git.exe not found.
	ECHO Ensure that git is installed and placed in your PATH environment variable.
	EXIT /B
)

REM ECHO Found git.exe at: %GitPath%

CALL "%~dp0env_find_msbuild.cmd"
IF NOT ERRORLEVEL 0 (
	ECHO ERROR: MSBuild.exe not found.
	ECHO Ensure that Microsoft Visual Studio 2022 is installed.
	EXIT /B
)

REM ECHO Found MSBuild.exe at: %MSBUILD%
EXIT /B 0
