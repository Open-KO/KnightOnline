@ECHO OFF
SETLOCAL

REM Try to find vswhere.exe -- this is present as such in the latest Visual Studio (2022)
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

IF NOT EXIST "%VSWHERE%" (
	ECHO ERROR: vswhere.exe not found at "%VSWHERE%".
	ECHO Please install Visual Studio 2022 or later.
	EXIT /B 1
)

REM Required minimum VS version (17.0 = VS2022)
SET "VS_VERSION=17.0"

REM Find latest MSBuild.exe path
SET "MSBUILD="
SET "MSBUILD_PREVIEW="

FOR /f "usebackq tokens=*" %%i IN (`"%VSWHERE%" -version %VS_VERSION% -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) DO (
	SET "MSBUILD=%%i"
)

FOR /f "usebackq tokens=*" %%i IN (`"%VSWHERE%" -version %VS_VERSION% -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) DO (
	SET "MSBUILD_PREVIEW=%%i"
)

REM Search for preview builds as a fallback only, for cases where only preview is installed (we don't want to use it by default)
IF "%MSBUILD%"=="" (
	IF "%MSBUILD_PREVIEW%"=="" (
		ECHO ERROR: MSBuild.exe not found! Please ensure that Visual Studio 2022 or later is installed.
		EXIT /B 1
	)
	
	SET "MSBUILD=%MSBUILD_PREVIEW%"
)

REM Export MSBUILD environment variable for caller
ENDLOCAL & SET "MSBUILD=%MSBUILD%"
EXIT /B 0
