#include "pch.h"

#include "BotRegistry.h"
#include "Define.h"
#include "User.h"

#include <mutex>

namespace Ebenezer
{

int BotRegistry::Register(const std::shared_ptr<CUser>& bot)
{
	if (bot == nullptr)
		return -1;

	std::unique_lock lock(_mutex);
	for (int userId = BOT_USER_ID_MIN; userId <= BOT_USER_ID_MAX; ++userId)
	{
		if (_users.contains(userId))
			continue;

		bot->SetSocketID(userId);
		_users.emplace(userId, bot);
		return userId;
	}

	return -1;
}

std::shared_ptr<CUser> BotRegistry::Get(int userId) const
{
	std::shared_lock lock(_mutex);
	const auto entry = _users.find(userId);
	return entry == _users.end() ? nullptr : entry->second;
}

std::shared_ptr<CUser> BotRegistry::Remove(int userId)
{
	std::unique_lock lock(_mutex);
	const auto entry = _users.find(userId);
	if (entry == _users.end())
		return nullptr;

	auto bot = entry->second;
	_users.erase(entry);
	return bot;
}

std::vector<std::shared_ptr<CUser>> BotRegistry::Snapshot() const
{
	std::shared_lock lock(_mutex);
	std::vector<std::shared_ptr<CUser>> snapshot;
	snapshot.reserve(_users.size());

	for (const auto& [userId, bot] : _users)
		snapshot.push_back(bot);

	return snapshot;
}

size_t BotRegistry::Size() const
{
	std::shared_lock lock(_mutex);
	return _users.size();
}

void BotRegistry::Clear()
{
	std::unique_lock lock(_mutex);
	_users.clear();
}

}
