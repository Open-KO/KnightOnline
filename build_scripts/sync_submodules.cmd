@ECHO OFF
SETLOCAL

REM Validate arguments
IF "%~2"=="" (
	ECHO Usage: %~nx0 BUILD_CONFIG BUILD_PLATFORM
	ECHO Example: %~nx0 Release Win32
	EXIT /B 1
)

SET "BUILD_CONFIG=%~1"
SET "BUILD_PLATFORM=%~2"

REM Setup environment
CALL "%~dp0env_setup.cmd"
IF ERRORLEVEL 1 EXIT /B 1

SET "REPO_ROOT=%~dp0.."

REM Ensure REPO_ROOT is an absolute path
PUSHD "%~dp0\.."
SET REPO_ROOT=%CD%
POPD

SET "BUILD_STATE_DIR=%REPO_ROOT%\deps\fetch-and-build-wrappers\last-build-states\%BUILD_PLATFORM%\%BUILD_CONFIG%"
SET "GIT_LOCK_DIR=%BUILD_STATE_DIR%\git_lock"

REM We sync submodules at the start of a build, so we can just reset the lock dir if it exists already.
IF EXIST "%GIT_LOCK_DIR%" (
	RMDIR "%GIT_LOCK_DIR%" 
)

REM Trigger build lock to force other projects to wait until git's done
CALL "%~dp0build_lock_acquire.cmd" "%GIT_LOCK_DIR%"
IF ERRORLEVEL 1 EXIT /B 1

REM Sync submodules.
"%GitPath%" submodule sync

REM Release build lock
CALL "%~dp0build_lock_release.cmd" "%GIT_LOCK_DIR%"
IF ERRORLEVEL 1 EXIT /B 1

EXIT /B 0
