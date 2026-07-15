#ifndef SERVER_EBENEZER_BOTCOMMANDFACADE_H
#define SERVER_EBENEZER_BOTCOMMANDFACADE_H

#pragma once

#include <chrono>

namespace Ebenezer
{
class CBotUser;
class EbenezerApp;

class BotCommandFacade
{
public:
	explicit BotCommandFacade(EbenezerApp& app);
	bool Approach(CBotUser& source, int targetId);
	bool BasicAttack(CBotUser& source, int targetId, std::chrono::steady_clock::time_point now);
	bool Patrol(CBotUser& source);
	bool Respawn(CBotUser& source);
	bool Despawn(CBotUser& source);

private:
	void PurgeRegionEntries(int userId);
	EbenezerApp& _app;
};
} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTCOMMANDFACADE_H
