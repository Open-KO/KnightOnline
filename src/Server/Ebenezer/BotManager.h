#ifndef SERVER_EBENEZER_BOTMANAGER_H
#define SERVER_EBENEZER_BOTMANAGER_H

#pragma once

#include "BotBrain.h"
#include "BotCommandFacade.h"
#include "BotTargetSelector.h"
#include "BotTypes.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>

class TimerThread;

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
	void TickBot(const std::shared_ptr<CBotUser>& bot, std::chrono::steady_clock::time_point now);

	EbenezerApp& _app;
	BotTargetSelector _selector;
	BotBrain _brain;
	BotCommandFacade _commands;
	mutable std::mutex _timerMutex;
	std::unique_ptr<TimerThread> _timer;
};
} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTMANAGER_H
