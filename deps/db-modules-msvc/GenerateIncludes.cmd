@ECHO OFF
SETLOCAL EnableDelayedExpansion

SET "SOURCE_DIR=%~1"
SET "ABS_SOURCE_DIR=%~2"

ECHO   ^<ItemGroup^>
FOR /r "%ABS_SOURCE_DIR%" %%F IN (*.ixx) DO (
	SET "FILE=%%F"
	SET "FILE=!FILE:%ABS_SOURCE_DIR%=%SOURCE_DIR%!"
	ECHO     ^<ClCompile Include="!FILE!"^>
	ECHO         ^<CompileAsCppModule^>true^</CompileAsCppModule^>
	ECHO     ^</ClCompile^>
)

ECHO   ^</ItemGroup^>
ENDLOCAL
