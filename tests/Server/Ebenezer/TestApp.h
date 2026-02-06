#ifndef TESTS_SERVER_EBENEZER_TESTAPP_H
#define TESTS_SERVER_EBENEZER_TESTAPP_H

#pragma once

#include <Ebenezer/EbenezerApp.h>
#include <Ebenezer/EbenezerLogger.h>
#include <Ebenezer/Map.h>
#include <Ebenezer/Npc.h>

#include "TestMap.h"
#include "TestUser.h"

class TestApp : public Ebenezer::EbenezerApp
{
public:
	TestApp() : EbenezerApp(_logger)
	{
		spdlog::set_level(spdlog::level::off);

		InitSocketManager(Ebenezer::MAX_USER);
	}

	void InitSocketManager(int serverSocketCount)
	{
		_serverSocketManager.InitSockets(serverSocketCount);

		for (int i = 0; i < serverSocketCount; i++)
		{
			auto user = std::make_shared<TestUser>();
			user->SetSocketID(i);

			_serverSocketManager._inactiveSocketArray[i] = std::move(user);
		}
	}

	std::shared_ptr<TestUser> AddUser()
	{
		std::shared_ptr<TestUser> user;
		int socketId = -1;

		{
			std::lock_guard<std::recursive_mutex> lock(_serverSocketManager._mutex);
			user = std::static_pointer_cast<TestUser>(_serverSocketManager.AcquireSocket(socketId));
		}

		if (user == nullptr)
			return nullptr;

		user->Initialize();

		return user;
	}

	TestMap* CreateMap(uint8_t zoneId, int mapSize = TestMap::DEFAULT_MAP_SIZE)
	{
		auto map = new TestMap(zoneId, mapSize);
		if (map == nullptr)
			return nullptr;

		m_ZoneArray.push_back(map);
		return map;
	}

	Ebenezer::CNpc* CreateNPC(uint16_t npcId)
	{
		auto npc = new Ebenezer::CNpc();
		if (npc == nullptr)
			return nullptr;

		npc->m_sNid = npcId;

		if (!m_NpcMap.PutData(npcId, npc))
		{
			delete npc;
			return nullptr;
		}

		return npc;
	}

	bool AddItemEntry(const Ebenezer::model::Item& itemModel)
	{
		auto modelForInsertion = new Ebenezer::model::Item { itemModel };
		if (modelForInsertion == nullptr)
			return false;

		if (!m_ItemTableMap.PutData(modelForInsertion->ID, modelForInsertion))
		{
			delete modelForInsertion;
			return false;
		}

		return true;
	}

	bool AddItemUpgradeEntry(const Ebenezer::model::ItemUpgrade& itemUpgradeModel)
	{
		auto modelForInsertion = new Ebenezer::model::ItemUpgrade { itemUpgradeModel };
		if (modelForInsertion == nullptr)
			return false;

		m_ItemUpgradeTableArray.push_back(modelForInsertion);
		return true;
	}

protected:
	Ebenezer::EbenezerLogger _logger;
};

#endif // TESTS_SERVER_EBENEZER_TESTAPP_H
