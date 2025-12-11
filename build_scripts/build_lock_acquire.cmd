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

SET "GIT_LOCK_DIR=%~1"

REM Config
SET MAX_WAIT_TIME=1800
SET SECONDS_ELAPSED=0

REM Try to acquire the "lock"
:acquire_lock
MKDIR "%GIT_LOCK_DIR%" 2>NUL
IF %ERRORLEVEL%==0 (
	REM Lock acquired
	ECHO git lock acquired
	EXIT /B 0
)

REM Lock already exists, so we should wait.
SET /a SECONDS_ELAPSED+=1

REM We've waited longer than our maximum (30s), so we should give up.
IF %SECONDS_ELAPSED% GEQ %MAX_WAIT_TIME% (
	ECHO ERROR: Could not acquire lock after %MAX_WAIT_TIME% seconds.
	EXIT /B 1
)

REM ECHO Waiting for lock... (%SECONDS_ELAPSED%s)
ping 127.0.0.1 -n 2 -w 1000 >NUL
GOTO acquire_lock
