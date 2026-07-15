# OpenKO Local Server-Side PK Bots Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and run OpenKO 1298/1299 entirely on localhost, then add ten socketless Ebenezer player bots that patrol, target the nearest enemy, perform authoritative basic attacks, die, and respawn.

**Architecture:** Keep real network sockets at 3000 while extending the player-entity band to 3500 for bot IDs 3000–3499. Ebenezer owns a BotManager, BotRegistry, socketless CBotUser objects, pure targeting/state logic, and a command facade that drives existing CUser movement and attack paths; AIServer accepts the wider entity band while Aujard and VersionManager remain real-player-only.

**Tech Stack:** C++20, MSVC v143, Visual Studio 2022/MSBuild x64, GoogleTest, PowerShell, SQL Server 2022 Express, 64-bit ODBC, Go 1.24+, and Open-KO/kodb-util.

## Global Constraints

- Active repository: C:\Users\hpete\Desktop\KO1453\openko-1299.
- Reference-only repository: C:\Users\hpete\Desktop\KO1453\legacy-v1453; never link or copy its code into the active build.
- Bind the client and every service to 127.0.0.1.
- Keep real socket IDs at 0–2999, bot IDs at 3000–3499, and NPC IDs at 10000+.
- First milestone is exactly 5 Karus and 5 El Morad transient bots in zone 201 unless map/database validation selects another working PK zone.
- Tick interval is exactly 200 ms and respawn delay is exactly 15 seconds.
- Bots are memory-only: no Aujard character, inventory, account, or SQL persistence.
- First milestone uses basic attacks only; no party, potion, skill chain, class-specific tactics, equipment persistence, NP persistence, or 100–500 bot performance claim.
- Reuse authoritative CUser movement, targeting, damage, HP, death, and region packet paths; do not duplicate combat formulas.
- Do not commit passwords, generated INI files, runtime logs, database files, client assets, or local tool downloads.
- Do not redistribute Open-KO/ko-client-assets and do not push this local AI-assisted fork upstream.
- Use x64 for the client, servers, ODBC DSN, and tests.
- Apply TDD for every production change and commit after each independently passing task.

---

## File Map

### Local operation

- Create local/Test-Prerequisites.ps1 — verifies MSBuild, VS components, Go, SQL Express, 64-bit ODBC, and submodules.
- Create local/Setup-Database.ps1 — imports KN_online and creates the 64-bit KN_online DSN.
- Create local/Build-Local.ps1 — builds Server.slnx, Client.slnx, and Tests.slnx for x64.
- Create local/Start-Local.ps1 and local/Stop-Local.ps1 — own the local service lifecycle and PID/log files.
- Create local/README.md — exact installation, build, launch, login, and verification procedure.
- Modify .gitignore — ignores local runtime state.

### Bot model and integration

- Modify src/Server/Ebenezer/Define.h and src/Server/AIServer/Define.h — separates real sockets from the total entity band.
- Modify src/Server/Ebenezer/EbenezerApp.h/.cpp and User.cpp — owns BotManager and resolves/attacks real or bot users.
- Create BotTypes.h, BotUser.h/.cpp, BotRegistry.h/.cpp, BotTargetSelector.h/.cpp, BotBrain.h/.cpp, BotMovement.h/.cpp, BotCommandFacade.h/.cpp, and BotManager.h/.cpp under src/Server/Ebenezer.
- Modify OperationMessage.h/.cpp — adds +bot_add, +bot_remove_all, +bot_start_pk, and +bot_status.
- Modify production CMake and Visual Studio project/filter files for every new source.

### Tests

- Modify tests/Server/Ebenezer/TestApp.h.
- Create BotCapacity_test.cpp, BotRegistry_test.cpp, BotUserIntegration_test.cpp, BotTargetSelector_test.cpp, BotBrain_test.cpp, BotManager_test.cpp, and BotOperationMessage_test.cpp.
- Modify test CMake and Visual Studio project/filter files for every new test.

---

### Task 1: Provision the local OpenKO baseline

**Files:**
- Create: local/Test-Prerequisites.ps1
- Create: local/Setup-Database.ps1
- Create: local/Build-Local.ps1
- Create: local/README.md
- Modify: .gitignore

**Interfaces:**
- Consumes: official Visual Studio 2022, SQL Server Express 2022, 64-bit ODBC, Go 1.24+, and repository submodules.
- Produces: prerequisite exit code 0, a KN_online DSN/database, populated assets/Client, and clean unmodified Debug-x64 builds/tests.

- [ ] **Step 1: Add ignored local runtime paths**

Append:

~~~gitignore
local/.env.local
local/logs/
local/runtime/
local/pids.json
~~~

- [ ] **Step 2: Write the prerequisite checker**

Create local/Test-Prerequisites.ps1:

~~~powershell
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()
$repoRoot = Split-Path -Parent $PSScriptRoot
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere)) {
    $failures.Add('Visual Studio Installer/vswhere is missing.')
} else {
    $msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) { $failures.Add('MSBuild 17.x is missing.') }
    foreach ($component in @('Microsoft.VisualStudio.Workload.VCTools','Microsoft.VisualStudio.Component.VC.ATL','Microsoft.VisualStudio.Component.VC.ATLMFC')) {
        $match = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires $component -property installationPath
        if (-not $match) { $failures.Add(('Visual Studio component missing: {0}' -f $component)) }
    }
}

if (-not (Get-Command go -ErrorAction SilentlyContinue)) {
    $failures.Add('Go 1.24+ is missing.')
} elseif ((go version) -notmatch 'go1\.(2[4-9]|[3-9][0-9])') {
    $failures.Add(('Go is too old: {0}' -f (go version)))
}

$sql = Get-Service -Name 'MSSQL$SQLEXPRESS' -ErrorAction SilentlyContinue
if ($null -eq $sql) { $failures.Add('SQL Server Express instance SQLEXPRESS is missing.') }

$dsn = Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue
if ($null -eq $dsn) { $failures.Add('64-bit user DSN KN_online is missing.') }

foreach ($relative in @('assets\Client\Server.ini.default','deps\googletest\CMakeLists.txt','deps\db-models\CMakeLists.txt')) {
    if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $relative))) {
        $failures.Add(('Submodule content missing: {0}' -f $relative))
    }
}

if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host 'OpenKO prerequisites: OK'
exit 0
~~~

- [ ] **Step 3: Verify the known failing baseline**

Run:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Test-Prerequisites.ps1
~~~

Expected before installation: exit 1 listing MSBuild, Go, SQL Express, ODBC DSN, and uninitialized submodule content. Do not start bot work while any item remains.

- [ ] **Step 4: Install the official prerequisites**

Download Visual Studio Build Tools from https://aka.ms/vs/17/release/vs_BuildTools.exe and run:

~~~powershell
.\vs_BuildTools.exe --quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.ATL --add Microsoft.VisualStudio.Component.VC.ATLMFC --includeRecommended
~~~

Install SQL Server 2022 Express as SQLEXPRESS with Database Engine Services, TCP/IP enabled, and the current Windows user as administrator. Install Microsoft 64-bit ODBC Driver 18 for SQL Server. Install Go 1.24 or newer from https://go.dev/dl/.

Expected: MSSQL$SQLEXPRESS is Running, vswhere resolves MSBuild 17.x, and go version reports 1.24+.

- [ ] **Step 5: Initialize repository dependencies**

Run from a normal user PowerShell:

~~~powershell
cmd /c .\build_scripts\sync_submodules.cmd Debug x64
git submodule update --init --recursive
~~~

Expected: assets\Client\Server.ini.default, deps\googletest\CMakeLists.txt, and deps\db-models\CMakeLists.txt exist.

- [ ] **Step 6: Write the database setup script**

Create local/Setup-Database.ps1:

~~~powershell
[CmdletBinding()]
param([Parameter(Mandatory)][string] $GameDbPassword)

$ErrorActionPreference = 'Stop'
$runtime = Join-Path $PSScriptRoot 'runtime'
$util = Join-Path $runtime 'kodb-util'
New-Item -ItemType Directory -Force -Path $runtime | Out-Null
if (-not (Test-Path -LiteralPath (Join-Path $util '.git'))) {
    git clone https://github.com/Open-KO/kodb-util.git $util
}
git -C $util submodule update --init --recursive

$configPath = Join-Path $util 'kodb-util-config.yaml'
$yaml = @"
databaseConfig:
  host: localhost
  instance: SQLEXPRESS
  port: 1433
genConfig:
  schemaDir: ./OpenKO-db
  gameDb:
    - name: KN_online
      isForbidClean: false
      isForbidImport: false
      isForbidExport: true
      schemas:
        - knight
      logins:
        - name: knight
          pass: "$GameDbPassword"
      users:
        - name: knight
          schema: knight
"@
Set-Content -LiteralPath $configPath -Value $yaml -Encoding UTF8

Push-Location $util
try {
    go mod download
    go run kodb-util.go -clean -import
} finally {
    Pop-Location
}

Get-OdbcDsn -Name 'KN_online' -DsnType User -Platform '64-bit' -ErrorAction SilentlyContinue | Remove-OdbcDsn
Add-OdbcDsn -Name 'KN_online' -DriverName 'ODBC Driver 18 for SQL Server' -DsnType User -Platform '64-bit' -SetPropertyValue @('Server=.\SQLEXPRESS','Database=KN_online','Trusted_Connection=No','Encrypt=Optional','AutoTranslate=No')
Write-Host 'KN_online import and 64-bit DSN setup: OK'
~~~

Run:

~~~powershell
$gamePassword = Read-Host 'Local knight SQL password'
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Setup-Database.ps1 -GameDbPassword $gamePassword
~~~

Expected: kodb-util exits 0 and Get-OdbcDsn returns KN_online with AutoTranslate=No.

- [ ] **Step 7: Write and run the baseline build script**

Create local/Build-Local.ps1:

~~~powershell
[CmdletBinding()]
param([ValidateSet('Debug','Release')][string] $Configuration = 'Debug')

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { throw 'MSBuild 17.x not found.' }

Push-Location $repoRoot
try {
    foreach ($solution in @('Server.slnx','Client.slnx','Tests.slnx')) {
        & $msbuild /m /p:Configuration=$Configuration /p:Platform=x64 $solution
        if ($LASTEXITCODE -ne 0) { throw ('Build failed: {0}' -f $solution) }
    }
} finally {
    Pop-Location
}
~~~

Run:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
~~~

Expected: all unmodified projects build and the existing Ebenezer tests pass before bot code is introduced.

- [ ] **Step 8: Document and commit the reproducible baseline**

local/README.md must record prerequisites, import command, AutoTranslate=No, x64 build command, generated INI filenames, localhost-only scope, and testing/testing development login.

Commit:

~~~powershell
git add .gitignore local\Test-Prerequisites.ps1 local\Setup-Database.ps1 local\Build-Local.ps1 local\README.md
git commit -m "build: add reproducible local OpenKO setup"
~~~

---

### Task 2: Separate socket capacity and add the bot registry

**Files:**
- Modify: src/Server/Ebenezer/Define.h:11
- Modify: src/Server/Ebenezer/EbenezerApp.cpp:271
- Modify: src/Server/AIServer/Define.h:23
- Modify: tests/Server/Ebenezer/TestApp.h:21
- Create: src/Server/Ebenezer/BotRegistry.h
- Create: src/Server/Ebenezer/BotRegistry.cpp
- Create: tests/Server/Ebenezer/BotCapacity_test.cpp
- Create: tests/Server/Ebenezer/BotRegistry_test.cpp
- Modify: production and test CMake/MSBuild project/filter files

**Interfaces:**
- Produces: MAX_SOCKET_USER=3000, MAX_BOT_USER=500, BOT_USER_ID_MIN=3000, BOT_USER_ID_MAX=3499, MAX_USER=3500.
- Produces: BotRegistry::Register, Get, Remove, Snapshot, Size, and Clear.
- Consumes later: CUser through a forward declaration; Register assigns the CUser ID atomically.

- [ ] **Step 1: Write failing capacity and registry tests**

BotCapacity_test.cpp:

~~~cpp
#include <gtest/gtest.h>
#include <Ebenezer/Define.h>

TEST(BotCapacityTest, KeepsSocketsAndBotsInDisjointBands)
{
    EXPECT_EQ(Ebenezer::MAX_SOCKET_USER, 3000);
    EXPECT_EQ(Ebenezer::BOT_USER_ID_MIN, 3000);
    EXPECT_EQ(Ebenezer::BOT_USER_ID_MAX, 3499);
    EXPECT_EQ(Ebenezer::MAX_USER, 3500);
    EXPECT_LT(Ebenezer::BOT_USER_ID_MAX, Ebenezer::NPC_BAND);
}
~~~

BotRegistry_test.cpp must use a concrete TestBotUser derived from CUser(test_tag {}) and cover: first allocation is 3000; the 500th allocation is 3499; the 501st returns -1; Get returns the same shared pointer; Remove returns the removed pointer; the removed lowest ID is reused; Snapshot remains valid after registry mutation; Clear makes Size zero.

- [ ] **Step 2: Run focused tests and verify failure**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotCapacityTest.*:BotRegistryTest.*
~~~

Expected: compile failure because the capacity constants and BotRegistry do not exist.

- [ ] **Step 3: Implement the capacity split**

Replace Ebenezer MAX_USER with:

~~~cpp
inline constexpr int MAX_SOCKET_USER = 3000;
inline constexpr int MAX_BOT_USER    = 500;
inline constexpr int BOT_USER_ID_MIN = MAX_SOCKET_USER;
inline constexpr int MAX_USER        = MAX_SOCKET_USER + MAX_BOT_USER;
inline constexpr int BOT_USER_ID_MAX = MAX_USER - 1;
~~~

Change EbenezerApp socket initialization and TestApp default initialization to MAX_SOCKET_USER. Set AIServer MAX_USER to 3500. Do not change Aujard or VersionManager MAX_USER because bots never open login or persistence sessions.

- [ ] **Step 4: Implement BotRegistry**

BotRegistry.h exposes:

~~~cpp
class BotRegistry
{
public:
    int Register(const std::shared_ptr<CUser>& bot);
    std::shared_ptr<CUser> Get(int userId) const;
    std::shared_ptr<CUser> Remove(int userId);
    std::vector<std::shared_ptr<CUser>> Snapshot() const;
    size_t Size() const;
    void Clear();

private:
    mutable std::shared_mutex _mutex;
    std::map<int, std::shared_ptr<CUser>> _users;
};
~~~

Register holds a unique lock, scans inclusively from BOT_USER_ID_MIN through BOT_USER_ID_MAX, assigns the first free ID using SetSocketID, inserts it, and returns the ID. It returns -1 for null or exhaustion. Get/Snapshot/Size take shared locks. Remove/Clear take unique locks.

- [ ] **Step 5: Add files to both build systems and rerun**

Add BotRegistry.cpp/.h to Ebenezer.Core in CMake and the MSBuild project/filter files. Add both tests to the test CMake and MSBuild project/filter files.

Run the focused command from Step 2.

Expected: every BotCapacityTest and BotRegistryTest passes.

- [ ] **Step 6: Run all existing tests and commit**

Run:

~~~powershell
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
~~~

Expected: all pre-existing and new tests pass.

Commit:

~~~powershell
git add src\Server\Ebenezer\Define.h src\Server\Ebenezer\EbenezerApp.cpp src\Server\AIServer\Define.h src\Server\Ebenezer\BotRegistry.h src\Server\Ebenezer\BotRegistry.cpp src\Server\Ebenezer\CMakeLists.txt src\Server\Ebenezer\Ebenezer.Core.vcxproj src\Server\Ebenezer\Ebenezer.Core.vcxproj.filters tests\Server\Ebenezer
git commit -m "feat: reserve server-side bot user IDs"
~~~

---

### Task 3: Add socketless bot users and unified player lookup

**Files:**
- Create: src/Server/Ebenezer/BotTypes.h
- Create: src/Server/Ebenezer/BotUser.h
- Create: src/Server/Ebenezer/BotUser.cpp
- Modify: src/Server/Ebenezer/EbenezerApp.h:201-218
- Modify: src/Server/Ebenezer/EbenezerApp.cpp
- Modify: src/Server/Ebenezer/User.cpp:1721-1730
- Create: tests/Server/Ebenezer/BotUserIntegration_test.cpp
- Modify: production and test CMake/MSBuild project/filter files

**Interfaces:**
- Produces: BotSpawnPoint, BotSpawnRequest, BotRuntime, and BotState.
- Produces: CBotUser::InitializeBot, Runtime, and Send.
- Produces: EbenezerApp::GetUserPtr(int), GetBotRegistry(), and IsValidUserId(int).
- Consumes: BotRegistry from Task 2.

- [ ] **Step 1: Write failing socketless-user tests**

The test fixture constructs TestApp, creates ZONE_FRONTIER, constructs CBotUser, initializes a Karus warrior spawn, registers it only after initialization, and asserts:

~~~cpp
EXPECT_EQ(bot->Send(buffer, sizeof(buffer)), sizeof(buffer));
EXPECT_EQ(bot->GetState(), CONNECTION_STATE_GAMESTART);
EXPECT_EQ(app->GetUserPtr(botId).get(), bot.get());
EXPECT_TRUE(app->IsValidUserId(botId));
EXPECT_EQ(app->GetUserSocketCount(), MAX_SOCKET_USER);
EXPECT_EQ(bot->m_pUserData->m_bNation, NATION_KARUS);
EXPECT_EQ(bot->m_pUserData->m_sClass, CLASS_KA_WARRIOR);
~~~

Add a region lifecycle case: UserInOut(USER_IN) adds the ID to the region; UserInOut(USER_OUT) removes it.

- [ ] **Step 2: Run focused tests and verify compile failure**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotUserIntegrationTest.*
~~~

Expected: compile failure because BotTypes, CBotUser, and unified lookup are absent.

- [ ] **Step 3: Define bot types**

BotTypes.h:

~~~cpp
enum class BotState : uint8_t
{
    Spawn,
    Patrol,
    SelectTarget,
    Approach,
    BasicAttack,
    Dead
};

struct BotSpawnPoint
{
    uint8_t zoneId = ZONE_FRONTIER;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct BotSpawnRequest
{
    std::string name;
    uint8_t nation = NATION_KARUS;
    e_Class characterClass = CLASS_KA_WARRIOR;
    uint8_t level = 60;
    BotSpawnPoint spawn;
};

struct BotRuntime
{
    BotState state = BotState::Spawn;
    int targetId = -1;
    BotSpawnPoint home;
    std::chrono::steady_clock::time_point nextAttackAt {};
    std::chrono::steady_clock::time_point respawnAt {};
    size_t patrolIndex = 0;
};
~~~

- [ ] **Step 4: Implement CBotUser**

CBotUser owns one zero-initialized _USER_DATA:

~~~cpp
class CBotUser final : public CUser
{
public:
    CBotUser();
    bool InitializeBot(const BotSpawnRequest& request);
    int Send(char* pBuf, int length) override;
    BotRuntime& Runtime();
    const BotRuntime& Runtime() const;

private:
    _USER_DATA _userData {};
    BotRuntime _runtime {};
};
~~~

InitializeBot calls CUser::Initialize, validates name/nation/class/level/zone position, safely copies the name, sets authority user, zone/coordinates/class/level, and game-start state. MVP values are max HP 1500, max MP 500, total hit 180, total AC 120, hit/evasion rate 1.0, speed 45, loyalty 100, and gold 0. Send returns length and never touches a socket.

- [ ] **Step 5: Integrate unified lookup**

EbenezerApp owns BotRegistry and moves integer lookup out of the header:

~~~cpp
std::shared_ptr<CUser> EbenezerApp::GetUserPtr(int userId) const
{
    if (_serverSocketManager.IsValidSocketId(userId))
        return _serverSocketManager.GetUser(userId);
    return _botRegistry.Get(userId);
}

bool EbenezerApp::IsValidUserId(int userId) const
{
    return GetUserPtr(userId) != nullptr;
}
~~~

Keep GetUserPtrUnchecked and every GetUserSocketCount loop real-socket-only.

- [ ] **Step 6: Remove the socket-only attack gate**

In CUser::Attack, remove _socketManager->IsValidSocketId(tid). Resolve once:

~~~cpp
pTUser = m_pMain->GetUserPtr(tid);
if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD
    || pTUser->m_bAbnormalType == ABNORMAL_BLINKING
    || pTUser->m_pUserData->m_bNation == m_pUserData->m_bNation)
{
    result = 0x00;
}
~~~

Add two cases: real TestUser attacks a bot ID; CBotUser attacks a real TestUser without dereferencing a null socket manager.

- [ ] **Step 7: Build, test, and commit**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
~~~

Expected: all tests pass with no invalid access during both attack directions.

Commit:

~~~powershell
git add src\Server\Ebenezer tests\Server\Ebenezer
git commit -m "feat: add socketless bot users"
~~~

---

### Task 4: Implement nearest-enemy selection and deterministic brain decisions

**Files:**
- Create: src/Server/Ebenezer/BotTargetSelector.h
- Create: src/Server/Ebenezer/BotTargetSelector.cpp
- Create: src/Server/Ebenezer/BotBrain.h
- Create: src/Server/Ebenezer/BotBrain.cpp
- Create: tests/Server/Ebenezer/BotTargetSelector_test.cpp
- Create: tests/Server/Ebenezer/BotBrain_test.cpp
- Modify: production and test CMake/MSBuild project/filter files

**Interfaces:**
- Produces: BotTargetSelector::SelectNearestEnemy(const CBotUser&) returning int.
- Produces: BotPerception, BotIntentType, BotIntent, and BotBrain::Decide.
- Consumes: unified GetUserPtr and BotRuntime.

- [ ] **Step 1: Write failing target-selector tests**

Create users in the same and neighboring regions:

~~~cpp
EXPECT_EQ(selector.SelectNearestEnemy(*karusBot), nearestElMoradId);
~~~

Separate cases prove it ignores self, same nation, zero HP, USER_DEAD, another zone, and candidates outside the 3x3 region neighborhood. Equal distance selects the lower ID.

- [ ] **Step 2: Write failing brain tests with a fixed clock**

~~~cpp
EXPECT_EQ(brain.Decide(runtime, noTarget, now, 2.5f).type, BotIntentType::SelectTarget);
EXPECT_EQ(brain.Decide(runtime, farTarget, now, 2.5f).type, BotIntentType::Approach);
EXPECT_EQ(brain.Decide(runtime, nearTarget, now, 2.5f).type, BotIntentType::BasicAttack);
EXPECT_EQ(brain.Decide(deadRuntime, noTarget, nowBeforeRespawn, 2.5f).type, BotIntentType::Wait);
EXPECT_EQ(brain.Decide(deadRuntime, noTarget, respawnTime, 2.5f).type, BotIntentType::Respawn);
~~~

- [ ] **Step 3: Run focused tests and verify failure**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotTargetSelectorTest.*:BotBrainTest.*
~~~

Expected: compile failure because selector/brain types do not exist.

- [ ] **Step 4: Implement the selector**

BotTargetSelector stores an EbenezerApp reference. It locks g_region_mutex, scans x/z offsets -1 through +1 within map bounds, copies candidate IDs, unlocks, then resolves candidates through GetUserPtr. Filter by different ID, same zone, different nation, game-start state, nonzero HP, and non-dead state. Compare GetDistanceSquared2D and use ID as the tie-breaker.

~~~cpp
class BotTargetSelector
{
public:
    explicit BotTargetSelector(EbenezerApp& app);
    int SelectNearestEnemy(const CBotUser& source) const;

private:
    EbenezerApp& _app;
};
~~~

- [ ] **Step 5: Implement the pure brain**

~~~cpp
enum class BotIntentType : uint8_t
{
    Wait,
    Patrol,
    SelectTarget,
    Approach,
    BasicAttack,
    Respawn
};

struct BotPerception
{
    bool alive = true;
    bool targetValid = false;
    float targetDistance = 0.0f;
};

struct BotIntent
{
    BotIntentType type = BotIntentType::Wait;
    int targetId = -1;
};

class BotBrain
{
public:
    BotIntent Decide(const BotRuntime& runtime, const BotPerception& perception,
        std::chrono::steady_clock::time_point now, float attackRange) const;
};
~~~

Decide is side-effect-free. Dead waits until respawnAt, then Respawn. Missing target requests SelectTarget. A valid far target requests Approach. A valid near target requests BasicAttack. Patrol is used after selector reports no enemy.

- [ ] **Step 6: Build, test, and commit**

Run the focused tests, then the full test executable. Expected: all pass.

Commit:

~~~powershell
git add src\Server\Ebenezer tests\Server\Ebenezer
git commit -m "feat: add deterministic bot targeting"
~~~

---

### Task 5: Drive movement, attacks, death, and respawn on a 200 ms manager tick

**Files:**
- Create: src/Server/Ebenezer/BotMovement.h
- Create: src/Server/Ebenezer/BotMovement.cpp
- Create: src/Server/Ebenezer/BotCommandFacade.h
- Create: src/Server/Ebenezer/BotCommandFacade.cpp
- Create: src/Server/Ebenezer/BotManager.h
- Create: src/Server/Ebenezer/BotManager.cpp
- Modify: src/Server/Ebenezer/EbenezerApp.h/.cpp
- Create: tests/Server/Ebenezer/BotManager_test.cpp
- Modify: production and test CMake/MSBuild project/filter files

**Interfaces:**
- Produces: BotMovement::NextStep and Move.
- Produces: BotCommandFacade::Approach, BasicAttack, Patrol, Respawn, and Despawn.
- Produces: BotManager::Spawn, RemoveAll, StartPk, Stop, Tick, Status, and FindUser.
- Consumes: registry, selector, brain, CUser::MoveProcess, CUser::Attack, CUser::UserInOut, and TimerThread.

- [ ] **Step 1: Write failing manager tests**

Use a fixed clock passed to Tick(now). Add cases proving:

- Spawn registers a bot and USER_IN adds its ID to the map region.
- A bot with no enemy patrols without leaving map bounds.
- A far enemy causes movement and decreases distance.
- An in-range enemy loses HP through CUser::Attack.
- HP=0 changes state to Dead and sets respawnAt to now+15s.
- At 14.999s the bot stays dead; at 15s it respawns at home with max HP.
- RemoveAll emits USER_OUT, removes region IDs, and leaves registry size zero.
- One invalid bot does not prevent another bot from ticking.

- [ ] **Step 2: Run focused tests and verify failure**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotManagerTest.*
~~~

Expected: compile failure because manager, movement, and facade classes do not exist.

- [ ] **Step 3: Implement validated movement**

BotMovement::NextStep computes a normalized 2D step no larger than 1.5 world units. Move validates C3DMap::IsValidPosition, serializes coordinates multiplied by ten, speed 45, echo 0, and calls CUser::MoveProcess.

~~~cpp
class BotMovement
{
public:
    static BotSpawnPoint NextStep(const CBotUser& source, float targetX, float targetZ,
        float maxStep);
    static bool Move(CBotUser& source, const BotSpawnPoint& destination, int16_t speed);
};
~~~

Patrol uses four deterministic offsets around home: (+10,0), (0,+10), (-10,0), and (0,-10). Invalid positions are skipped.

- [ ] **Step 4: Implement authoritative basic attacks**

BasicAttack re-resolves the target, checks zone, nation, alive state, and distance, then enforces nextAttackAt with a one-second interval. It creates the existing payload and calls source.Attack:

~~~cpp
char attack[16] {};
int index = 0;
SetByte(attack, DIRECT_ATTACK, index);
SetByte(attack, 1, index);
SetShort(attack, targetId, index);
SetShort(attack, 100, index);
SetShort(attack, static_cast<int16_t>(distance * 10.0f), index);
source.Attack(attack);
source.Runtime().nextAttackAt = now + std::chrono::seconds(1);
~~~

The facade rejects distances above 2.5 before calling the legacy Attack path, so an empty-handed bot cannot exploit the legacy client-supplied distance field.

- [ ] **Step 5: Implement respawn and despawn**

Respawn calls USER_OUT when registered, restores home coordinates and max HP/MP, sets USER_STANDING and ABNORMAL_NORMAL, recomputes region indices, calls USER_REGENE, and clears target/death timestamps. Despawn calls USER_OUT before registry removal. Neither method calls Aujard or writes SQL.

- [ ] **Step 6: Implement BotManager**

~~~cpp
struct BotStatus
{
    size_t total = 0;
    size_t alive = 0;
    size_t dead = 0;
    bool running = false;
};

class BotManager
{
public:
    explicit BotManager(EbenezerApp& app);
    ~BotManager();
    int Spawn(const BotSpawnRequest& request);
    size_t RemoveAll();
    void StartPk();
    void Stop();
    void Tick(std::chrono::steady_clock::time_point now);
    BotStatus Status() const;
    std::shared_ptr<CUser> FindUser(int userId) const;

private:
    void TickBot(const std::shared_ptr<CBotUser>& bot,
        std::chrono::steady_clock::time_point now);
};
~~~

Production StartPk creates one TimerThread with 200ms and calls Tick(steady_clock::now()). Tests call Tick directly and never sleep. TickBot catches std::exception per bot, logs ID/state, calls Despawn for that bot, and continues the snapshot.

BotRegistry deliberately stores `shared_ptr<CUser>` so unified lookup remains generic. During each Tick, dynamically cast every snapshot entry to CBotUser before calling TickBot. If an entry is not a CBotUser, log the invalid registry entry, remove it safely, and continue:

~~~cpp
for (const auto& entry : registry.Snapshot())
{
    auto bot = std::dynamic_pointer_cast<CBotUser>(entry);
    if (bot == nullptr)
    {
        registry.Remove(entry->GetSocketID());
        continue;
    }
    TickBot(bot, now);
}
~~~

- [ ] **Step 7: Wire lifecycle order into EbenezerApp**

Construct BotManager after the app singleton is ready. Stop it at the beginning of EbenezerApp destruction before region, maps, and socket managers are released. Do not auto-spawn until maps, HOME data, and AIServer connectivity succeed.

- [ ] **Step 8: Build, test, and commit**

Run focused and full tests. Expected: deterministic pass and no stale region IDs after RemoveAll.

Commit:

~~~powershell
git add src\Server\Ebenezer tests\Server\Ebenezer
git commit -m "feat: run server-side bot combat loop"
~~~

---

### Task 6: Add configuration and administrator commands

**Files:**
- Modify: src/Server/Ebenezer/EbenezerApp.h/.cpp
- Modify: src/Server/Ebenezer/OperationMessage.h/.cpp
- Modify: src/Server/Ebenezer/BotTypes.h
- Modify: src/Server/Ebenezer/BotManager.h/.cpp
- Create: tests/Server/Ebenezer/BotOperationMessage_test.cpp
- Modify: test build file lists

**Interfaces:**
- Produces: BotConfig parsing and validation.
- Produces: +bot_add nation class count, +bot_remove_all, +bot_start_pk, and +bot_status.
- Consumes: BotManager public surface from Task 5.

- [ ] **Step 1: Write failing config and command tests**

~~~cpp
EXPECT_TRUE(op.Process("+bot_add karus warrior 5"));
EXPECT_TRUE(op.Process("+bot_add elmorad priest 2"));
EXPECT_EQ(app.GetBotManager().Status().total, 7);
EXPECT_TRUE(op.Process("+bot_remove_all"));
EXPECT_EQ(app.GetBotManager().Status().total, 0);
EXPECT_TRUE(op.Process("+bot_start_pk"));
EXPECT_TRUE(app.GetBotManager().Status().running);
EXPECT_TRUE(op.Process("+bot_status"));
~~~

Add invalid cases for nation, class, count 0, count 501, exhausted capacity, missing zone, and invalid spawn position. Every failed batch must leave the previous registry size unchanged. Preserve the existing Chat authority gate so non-managers cannot invoke these commands.

- [ ] **Step 2: Run focused tests and verify failure**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotOperationMessageTest.*
~~~

Expected: commands are unhandled.

- [ ] **Step 3: Parse and validate BOTS configuration**

~~~cpp
struct BotConfig
{
    bool enabled = false;
    uint16_t count = 10;
    uint16_t tickMilliseconds = 200;
    uint16_t respawnSeconds = 15;
    uint8_t zoneId = ZONE_FRONTIER;
    float attackRange = 2.5f;
    float moveStep = 1.5f;
};
~~~

Load [BOTS] Enabled, Count, TickMilliseconds, RespawnSeconds, Zone, AttackRange, and MoveStep. Valid ranges: Count 0–500, TickMilliseconds exactly 200 for the milestone, RespawnSeconds exactly 15, AttackRange 0.5–10.0, MoveStep 0.1–5.0, and an existing map zone. Invalid bot config logs one error, sets enabled=false, and does not fail ordinary Ebenezer LoadConfig.

- [ ] **Step 4: Implement class/nation parsing**

~~~cpp
e_Class ResolveBotClass(uint8_t nation, std::string_view token)
{
    if (nation == NATION_KARUS)
    {
        if (token == "warrior") return CLASS_KA_WARRIOR;
        if (token == "rogue") return CLASS_KA_ROGUE;
        if (token == "mage") return CLASS_KA_WIZARD;
        if (token == "priest") return CLASS_KA_PRIEST;
    }
    if (nation == NATION_ELMORAD)
    {
        if (token == "warrior") return CLASS_EL_WARRIOR;
        if (token == "rogue") return CLASS_EL_ROGUE;
        if (token == "mage") return CLASS_EL_WIZARD;
        if (token == "priest") return CLASS_EL_PRIEST;
    }
    return CLASS_UNKNOWN;
}
~~~

Names are deterministic Bot_K_000 through Bot_K_499 and Bot_E_000 through Bot_E_499. Spawn positions use the nation HOME record FreeZoneX/FreeZoneZ plus bounded offsets and must pass map validation. If any member of one +bot_add batch fails, remove every bot created by that command.

- [ ] **Step 5: Add OperationMessage cases**

Add switch cases for the four commands and protected methods BotAdd, BotRemoveAll, BotStartPk, and BotStatus. Each method logs invoking GM/server, normalized arguments, requested count, result count, and success. BotStatus logs total/alive/dead/running.

- [ ] **Step 6: Auto-create the approved roster after successful startup**

When Enabled=1 and Count=10, create five Karus warriors and five El Morad warriors after MapFileLoad and AIServerConnect succeed, then start the timer. Any spawn failure removes the complete auto-roster and disables only BotManager.

- [ ] **Step 7: Build, test, and commit**

Run:

~~~powershell
& $msbuild /m /p:Configuration=Debug /p:Platform=x64 Tests.slnx
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
~~~

Expected: all tests pass and invalid BOTS config leaves ordinary server behavior intact.

Commit:

~~~powershell
git add src\Server\Ebenezer tests\Server\Ebenezer
git commit -m "feat: add bot configuration and GM commands"
~~~

---

### Task 7: Add safe launch scripts and perform the complete acceptance run

**Files:**
- Create: local/Start-Local.ps1
- Create: local/Stop-Local.ps1
- Modify: local/README.md
- Modify: docs/superpowers/specs/2026-07-15-openko-server-bots-design.md

**Interfaces:**
- Produces: one-command localhost launch/stop with owned PID tracking.
- Produces: final evidence for client login, ten-bot visibility/combat/respawn, and 30-minute stability.
- Consumes: all preceding build, database, server, and bot interfaces.

- [ ] **Step 1: Write Start-Local.ps1 with process ownership**

The script must require the prerequisite checker to pass, require MSSQL$SQLEXPRESS Running, require Debug-x64 executables, reject non-loopback server addresses, start Aujard, ItemManager, VersionManager, AIServer, then Ebenezer, save only created process IDs, and poll readiness for at most 60 seconds.

~~~powershell
function Start-OwnedProcess {
    param(
        [Parameter(Mandatory)][string] $Name,
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][string] $WorkingDirectory
    )
    $stdout = Join-Path $logDir ($Name + '.out.log')
    $stderr = Join-Path $logDir ($Name + '.err.log')
    $process = Start-Process -FilePath $Path -WorkingDirectory $WorkingDirectory -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    if ($process.HasExited) { throw ('{0} exited during startup.' -f $Name) }
    [pscustomobject]@{ Name = $Name; Id = $process.Id }
}
~~~

Copy assets\Client\Server.ini.default to ignored assets\Client\Server.ini when absent, set its server address to 127.0.0.1, and start WarFare only after VersionManager and Ebenezer are ready.

- [ ] **Step 2: Write Stop-Local.ps1**

Read local/pids.json, verify each PID still belongs to the recorded executable name, stop only those owned PIDs, wait up to ten seconds, and remove pids.json. Never kill unrelated processes by enumerated name.

- [ ] **Step 3: Generate and verify gameserver.ini**

The ignored file must contain:

~~~ini
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
~~~

Start-Local.ps1 reads GAME_DB_PASSWORD from ignored local/.env.local and writes the actual GAME_PWD line at runtime. Verify git status never shows INI files, client assets, runtime files, logs, or PID state.

- [ ] **Step 4: Run Debug-x64 automated verification**

Run:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
git diff --check
git status --short
~~~

Expected: build and tests pass; diff check is clean; status contains only intended source/script/doc changes.

- [ ] **Step 5: Perform the manual acceptance scenario**

Run Start-Local.ps1, log in with testing/testing, enter zone 201, and verify:

- Exactly ten bots are visible: five Bot_K_* and five Bot_E_*.
- Movement is visible to the real client.
- Bots select the opposing nation, approach, and use basic attacks.
- The real player can target, damage, and kill a bot.
- A bot can damage and kill a real player under ordinary CUser rules.
- Dead bots remain dead for 15 seconds and respawn at their nation home point.
- +bot_status reports total=10 and consistent alive+dead counts.
- +bot_remove_all removes every bot without a ghost model.
- Two +bot_add commands restore 5+5 bots.
- +bot_start_pk resumes combat.

- [ ] **Step 6: Run the 30-minute stability check**

Keep client and services running for 30 minutes. Every minute record total/alive/dead, process liveness, and registry size in ignored local/logs/stability.csv. At completion assert:

- no process exited;
- no bot ID is outside 3000–3499;
- no duplicate bot ID exists;
- every region bot ID resolves through GetUserPtr;
- alive+dead equals total;
- after RemoveAll no region contains a bot ID.

Stop services with Stop-Local.ps1.

- [ ] **Step 7: Build Release-x64 and commit operations support**

Run:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Release
.\bin\Release-x64\Ebenezer.Tests\Ebenezer.Tests.exe
~~~

Expected: Release build and tests pass.

Commit:

~~~powershell
git add local\Start-Local.ps1 local\Stop-Local.ps1 local\README.md docs\superpowers\specs\2026-07-15-openko-server-bots-design.md
git commit -m "docs: add local bot server operations"
~~~

---

## Final Verification Gate

Run from C:\Users\hpete\Desktop\KO1453\openko-1299:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Test-Prerequisites.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
powershell -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Release
.\bin\Release-x64\Ebenezer.Tests\Ebenezer.Tests.exe
git diff --check
git status --short --branch
~~~

Required result: every command succeeds, the worktree is clean after the final commit, the branch is only locally ahead of origin, and the manual plus 30-minute acceptance evidence satisfies the design specification.
