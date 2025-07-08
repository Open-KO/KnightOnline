@ECHO OFF

:: Validate arguments
IF "%~5"=="" (
	ECHO Usage: %~nx0 DEP_NAME DEP_PATH BUILD_CONFIG BUILD_PLATFORM VCTOOLS_VERSION [PROJECT_PATH]
	ECHO Example: %~nx0 zlib deps\zlib Release Win32 14.44.35207 deps\zlib-msvc\zlib.vcxproj
	EXIT /B 1
)

SET "DEP_NAME=%~1"
SET "DEP_PATH=%~2"
SET "BUILD_CONFIG=%~3"
SET "BUILD_PLATFORM=%~4"
SET "VCTOOLS_VERSION=%~5"
SET "PROJECT_PATH=%~6"

ECHO === Building dependency: %DEP_NAME% [%DEP_PATH%] (%BUILD_CONFIG%-%BUILD_PLATFORM%) with toolset %VCTOOLS_VERSION%

:: Setup environment
CALL "%~dp0env_setup.cmd"
IF ERRORLEVEL 1 EXIT /B 1

SET "REPO_ROOT=%~dp0.."
SET "BUILD_STATE_DIR=%~dp0..\deps\fetch-and-build-wrappers\last-build-states\%BUILD_PLATFORM%\%BUILD_CONFIG%"
SET "BUILD_STATE_FILE=%BUILD_STATE_DIR%\%DEP_NAME%.txt"

:: Create the build state directory if it doesn't already exist
IF NOT EXIST "%BUILD_STATE_DIR%" (
	MKDIR "%BUILD_STATE_DIR%"
)

:: Read stored key if exists
SET "STORED_BUILD_STATE_KEY="
IF EXIST "%BUILD_STATE_FILE%" (
	SET /P "STORED_BUILD_STATE_KEY="<"%BUILD_STATE_FILE%"
)

:: Get current submodule commit hash
SET "CURRENT_COMMIT="
IF EXIST "%REPO_ROOT%\%DEP_PATH%\.git" (
	PUSHD "%REPO_ROOT%"
	FOR /F "delims=" %%c IN ('"%GitPath%" -C %DEP_PATH% rev-parse HEAD') DO SET "CURRENT_COMMIT=%%c"
	POPD
)

:: Prepare build state key: commit + compiler version
SET "CURRENT_BUILD_STATE_KEY=%CURRENT_COMMIT%;%VCTOOLS_VERSION%"

:: Compare: rebuild only if changed
IF "%CURRENT_BUILD_STATE_KEY%"=="%STORED_BUILD_STATE_KEY%" (
	ECHO [%DEP_NAME%] No changes detected - commit and compiler match. Skipping build.
	EXIT /B 0
)

:: This is just fluff; we can detect if it's a new build or a changed one here.
:: If the submodule's not initialized, .git won't exist, so %CURRENT_COMMIT% will not be set. 
IF "%CURRENT_COMMIT%"=="" (
	ECHO [%DEP_NAME%] New submodule detected - initializing dependency.
) ELSE (
	ECHO [%DEP_NAME%] Change detected - updating dependency.
)

ECHO Please note that this may take some time, so please be patient.

:: Update submodule.
PUSHD "%REPO_ROOT%"
"%GitPath%" submodule update --init --recursive "%DEP_PATH%"
POPD

:: Not all dependencies need to be built (e.g. ko-client-assets).
:: As such, we should check if the linked project is set.
IF NOT "%PROJECT_PATH%"=="" (
	:: Validate dependency project even exists
	IF NOT EXIST "%PROJECT_PATH%" (
		ECHO ERROR: Dependency project not found: "%PROJECT_PATH%"
		EXIT /B 1
	)

	:: Build dependency
	"%MSBUILD%" "%PROJECT_PATH%" /t:Rebuild /p:Configuration=%BUILD_CONFIG% /p:Platform=%BUILD_PLATFORM%

	IF ERRORLEVEL 1 (
		ECHO ERROR: Failed to build dependency: %DEP_NAME%
		EXIT /B 1
	)
)

:: Fetch current commit hash, now that the submodule is initialized and updated
PUSHD "%REPO_ROOT%"
FOR /F "delims=" %%c IN ('"%GitPath%" -C %DEP_PATH% rev-parse HEAD') DO SET "CURRENT_COMMIT=%%c"
POPD

:: Rebuild the build state key
SET "CURRENT_BUILD_STATE_KEY=%CURRENT_COMMIT%;%VCTOOLS_VERSION%"

:: Save new build key to the build state file for future lookups
ECHO %CURRENT_BUILD_STATE_KEY%>"%BUILD_STATE_FILE%"

:: And we're done.
ECHO [%DEP_NAME%] Build complete.
EXIT /B 0
