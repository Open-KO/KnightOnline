# Local OpenKO baseline

This setup is for localhost development only. Do not expose the database, DSN, or game services to another host.

## Prerequisites

- Visual Studio 2022 Build Tools with MSBuild, the C++ build tools workload, ATL, and ATL/MFC
- SQL Server 2022 Express installed as `SQLEXPRESS`, with Database Engine Services and TCP/IP enabled; the current Windows user must be an administrator for the instance
- Microsoft 64-bit ODBC Driver 18 for SQL Server
- Go 1.24 or newer
- Initialized repository submodules

Initialize dependencies from a normal user PowerShell:

```powershell
cmd /c .\build_scripts\sync_submodules.cmd Debug x64
git submodule update --init --recursive
```

Verify the host before continuing:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Test-Prerequisites.ps1
```

The checker must exit 0 and print `OpenKO prerequisites: OK`.

## Database and DSN

Import `KN_online` and create the 64-bit user DSN:

```powershell
$gamePassword = Read-Host 'Local knight SQL password'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Setup-Database.ps1 -GameDbPassword $gamePassword
```

The `KN_online` DSN uses ODBC Driver 18, SQL authentication, optional encryption, and `AutoTranslate=No`. The password is supplied interactively and must not be committed.

## Build and test

Build every unmodified solution for x64 and run the existing server tests:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
```

The default development login is `testing/testing`.

## Generated local configuration

The ignored local INI files are `Aujard.ini`, `ItemManager.ini`, `Version.ini`, `server.ini`, `gameserver.ini`, and `assets\Client\Server.ini`. The client file is generated from `assets\Client\Server.ini.default`. Keep all server addresses loopback-only (`127.0.0.1` or `localhost`) and never commit generated INI files.
