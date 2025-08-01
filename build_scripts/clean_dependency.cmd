@ECHO OFF
SETLOCAL

REM Validate arguments
IF "%~4"=="" (
	ECHO Usage: %~nx0 DEP_NAME BUILD_CONFIG BUILD_PLATFORM PROJECT_PATH
	ECHO Example: %~nx0 zlib Release Win32 deps\zlib-msvc\zlib.vcxproj
	EXIT /B 1
)

SET "DEP_NAME=%~1"
SET "BUILD_CONFIG=%~2"
SET "BUILD_PLATFORM=%~3"
SET "PROJECT_PATH=%~4"

ECHO === Cleaning dependency: %DEP_NAME% (%BUILD_CONFIG%-%BUILD_PLATFORM%)

REM Setup environment
CALL "%~dp0env_setup.cmd"
IF ERRORLEVEL 1 EXIT /B 1

SET "REPO_ROOT=%~dp0.."
SET "BUILD_STATE_DIR=%~dp0..\deps\fetch-and-build-wrappers\last-build-states\%BUILD_PLATFORM%\%BUILD_CONFIG%"
SET "BUILD_STATE_FILE=%BUILD_STATE_DIR%\%DEP_NAME%.txt"

REM Validate dependency project even exists
IF NOT EXIST "%PROJECT_PATH%" (
	ECHO ERROR: Dependency project not found: "%PROJECT_PATH%"
	EXIT /B 1
)

REM Build dependency
"%MSBUILD%" "%PROJECT_PATH%" /t:Clean /p:Configuration=%BUILD_CONFIG% /p:Platform=%BUILD_PLATFORM%

IF ERRORLEVEL 1 (
	ECHO ERROR: Failed to build dependency: %DEP_NAME%
	EXIT /B 1
)

REM Delete build key so the next request will invoke a build.
DEL /Q "%BUILD_STATE_FILE%"

REM And we're done.
ECHO [%DEP_NAME%] Cleaning complete.
EXIT /B 0
