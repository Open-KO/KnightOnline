@ECHO OFF
SETLOCAL

REM Try to find vswhere.exe -- this is present as such in the latest Visual Studio (2022)
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

IF NOT EXIST "%VSWHERE%" (
	ECHO ERROR: vswhere.exe not found at "%VSWHERE%".
	ECHO Please install Visual Studio 2022 or later.
	EXIT /B 1
)

REM Find latest MSBuild.exe path
SET "MSBUILD="
FOR /f "usebackq tokens=*" %%i IN (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) DO (
	SET "MSBUILD=%%i"
)

IF  "%MSBUILD%"=="" (
	ECHO ERROR: MSBuild.exe not found! Please ensure that Visual Studio 2022 or later is installed.
	EXIT /B 1
)

REM Export MSBUILD environment variable for caller
ENDLOCAL & SET "MSBUILD=%MSBUILD%"
EXIT /B 0
