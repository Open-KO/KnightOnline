@ECHO OFF
SETLOCAL

SET "GitDir="
WHERE /q git
IF NOT ERRORLEVEL 1 (
	FOR /f "delims=" %%G IN ('where git') DO (
		SET "GitPath=%%G"
		GOTO found_git
	)
)

SET GithubDir=%LOCALAPPDATA%\GitHubDesktop
SET "LocalGithubGitDir32=resources\app\git\mingw32\bin\"
SET "LocalGithubGitDir64=resources\app\git\mingw64\bin\"

SET "GitPath="

FOR /f "delims=" %%a IN ('dir /b /ad /oN "%GithubDir%\app-*"') DO (
	IF EXIST "%GithubDir%\%%a\%LocalGithubGitDir64%\git.exe" (
		SET "GitPath=%GithubDir%\%%a\%LocalGithubGitDir64%\git.exe"
		GOTO found_git
	) ELSE IF EXIST "%GithubDir%\%%a\%LocalGithubGitDir32%\git.exe" (
		SET "GitPath=%GithubDir%\%%a\%LocalGithubGitDir32%\git.exe"
		GOTO found_git
	)
)

EXIT /B 1

:found_git
:: Extract directory part
FOR %%D IN ("%GitPath%") do set "GitDir=%%~dpD"

:: Export GitDir and GitPath environment variables for caller
ENDLOCAL & SET "GitDir=%GitDir%" & SET "GitPath=%GitPath%"

EXIT /B 0
