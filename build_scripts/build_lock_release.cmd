@ECHO OFF
SETLOCAL

REM The idea behind this is that MKDIR is atomic.
REM As such, it can be used as an arbitrary locking mechanism.

REM Validate arguments
IF "%~1"=="" (
	ECHO Usage: %~nx0 GIT_LOCK_DIR
	ECHO Example: %~nx0 "deps\fetch-and-build-wrappers\last-build-states\Win32\Release\git_lock"
	EXIT /B 1
)

REM Just a pre-safety check. This should definitely exist at this point.
IF NOT EXIST "%GIT_LOCK_DIR%" (
	ECHO WARNING: Lock directory does not exist - %GIT_LOCK_DIR%
	EXIT /B 1
)

REM Attempt to remove it.
RMDIR "%GIT_LOCK_DIR%" 2>nul

REM Make sure we actually did...
IF %ERRORLEVEL% NEQ 0 (
	ECHO WARNING: Failed to remove lock directory - %GIT_LOCK_DIR%
	EXIT /B 1
) ELSE (
	ECHO git lock released
	EXIT /B 0
)
