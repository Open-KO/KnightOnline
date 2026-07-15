#include "pch.h"
#include "BotManager.h"

#include "BotUser.h"
#include "EbenezerApp.h"

#include <shared/TimerThread.h>

#include <stdexcept>
#include <unordered_set>
#include <utility>

using namespace std::chrono_literals;

namespace Ebenezer
{
namespace
{
class TimerThreadAdapter final : public IBotTimer
{
public:
	TimerThreadAdapter(std::chrono::milliseconds delay, std::function<void()> callback) :
		_timer(delay, std::move(callback))
	{
	}

	void Start() override
	{
		_timer.start();
	}

	void Shutdown() noexcept override
	{
		// Thread::shutdown() signals and joins by default. If its join ever reports an unexpected
		// system error, noexcept terminates rather than returning with a live callback.
		_timer.shutdown();
	}

private:
	TimerThread _timer;
};

bool IsValidTarget(const CBotUser& source, const std::shared_ptr<CUser>& target)
{
	return source.m_pUserData != nullptr && target != nullptr && target->m_pUserData != nullptr
		   && target->GetState() == CONNECTION_STATE_GAMESTART
		   && target->m_pUserData->m_bZone == source.m_pUserData->m_bZone
		   && target->m_pUserData->m_bNation != source.m_pUserData->m_bNation
		   && target->m_pUserData->m_sHp > 0 && target->m_bResHpType != USER_DEAD;
}
} // namespace

BotManager::BotManager(EbenezerApp& app) :
	BotManager(app,
		[](std::chrono::milliseconds delay, std::function<void()> callback)
		{ return std::make_unique<TimerThreadAdapter>(delay, std::move(callback)); })
{
}

BotManager::BotManager(EbenezerApp& app, BotTimerFactory timerFactory) :
	_app(app), _selector(app), _commands(app), _timerFactory(std::move(timerFactory))
{
	if (!_timerFactory)
		throw std::invalid_argument("BotManager requires a timer factory");
}

BotManager::~BotManager() noexcept
{
	Stop();
	RemoveAll();
}

int BotManager::Spawn(const BotSpawnRequest& request)
{
	std::lock_guard operationLock(_operationMutex);
	return SpawnUnlocked(request);
}

int BotManager::SpawnUnlocked(const BotSpawnRequest& request)
{
	auto bot = std::make_shared<CBotUser>();
	if (!bot->InitializeBot(request))
		return -1;
	const int userId = _app.GetBotRegistry().Register(bot);
	if (userId < 0)
		return -1;
	try
	{
		bot->UserInOut(USER_IN);
	}
	catch (...)
	{
		_commands.Despawn(*bot);
		throw;
	}
	return userId;
}

void BotManager::RollbackUnlocked(const std::vector<int>& userIds) noexcept
{
	for (auto itr = userIds.rbegin(); itr != userIds.rend(); ++itr)
	{
		auto bot = std::dynamic_pointer_cast<CBotUser>(_app.GetBotRegistry().Get(*itr));
		if (bot == nullptr)
			continue;
		try
		{
			_commands.Despawn(*bot);
		}
		catch (...)
		{
			_app.GetBotRegistry().Remove(*itr);
		}
	}
}

std::vector<int> BotManager::SpawnBatch(const std::vector<BotSpawnRequest>& requests,
	std::optional<size_t> expectedRegistrySize)
{
	std::lock_guard operationLock(_operationMutex);
	const size_t registrySize = _app.GetBotRegistry().Size();
	if ((expectedRegistrySize.has_value() && registrySize != *expectedRegistrySize)
		|| requests.empty()
		|| requests.size() > static_cast<size_t>(MAX_BOT_USER) - registrySize)
		return {};

	std::unordered_set<std::string> names;
	for (const auto& entry : _app.GetBotRegistry().Snapshot())
	{
		if (entry != nullptr && entry->m_pUserData != nullptr)
			names.insert(NormalizeBotToken(entry->m_pUserData->m_id));
	}
	for (const auto& request : requests)
	{
		if (request.name.empty() || !names.insert(NormalizeBotToken(request.name)).second)
			return {};
	}

	std::vector<int> created;
	created.reserve(requests.size());
	try
	{
		for (const auto& request : requests)
		{
			const int userId = SpawnUnlocked(request);
			if (userId < 0)
			{
				RollbackUnlocked(created);
				return {};
			}
			created.push_back(userId);
		}
	}
	catch (...)
	{
		RollbackUnlocked(created);
		return {};
	}
	return created;
}

size_t BotManager::RemoveBatch(const std::vector<int>& userIds)
{
	std::lock_guard operationLock(_operationMutex);
	std::unordered_set<int> uniqueIds(userIds.begin(), userIds.end());
	size_t removed = 0;
	for (int userId : uniqueIds)
	{
		auto bot = std::dynamic_pointer_cast<CBotUser>(_app.GetBotRegistry().Get(userId));
		if (bot != nullptr && _commands.Despawn(*bot))
			++removed;
	}
	return removed;
}

size_t BotManager::RemoveAll()
{
	std::lock_guard operationLock(_operationMutex);
	size_t removed = 0;
	for (const auto& entry : _app.GetBotRegistry().Snapshot())
	{
		if (entry == nullptr)
			continue;
		auto bot = std::dynamic_pointer_cast<CBotUser>(entry);
		if (bot != nullptr)
		{
			try
			{
				if (_commands.Despawn(*bot))
					++removed;
			}
			catch (const std::exception& ex)
			{
				spdlog::error("BotManager::RemoveAll: bot {} failed to despawn: {}",
					bot->GetSocketID(), ex.what());
				if (_app.GetBotRegistry().Remove(bot->GetSocketID()) != nullptr)
					++removed;
			}
		}
		else if (_app.GetBotRegistry().Remove(entry->GetSocketID()) != nullptr)
			++removed;
	}
	return removed;
}

void BotManager::StartPk()
{
	std::lock_guard lock(_lifecycleMutex);
	if (_lifecycle != Lifecycle::Idle)
		return;
	auto timer = _timerFactory(std::chrono::milliseconds(_app.GetBotConfig().tickMilliseconds),
		[this]() { Tick(std::chrono::steady_clock::now()); });
	if (timer == nullptr)
		throw std::runtime_error("BotManager timer factory returned null");
	_lifecycle = Lifecycle::Running;
	_timer = std::move(timer);
	try
	{
		_timer->Start();
	}
	catch (...)
	{
		_timer.reset();
		_lifecycle = Lifecycle::Idle;
		throw;
	}
}

void BotManager::Stop() noexcept
{
	std::unique_ptr<IBotTimer> timer;
	{
		std::unique_lock lock(_lifecycleMutex);
		if (_lifecycle == Lifecycle::Shutdown)
			return;
		if (_lifecycle == Lifecycle::Stopping)
		{
			_lifecycleCv.wait(lock, [this]() { return _lifecycle == Lifecycle::Shutdown; });
			return;
		}
		if (_lifecycle == Lifecycle::Idle)
		{
			_lifecycle = Lifecycle::Shutdown;
			_lifecycleCv.notify_all();
			return;
		}
		_lifecycle = Lifecycle::Stopping;
		timer = std::move(_timer);
	}

	if (timer != nullptr)
		timer->Shutdown();
	{
		std::lock_guard lock(_lifecycleMutex);
		_lifecycle = Lifecycle::Shutdown;
	}
	_lifecycleCv.notify_all();
}

void BotManager::Tick(std::chrono::steady_clock::time_point now)
{
	std::lock_guard operationLock(_operationMutex);
	for (const auto& entry : _app.GetBotRegistry().Snapshot())
	{
		if (entry == nullptr)
			continue;
		auto bot = std::dynamic_pointer_cast<CBotUser>(entry);
		if (bot == nullptr)
		{
			spdlog::error("BotManager::Tick: invalid registry entry {}", entry->GetSocketID());
			_app.GetBotRegistry().Remove(entry->GetSocketID());
			continue;
		}
		try
		{
			TickBot(bot, now);
		}
		catch (const std::exception& ex)
		{
			spdlog::error("BotManager::Tick: bot {} state {} failed: {}", bot->GetSocketID(),
				static_cast<int>(bot->Runtime().state), ex.what());
			try
			{
				_commands.Despawn(*bot);
			}
			catch (const std::exception& despawnError)
			{
				spdlog::error("BotManager::Tick: bot {} cleanup failed: {}", bot->GetSocketID(),
					despawnError.what());
				_app.GetBotRegistry().Remove(bot->GetSocketID());
			}
		}
	}
}

void BotManager::TickBot(
	const std::shared_ptr<CBotUser>& bot, std::chrono::steady_clock::time_point now)
{
	if (bot == nullptr || bot->m_pUserData == nullptr)
		throw std::runtime_error("missing bot user data");
	auto& runtime    = bot->Runtime();
	const bool alive = bot->m_pUserData->m_sHp > 0 && bot->m_bResHpType != USER_DEAD;
	if (!alive && runtime.state != BotState::Dead)
	{
		runtime.state        = BotState::Dead;
		runtime.targetId     = -1;
		runtime.nextAttackAt = {};
		runtime.respawnAt = now + std::chrono::seconds(_app.GetBotConfig().respawnSeconds);
	}

	auto target            = _app.GetUserPtr(runtime.targetId);
	const bool targetValid = alive && IsValidTarget(*bot, target);
	BotPerception perception;
	perception.alive       = alive;
	perception.targetValid = targetValid;
	if (targetValid)
		perception.targetDistance = bot->GetDistance2D(
			target->m_pUserData->m_curx, target->m_pUserData->m_curz);

	const BotConfig& config = _app.GetBotConfig();
	const BotIntent intent = _brain.Decide(runtime, perception, now, config.attackRange);
	switch (intent.type)
	{
		case BotIntentType::SelectTarget:
			runtime.targetId = _selector.SelectNearestEnemy(*bot);
			runtime.state    = BotState::SelectTarget;
			break;
		case BotIntentType::Patrol:
			_commands.Patrol(*bot, config.moveStep);
			runtime.state = BotState::Patrol;
			break;
		case BotIntentType::Approach:
			if (_commands.Approach(*bot, runtime.targetId, config.moveStep))
				runtime.state = BotState::Approach;
			else
				runtime.targetId = -1;
			break;
		case BotIntentType::BasicAttack:
			_commands.BasicAttack(*bot, runtime.targetId, now, config.attackRange);
			runtime.state = BotState::BasicAttack;
			break;
		case BotIntentType::Respawn:
			if (!_commands.Respawn(*bot))
				throw std::runtime_error("respawn failed");
			break;
		case BotIntentType::Wait:
		default:
			break;
	}
}

BotStatus BotManager::Status() const
{
	std::lock_guard operationLock(_operationMutex);
	BotStatus status;
	const auto snapshot = _app.GetBotRegistry().Snapshot();
	status.total        = snapshot.size();
	for (const auto& entry : snapshot)
	{
		auto bot = std::dynamic_pointer_cast<CBotUser>(entry);
		if (bot == nullptr || bot->m_pUserData == nullptr)
			continue;
		if (bot->Runtime().state == BotState::Dead || bot->m_bResHpType == USER_DEAD
			|| bot->m_pUserData->m_sHp == 0)
			++status.dead;
		else
			++status.alive;
	}
	{
		std::lock_guard lock(_lifecycleMutex);
		status.running = _lifecycle == Lifecycle::Running;
	}
	return status;
}

std::shared_ptr<CUser> BotManager::FindUser(int userId) const
{
	return _app.GetBotRegistry().Get(userId);
}
} // namespace Ebenezer
