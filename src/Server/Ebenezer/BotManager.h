#ifndef SERVER_EBENEZER_BOTMANAGER_H
#define SERVER_EBENEZER_BOTMANAGER_H

#pragma once

#include "BotBrain.h"
#include "BotCommandFacade.h"
#include "BotTargetSelector.h"
#include "BotTypes.h"

#include <chrono>
#include <cstddef>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace Ebenezer
{
class CBotUser;
class CUser;
class EbenezerApp;

struct BotStatus
{
	size_t total = 0;
	size_t alive = 0;
	size_t dead  = 0;
	bool running = false;
};

class IBotTimer
{
public:
	virtual ~IBotTimer() = default;
	virtual void Start() = 0;
	// Postcondition on return: the timer is stopped and every callback has completed/joined.
	virtual void Shutdown() noexcept = 0;
};

using BotTimerFactory = std::function<std::unique_ptr<IBotTimer>(
	std::chrono::milliseconds, std::function<void()>)>;

class BotManager
{
public:
	explicit BotManager(EbenezerApp& app);
	BotManager(EbenezerApp& app, BotTimerFactory timerFactory);
	~BotManager() noexcept;
	int Spawn(const BotSpawnRequest& request);
	std::vector<int> SpawnBatch(const std::vector<BotSpawnRequest>& requests,
		std::optional<size_t> expectedRegistrySize = std::nullopt);
	bool StartConfiguredRoster(
		const std::vector<BotSpawnRequest>& requests, size_t expectedRegistrySize);
	size_t RemoveBatch(const std::vector<int>& userIds);
	size_t RemoveAll();
	void StartPk();
	void Stop() noexcept;
	void Tick(std::chrono::steady_clock::time_point now);
	BotStatus Status() const;
	std::shared_ptr<CUser> FindUser(int userId) const;

private:
	struct OwnedBot
	{
		int userId = -1;
		std::shared_ptr<CBotUser> bot;
	};

	enum class Lifecycle
	{
		Idle,
		Running,
		Stopping,
		Shutdown
	};

	void TickBot(const std::shared_ptr<CBotUser>& bot, std::chrono::steady_clock::time_point now);
	OwnedBot SpawnOwnedUnlocked(const BotSpawnRequest& request);
	std::vector<OwnedBot> SpawnBatchUnlocked(const std::vector<BotSpawnRequest>& requests,
		std::optional<size_t> expectedRegistrySize);
	void RollbackUnlocked(const std::vector<OwnedBot>& bots) noexcept;
	bool StartPkUnlocked();

	EbenezerApp& _app;
	BotTargetSelector _selector;
	BotBrain _brain;
	BotCommandFacade _commands;
	// Lock order inside BotManager: _operationMutex -> _lifecycleMutex. Registry methods take and
	// release their own internal lock; an operation never retains a registry lock while calling
	// region code. Socket-manager snapshots are completed before acquiring _operationMutex.
	mutable std::mutex _operationMutex;
	mutable std::mutex _lifecycleMutex;
	std::condition_variable _lifecycleCv;
	Lifecycle _lifecycle = Lifecycle::Idle;
	BotTimerFactory _timerFactory;
	std::unique_ptr<IBotTimer> _timer;
};
} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTMANAGER_H
