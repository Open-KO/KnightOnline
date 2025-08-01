@ECHO OFF
SETLOCAL DisableDelayedExpansion

:: Config
SET "SOURCE_DIR=..\db-modules"
SET "OUTPUT_FILE=%~dp0db-modules.vcxproj"
SET "OUTPUT_FILE_TMP=%OUTPUT_FILE%.tmp"
SET "TEMPLATE_FILE=%OUTPUT_FILE%.template"
SET "OUTPUT_FILE_FILTERS=%OUTPUT_FILE%.filters"
SET "OUTPUT_FILE_FILTERS_TMP=%OUTPUT_FILE_FILTERS%.tmp"

:: Get absolute path of source dir so that we can replace it in the output with relative paths
PUSHD "%SOURCE_DIR%"
SET "ABS_SOURCE_DIR=%CD%"
POPD

ECHO Generating %OUTPUT_FILE% from %TEMPLATE_FILE%...

> "%OUTPUT_FILE_TMP%" (
	FOR /F "usebackq delims=" %%L IN ("%TEMPLATE_FILE%") DO (
		REM Found the placeholder. Find all our module files and include them here.
		IF "%%L"=="<!-- SOURCE FILES -->" (
			REM Scan for all .ixx (module) files recursively
			CALL "%~dp0GenerateIncludes.cmd" "%SOURCE_DIR%" "%ABS_SOURCE_DIR%"
		) ELSE IF "%%L"=="</Project>" (
			REM Preserve official project behaviour with no trailing newline (so VS shouldn't change it)
			<nul SET /P=^</Project^>
		) ELSE (
			REM Echo lines starting with special characters safely (it looks weird, but that's batch for you)
			ECHO(%%L
		)
	)
)

ECHO Done.

> "%OUTPUT_FILE_FILTERS_TMP%" (
	CALL "%~dp0GenerateFilters.cmd" "%SOURCE_DIR%" "%ABS_SOURCE_DIR%"
)

REM Only replace if they don't already match
REM This avoids VS unnecessarily triggering due to the modification time changing;
REM we should just only replace if they're different period.
FC /B "%OUTPUT_FILE%" "%OUTPUT_FILE_TMP%" >nul
IF ERRORLEVEL 1 (
	COPY /Y "%OUTPUT_FILE_TMP%" "%OUTPUT_FILE%" >nul
)

DEL /F /Q "%OUTPUT_FILE_TMP%"

FC /B "%OUTPUT_FILE_FILTERS%" "%OUTPUT_FILE_FILTERS_TMP%" >nul
IF ERRORLEVEL 1 (
	COPY /Y "%OUTPUT_FILE_FILTERS_TMP%" "%OUTPUT_FILE_FILTERS%" >nul
)

DEL /F /Q "%OUTPUT_FILE_FILTERS_TMP%"

EXIT /B 0
