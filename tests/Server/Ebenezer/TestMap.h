#ifndef TESTS_SERVER_EBENEZER_TESTMAP_H
#define TESTS_SERVER_EBENEZER_TESTMAP_H

#pragma once

#include <Ebenezer/EbenezerApp.h>
#include <Ebenezer/Map.h>
#include <Ebenezer/Npc.h>
#include <Ebenezer/Region.h>
#include <Ebenezer/User.h>

class TestMap : public Ebenezer::C3DMap
{
public:
	static constexpr int DEFAULT_MAP_SIZE = 1024; // 1024x1024

	TestMap(uint8_t zoneId, int mapSize = DEFAULT_MAP_SIZE)
	{
		m_nZoneNumber = zoneId;
		m_pMain       = Ebenezer::EbenezerApp::instance();
		m_nMapSize    = mapSize;

		m_nXRegion    = (int) (mapSize / VIEW_DISTANCE) + 1;
		m_nZRegion    = (int) (mapSize / VIEW_DISTANCE) + 1;

		m_ppRegion    = new Ebenezer::CRegion*[m_nXRegion];
		for (int i = 0; i < m_nXRegion; i++)
			m_ppRegion[i] = new Ebenezer::CRegion[m_nZRegion];

		m_fHeight = new float*[mapSize];
		for (int z = 0; z < mapSize; z++)
			m_fHeight[z] = new float[mapSize] {};

		m_ppnEvent = new int16_t*[mapSize];
		for (int x = 0; x < mapSize; x++)
			m_ppnEvent[x] = new int16_t[mapSize] {};
	}

	bool Add(Ebenezer::CUser* user, uint16_t regionX, uint16_t regionZ)
	{
		user->m_pUserData->m_bZone = m_nZoneNumber;
		user->m_iZoneIndex         = m_pMain->GetZoneIndex(m_nZoneNumber);
		user->m_RegionX            = regionX;
		user->m_RegionZ            = regionZ;
		return RegionUserAdd(regionX, regionZ, user->GetSocketID());
	}

	bool Add(Ebenezer::CNpc* npc, uint16_t regionX, uint16_t regionZ)
	{
		npc->m_sCurZone   = m_nZoneNumber;
		npc->m_sZoneIndex = m_pMain->GetZoneIndex(m_nZoneNumber);
		npc->m_sRegion_X  = regionX;
		npc->m_sRegion_Z  = regionZ;
		return RegionNpcAdd(regionX, regionZ, npc->m_sNid);
	}
};

#endif // TESTS_SERVER_EBENEZER_TESTMAP_H
