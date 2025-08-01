@ECHO OFF
SETLOCAL EnableDelayedExpansion

SET "SOURCE_DIR=%~1"
SET "ABS_SOURCE_DIR=%~2"

ECHO ^<?xml version="1.0" encoding="utf-8"?^>
ECHO ^<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003"^>
ECHO   ^<ItemGroup^>

FOR /R "%ABS_SOURCE_DIR%" %%F IN (*.ixx) DO (
	SET "FILE=%%F"
	SET "FILE=!FILE:%ABS_SOURCE_DIR%=%SOURCE_DIR%!"
	ECHO     ^<ClCompile Include="!FILE!" /^>
)

ECHO   ^</ItemGroup^>
REM ECHO ^</Project^>
<nul SET /P=^</Project^>

ENDLOCAL
