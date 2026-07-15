# Local OpenKO bot server

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

`Start-Local.ps1` reads exactly one `GAME_DB_PASSWORD` entry from the ignored `local\.env.local` file. If setup did not create that file, add it without committing it:

```text
GAME_DB_PASSWORD=<the local knight SQL password>
```

The launcher regenerates the ignored `Aujard.ini`, `ItemManager.ini`, `Version.ini`, `server.ini`, and `gameserver.ini` files. It creates `assets\Client\Server.ini` from `assets\Client\Server.ini.default` when needed and forces every client IP entry to `127.0.0.1`. The generated bot configuration is:

```ini
[AI_SERVER]
IP=127.0.0.1

[ODBC]
GAME_DSN=KN_online
GAME_UID=knight

[BOTS]
Enabled=1
Count=10
TickMilliseconds=200
RespawnSeconds=15
Zone=201
AttackRange=2.5
MoveStep=1.5
```

The runtime file also contains `GAME_PWD`, populated from `local\.env.local`; the launcher never prints it. Generated INI files, logs, runtime files, temporary PID-state files, and `local\pids.json` are ignored by Git.

## Start and stop

Build the selected configuration before launching it. Start the complete stack from the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Start-Local.ps1 -Configuration Debug
```

`-Configuration` accepts exactly `Debug` or `Release`. The launcher requires the prerequisite check and `MSSQL$SQLEXPRESS` to pass, rejects occupied service ports or non-loopback configuration, and starts these owned processes in order:

```text
Aujard -> ItemManager -> VersionManager -> AIServer -> Ebenezer -> KnightOnLine
```

It waits no more than 60 seconds for VersionManager (`15100`), AIServer (`10020`), and Ebenezer (`15001`) on `127.0.0.1`. `KnightOnLine.exe` starts only after those readiness checks pass. Standard output and error are written under `local\logs`. Exact executable path, PID, logical executable name, and UTC start time are atomically recorded in `local\pids.json`. A partial startup stops only processes created by that invocation; incomplete rollback leaves ownership state for a safe retry.

Stop only the processes owned by the launcher:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Stop-Local.ps1
```

The stop script validates the state schema, executable name/path, PID, and start time before stopping an exact PID. It never enumerates or kills by process name. Already-exited PIDs are tolerated. A mismatch or incomplete stop preserves the remaining state instead of risking an unrelated process.

## Bot administrator commands

Run these from an authorized in-game GM account:

```text
+bot_status
+bot_remove_all
+bot_add karus warrior 5
+bot_add elmorad warrior 5
+bot_start_pk
```

The default startup roster is exactly five Karus warriors and five El Morad warriors in zone `201`.

## Acceptance run

The operations scripts do not by themselves prove gameplay acceptance. After the Debug build/tests pass, perform and record all of the following with `testing/testing`:

1. Start the stack, log in, and enter zone `201`.
2. Confirm exactly five `Bot_K_*` and five `Bot_E_*` models are visible.
3. Confirm movement, opposing-nation targeting, approach, and basic attacks.
4. Confirm a real player can target, damage, and kill a bot, and a bot can damage and kill the real player through ordinary rules.
5. Confirm a dead bot remains dead for 15 seconds and respawns at its nation home point.
6. Confirm `+bot_status` reports `total=10` and `alive+dead=total`.
7. Confirm `+bot_remove_all` leaves no ghost model, add the 5+5 roster again, and run `+bot_start_pk`.
8. Keep the client and services running for 30 minutes. Once per minute record total/alive/dead, registry size, and every owned process's liveness in ignored `local\logs\stability.csv`.
9. At the end, confirm no process exited; bot IDs are unique and within `3000-3499`; every region bot ID resolves through `GetUserPtr`; `alive+dead=total`; and `+bot_remove_all` leaves no bot ID in any region.
10. Stop the stack with `Stop-Local.ps1`, then build and run the Release-x64 tests.

The manual gameplay and 30-minute evidence remain pending until this complete scenario is run on the local host.
