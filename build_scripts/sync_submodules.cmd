@ECHO OFF
SETLOCAL

REM Setup environment
CALL "%~dp0env_setup.cmd"
IF ERRORLEVEL 1 EXIT /B 1

REM Sync submodules.
"%GitPath%" submodule sync

EXIT /B 0
