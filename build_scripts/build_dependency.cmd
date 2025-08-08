@ECHO OFF
SETLOCAL

REM Validate arguments
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

SET "DEP_PATH_NORMALISED=%DEP_PATH:\=/%"

REM ECHO "%~dp0%~nx0" "%DEP_NAME%" "%DEP_PATH%" "%BUILD_CONFIG%" "%BUILD_PLATFORM%" "%VCTOOLS_VERSION%" "%PROJECT_PATH%"

ECHO === Building dependency: %DEP_NAME% [%DEP_PATH%] (%BUILD_CONFIG%-%BUILD_PLATFORM%) with toolset %VCTOOLS_VERSION%

REM Setup environment
CALL "%~dp0env_setup.cmd"
IF ERRORLEVEL 1 EXIT /B 1

SET "REPO_ROOT=%~dp0.."

REM Ensure REPO_ROOT is an absolute path
PUSHD "%~dp0\.."
SET REPO_ROOT=%CD%
POPD

SET "BUILD_STATE_DIR=%REPO_ROOT%\deps\fetch-and-build-wrappers\last-build-states\%BUILD_PLATFORM%\%BUILD_CONFIG%"
SET "BUILD_STATE_FILE=%BUILD_STATE_DIR%\%DEP_NAME%.txt"

REM Create the build state directory if it doesn't already exist
IF NOT EXIST "%BUILD_STATE_DIR%" (
	MKDIR "%BUILD_STATE_DIR%"
)

REM Read stored key if exists
SET "STORED_BUILD_STATE_KEY="
IF EXIST "%BUILD_STATE_FILE%" (
	SET /P "STORED_BUILD_STATE_KEY="<"%BUILD_STATE_FILE%"
)

REM Get intended submodule commit hash
SET "INTENDED_COMMIT="
SET "SUBMODULE_STAGED=0"
SET "SUBMODULE_DIRTY=0"
SET "OVERRIDE_BUILD=0"
IF EXIST "%REPO_ROOT%\%DEP_PATH%\.git" (
	PUSHD "%REPO_ROOT%"
	FOR /F "tokens=3" %%c IN ('"%GitPath%" ls-tree HEAD %DEP_PATH%') DO SET "INTENDED_COMMIT=%%c"

	REM Determine if the developer has staged a change to this submodule.
	FOR /F %%c IN ('"%GitPath%" diff --cached HEAD --name-only') DO (
		IF /i "%%c"=="%DEP_PATH_NORMALISED%" (
			SET "SUBMODULE_STAGED=1"
			SET "OVERRIDE_BUILD=1"
		)
	)
	
	REM Determine if the submodule's dirty (pending unstaged changes)
	"%GitPath%" -C %DEP_PATH% diff --quiet
	IF ERRORLEVEL 1 (
		SET "SUBMODULE_DIRTY=1"
		SET "OVERRIDE_BUILD=1"
	)
	POPD
)

REM Prepare build state key: commit + compiler version
SET "CURRENT_BUILD_STATE_KEY=%INTENDED_COMMIT%;%VCTOOLS_VERSION%"

REM Compare: rebuild only if changed
IF "%OVERRIDE_BUILD%"=="0" (
	IF "%CURRENT_BUILD_STATE_KEY%"=="%STORED_BUILD_STATE_KEY%" (
		ECHO [%DEP_NAME%] No changes detected - commit [%INTENDED_COMMIT%] and compiler [%VCTOOLS_VERSION%] match. Skipping build.
		EXIT /B 0
	)
)

REM This is just fluff; we can detect if it's a new build or a changed one here.
REM If the submodule's not initialized, .git won't exist, so %INTENDED_COMMIT% will not be set. 
IF "%INTENDED_COMMIT%"=="" (
	ECHO [%DEP_NAME%] New submodule detected - initializing dependency.
	SET "UPDATE_SUBMODULE=1"
) ELSE IF "%SUBMODULE_STAGED%"=="1" (
	ECHO [%DEP_NAME%] Submodule staged - using submodule as-is.
) ELSE IF "%SUBMODULE_DIRTY%"=="1" (
	ECHO [%DEP_NAME%] Submodule dirty - using submodule as-is. If this is undesired, please reset the submodule.
) ELSE (
	ECHO [%DEP_NAME%] Change detected - updating dependency.
	SET "UPDATE_SUBMODULE=1"
)

IF "%UPDATE_SUBMODULE%"=="1" (
	ECHO Please note that this may take some time, so please be patient.

	REM Update submodule.
	PUSHD "%REPO_ROOT%"
	"%GitPath%" submodule update --init --recursive "%DEP_PATH%"
	POPD
)

REM Not all dependencies need to be built (e.g. ko-client-assets).
REM As such, we should check if the linked project is set.
IF NOT "%PROJECT_PATH%"=="" (
	REM Validate dependency project even exists
	IF NOT EXIST "%PROJECT_PATH%" (
		ECHO ERROR: Dependency project not found: "%PROJECT_PATH%"
		EXIT /B 1
	)

	REM Build dependency with a full rebuild (clean + build)
	"%MSBUILD%" "%PROJECT_PATH%" /t:Rebuild /p:Configuration=%BUILD_CONFIG% /p:Platform=%BUILD_PLATFORM%
	IF ERRORLEVEL 1 (
		ECHO ERROR: Failed to build dependency: %DEP_NAME%
		EXIT /B 1
	)
)

REM Fetch current commit hash, now that the submodule is initialized and updated
SET "CURRENT_COMMIT="

REM NOTE: If we're overriding builds, just store an empty commit to encourage a proper build on next change.
IF "%OVERRIDE_BUILD%"=="0" (
	PUSHD "%REPO_ROOT%"
	FOR /F "delims=" %%c IN ('"%GitPath%" -C %DEP_PATH% rev-parse HEAD') DO SET "CURRENT_COMMIT=%%c"
	POPD
)

REM Rebuild the build state key
SET "CURRENT_BUILD_STATE_KEY=%CURRENT_COMMIT%;%VCTOOLS_VERSION%"

REM Save new build key to the build state file for future lookups
ECHO %CURRENT_BUILD_STATE_KEY%>"%BUILD_STATE_FILE%"

REM And we're done.
ECHO [%DEP_NAME%] Build complete.
EXIT /B 0
