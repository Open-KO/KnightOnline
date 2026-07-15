#ifndef SERVER_EBENEZER_BOTREGISTRY_H
#define SERVER_EBENEZER_BOTREGISTRY_H

#pragma once

#include <map>
#include <memory>
#include <shared_mutex>
#include <vector>

namespace Ebenezer
{

class CUser;

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

}

#endif // SERVER_EBENEZER_BOTREGISTRY_H
