# Local OpenKO baseline

This setup is for single-machine development only. Configure every game and client service endpoint as the literal `127.0.0.1`; hostnames such as `localhost` are not permitted. Do not expose the database, DSN, or game services to another host.

## Prerequisites

- Visual Studio 2022 Build Tools with MSBuild, the C++ build tools workload, ATL, and ATL/MFC
- SQL Server 2022 Express installed as `SQLEXPRESS`, with Database Engine Services and TCP/IP enabled; the current Windows user must be an administrator for the instance
- Microsoft 64-bit ODBC Driver 18 for SQL Server
- Go 1.24 or newer
- Initialized repository submodules

Initialize dependencies from a normal user PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Initialize-Submodules.ps1
```

The committed initializer uses Git Bash so submodule commands do not depend on the inherited PowerShell `PATH` bridge.

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

Database setup pins `kodb-util` to `aa62573a9eb4594b369e1ffa2df18327669c3feb` and its `OpenKO-db` submodule to `bec619466938d278339c30e5b1a4bff3c9413bab`. Before import it force-resets the submodule and idempotently applies the auditable `local\patches\kodb-util-openko-db-knightsindex.patch`, which normalizes variable casing in `CHECK_KNIGHTS` and `EDITER_KNIGHTS` for case-sensitive SQL collations.

The sanitized combined import log remains visible, but setup fails on nonzero exit or upstream false-success markers such as `Recovered from panic` and `error executing batch`. Before creating the DSN, an integrated-security SQL check requires a non-null `KN_online` collation plus tables `USERDATA`, `ITEM`, `MAGIC` and procedures `ACCOUNT_LOGIN`, `LOAD_USER_DATA`, `UPDATE_USER_DATA`, `CHECK_KNIGHTS`, `EDITER_KNIGHTS`.

## Build and test

Build every unmodified solution for x64 and run the existing server tests:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
```

The default development login is `testing/testing`.

## Generated local configuration

The ignored local INI files are `Aujard.ini`, `ItemManager.ini`, `Version.ini`, `server.ini`, `gameserver.ini`, and `assets\Client\Server.ini`. The client file is generated from `assets\Client\Server.ini.default`. Keep all game and client server addresses at the literal `127.0.0.1` only and never commit generated INI files.
