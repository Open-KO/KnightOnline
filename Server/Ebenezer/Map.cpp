// Map.cpp: implementation of the CMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ebenezer.h"
#include "Map.h"
#include "Region.h"
#include "Define.h"
#include "User.h"
#include "EbenezerDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include <db-library/RecordSetLoader.h>

import EbenezerBinder;

using namespace db;

extern CRITICAL_SECTION g_region_critical;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C3DMap::C3DMap()
{
	m_nMapSize = 0;
	m_fUnitDist = 0.0f;
	m_fHeight = nullptr;

	m_nXRegion = 0;
	m_nZRegion = 0;

	m_ppRegion = nullptr;
	m_nZoneNumber = 0;
	m_bType = 0;
	m_wBundle = 1;
	m_sMaxUser = 150;	// Max user in Battlezone!!!
	m_pMain = nullptr;
}

C3DMap::~C3DMap()
{
	if (m_ppRegion != nullptr)
	{
		for (int i = 0; i < m_nXRegion; i++)
		{
			delete[] m_ppRegion[i];
			m_ppRegion[i] = nullptr;
		}

		delete[] m_ppRegion;
		m_ppRegion = nullptr;
	}

	if (m_fHeight != nullptr)
	{
		for (int i = 0; i < m_nMapSize; i++)
		{
			delete[] m_fHeight[i];
			m_fHeight[i] = nullptr;
		}

		delete[] m_fHeight;
	}

	if (m_ppnEvent != nullptr)
	{
		for (int i = 0; i < m_nMapSize; i++)
		{
			delete[] m_ppnEvent[i];
			m_ppnEvent[i] = nullptr;
		}

		delete[] m_ppnEvent;
		m_ppnEvent = nullptr;
	}

	if (!m_EventArray.IsEmpty())
		m_EventArray.DeleteAllData();

	if (!m_ObjectEventArray.IsEmpty())
		m_ObjectEventArray.DeleteAllData();

	if (!m_ObjectRegeneArray.IsEmpty())
		m_ObjectRegeneArray.DeleteAllData();

	if (!m_WarpArray.IsEmpty())
		m_WarpArray.DeleteAllData();
}

bool C3DMap::LoadMap(HANDLE hFile)
{
	m_pMain = (CEbenezerDlg*) AfxGetApp()->GetMainWnd();

	LoadTerrain(hFile);

	if (!m_N3ShapeMgr.Create(
		(m_nMapSize - 1) * m_fUnitDist,
		(m_nMapSize - 1) * m_fUnitDist))
		return false;

	if (!m_N3ShapeMgr.LoadCollisionData(hFile))
		return false;

	if ((m_nMapSize - 1) * m_fUnitDist != m_N3ShapeMgr.Width()
		|| (m_nMapSize - 1) * m_fUnitDist != m_N3ShapeMgr.Height())
		return false;

	int mapwidth = (int) m_N3ShapeMgr.Width();

	m_nXRegion = (int) (mapwidth / VIEW_DISTANCE) + 1;
	m_nZRegion = (int) (mapwidth / VIEW_DISTANCE) + 1;

	m_ppRegion = new CRegion* [m_nXRegion];
	for (int i = 0; i < m_nXRegion; i++)
		m_ppRegion[i] = new CRegion[m_nZRegion];

	LoadObjectEvent(hFile);
	LoadMapTile(hFile);
	LoadRegeneEvent(hFile);		// 이건 내가 추가했슴
	LoadWarpList(hFile);

	if (!LoadEvent())
	{
		AfxMessageBox(_T("Event Load Fail!!"));
		return false;
	}

	return true;
}

void C3DMap::LoadObjectEvent(HANDLE hFile)
{
	int iEventObjectCount = 0;
	__Vector3 vPos(0, 0, 0);
	DWORD dwNum;
	_OBJECT_EVENT* pEvent = nullptr;

	BYTE regene_point_counter_karus = 0;
	BYTE regene_point_counter_elmo = 0;

	ReadFile(hFile, &iEventObjectCount, 4, &dwNum, nullptr);

	for (int i = 0; i < iEventObjectCount; i++)
	{
		pEvent = new _OBJECT_EVENT;
		ReadFile(hFile, &pEvent->sBelong, 4, &dwNum, nullptr);				// 소속 : 0 -> 무소속
		ReadFile(hFile, &pEvent->sIndex, 2, &dwNum, nullptr);				// Event Index
		ReadFile(hFile, &pEvent->sType, 2, &dwNum, nullptr);				// 0 : bind point, 1,2 : gate, 3 : lever, 4 : flag lever, 5 : Warp point
		ReadFile(hFile, &pEvent->sControlNpcID, 2, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->sStatus, 2, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosX, 4, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosY, 4, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosZ, 4, &dwNum, nullptr);
		pEvent->byLife = 1;

		if (pEvent->sIndex <= 0)
			continue;

		if (!m_ObjectEventArray.PutData(pEvent->sIndex, pEvent))
		{
			spdlog::error("Map::LoadObjectEvent: ObjectEventArray put failed [eventId={} zoneId={}]",
				pEvent->sIndex, m_nZoneNumber);
			delete pEvent;
			pEvent = nullptr;
		}

//		TRACE ("성래 요청 : %d , %d , %d , %d , %d \r\n", pEvent->sBelong, pEvent->sIndex, pEvent->sType, pEvent->sControlNpcID, pEvent->sStatus);
	}
}

void C3DMap::LoadMapTile(HANDLE hFile)
{
	DWORD dwNum;

	m_ppnEvent = new short* [m_nMapSize];
	for (int x = 0; x < m_nMapSize;x++)
	{
		m_ppnEvent[x] = new short[m_nMapSize];
		ReadFile(hFile, m_ppnEvent[x], sizeof(short) * m_nMapSize, &dwNum, nullptr);
	}
}

void C3DMap::LoadRegeneEvent(HANDLE hFile)
{
	int iEventObjectCount = 0;

	__Vector3 vPos(0, 0, 0);
	DWORD dwNum;
	_REGENE_EVENT* pEvent = nullptr;

	ReadFile(hFile, &iEventObjectCount, 4, &dwNum, nullptr);

	for (int i = 0; i < iEventObjectCount; i++)
	{
		pEvent = new _REGENE_EVENT;

		pEvent->sRegenePoint = i;
		ReadFile(hFile, &pEvent->fRegenePosX, 4, &dwNum, nullptr);	// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 X
		ReadFile(hFile, &pEvent->fRegenePosY, 4, &dwNum, nullptr);	// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Y
		ReadFile(hFile, &pEvent->fRegenePosZ, 4, &dwNum, nullptr);	// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Z
		ReadFile(hFile, &pEvent->fRegeneAreaZ, 4, &dwNum, nullptr);	// 캐릭터 나타나는 지역의 Z 축 길이 
		ReadFile(hFile, &pEvent->fRegeneAreaX, 4, &dwNum, nullptr);	// 캐릭터 나타나는 지역의 X 축 길이

		if (pEvent->sRegenePoint < 0)
			continue;

		if (!m_ObjectRegeneArray.PutData(pEvent->sRegenePoint, pEvent))
		{
			spdlog::error("Map::LoadObjectEvent: RegenPoint put failed [regenPoint={} zoneId={}]",
				pEvent->sRegenePoint, m_nZoneNumber);
			delete pEvent;
			pEvent = nullptr;
		}

		// TRACE(_T("Num: %d , (x, z, y) : (%f, %f, %f) , length_x : %f  length_z : %f\n"), i,
		// pEvent->fRegenePosX, pEvent->fRegenePosZ, pEvent->fRegenePosY,
		// pEvent->fRegeneAreaX, pEvent->fRegeneAreaZ);
	}

	//TRACE(_T("\n\n"));

//	m_pMain->m_bMaxRegenePoint = iEventObjectCount;
}

void C3DMap::LoadWarpList(HANDLE hFile)
{
	int WarpCount = 0;
	DWORD dwNum;
	_WARP_INFO* pWarp = nullptr;

	ReadFile(hFile, &WarpCount, 4, &dwNum, nullptr);

	for (int i = 0; i < WarpCount; i++)
	{
		pWarp = new _WARP_INFO;

		ReadFile(hFile, pWarp, sizeof(_WARP_INFO), &dwNum, nullptr);

		if (!m_WarpArray.PutData(pWarp->sWarpID, pWarp))
		{
			spdlog::error("Map::LoadObjectEvent: WarpArray put failed [warpId={} zoneId={}]",
				pWarp->sWarpID, m_nZoneNumber);
			delete pWarp;
			pWarp = nullptr;
		}
	}
}

void C3DMap::LoadTerrain(HANDLE hFile)
{
	DWORD dwRWC;
	ReadFile(hFile, &m_nMapSize, sizeof(int), &dwRWC, nullptr);	// 가로세로 정보가 몇개씩인가?
	ReadFile(hFile, &m_fUnitDist, sizeof(float), &dwRWC, nullptr);

	m_fHeight = new float* [m_nMapSize];
	for (int z = 0; z < m_nMapSize; z++)
		m_fHeight[z] = new float[m_nMapSize];

	for (int z = 0; z < m_nMapSize; z++)
	{
		for (int x = 0; x < m_nMapSize; x++)
			ReadFile(hFile, &m_fHeight[x][z], sizeof(float), &dwRWC, nullptr);	// 높이값 읽어오기
	}
}

float C3DMap::GetHeight(float x, float y, float z)
{
	int iX, iZ;
	iX = (int) (x / m_fUnitDist);
	iZ = (int) (z / m_fUnitDist);
	//_ASSERT( iX, iZ가 범위내에 있는 값인지 체크하기);

	float fYTerrain;
	float h1, h2, h3;
	float dX, dZ;
	dX = (x - iX * m_fUnitDist) / m_fUnitDist;
	dZ = (z - iZ * m_fUnitDist) / m_fUnitDist;

//	_ASSERT(dX>=0.0f && dZ>=0.0f && dX<1.0f && dZ<1.0f);
	if (!(dX >= 0.0f
		&& dZ >= 0.0f
		&& dX < 1.0f
		&& dZ < 1.0f))
		return FLT_MIN;

	if ((iX + iZ) % 2 == 1)
	{
		if ((dX + dZ) < 1.0f)
		{
			h1 = m_fHeight[iX][iZ + 1];
			h2 = m_fHeight[iX + 1][iZ];
			h3 = m_fHeight[iX][iZ];

			//if (dX == 1.0f) return h2;

			float h12 = h1 + (h2 - h1) * dX;	// h1과 h2사이의 높이값
			float h32 = h3 + (h2 - h3) * dX;	// h3과 h2사이의 높이값
			fYTerrain = h32 + (h12 - h32) * ((dZ) / (1.0f - dX));	// 찾고자 하는 높이값
		}
		else
		{
			h1 = m_fHeight[iX][iZ + 1];
			h2 = m_fHeight[iX + 1][iZ];
			h3 = m_fHeight[iX + 1][iZ + 1];

			if (dX == 0.0f)
				return h1;

			float h12 = h1 + (h2 - h1) * dX;	// h1과 h2사이의 높이값
			float h13 = h1 + (h3 - h1) * dX;	// h1과 h3사이의 높이값
			fYTerrain = h13 + (h12 - h13) * ((1.0f - dZ) / (dX));	// 찾고자 하는 높이값
		}
	}
	else
	{
		if (dZ > dX)
		{
			h1 = m_fHeight[iX][iZ + 1];
			h2 = m_fHeight[iX + 1][iZ + 1];
			h3 = m_fHeight[iX][iZ];

			//if (dX == 1.0f) return h2;

			float h12 = h1 + (h2 - h1) * dX;	// h1과 h2사이의 높이값
			float h32 = h3 + (h2 - h3) * dX;	// h3과 h2사이의 높이값
			fYTerrain = h12 + (h32 - h12) * ((1.0f - dZ) / (1.0f - dX));	// 찾고자 하는 높이값
		}
		else
		{
			h1 = m_fHeight[iX][iZ];
			h2 = m_fHeight[iX + 1][iZ];
			h3 = m_fHeight[iX + 1][iZ + 1];

			if (dX == 0.0f)
				return h1;

			float h12 = h1 + (h2 - h1) * dX;	// h1과 h2사이의 높이값
			float h13 = h1 + (h3 - h1) * dX;	// h1과 h3사이의 높이값
			fYTerrain = h12 + (h13 - h12) * ((dZ) / (dX));	// 찾고자 하는 높이값
		}
	}

	__Vector3 vPos(x, y, z);
	float fHeight = m_N3ShapeMgr.GetHeightNearstPos(vPos); // 가장 가까운 높이값을 돌려준다..
	if (-FLT_MAX != fHeight
		&& fHeight > fYTerrain)
		return fHeight;

	return fYTerrain;
}

bool C3DMap::ObjectCollision(float x1, float z1, float y1, float x2, float z2, float y2)
{
	__Vector3 vec1(x1, y1, z1), vec2(x2, y2, z2);
	__Vector3 vDir = vec2 - vec1;

	float fSpeed = vDir.Magnitude();
	vDir.Normalize();

	return m_N3ShapeMgr.CheckCollision(vec1, vDir, fSpeed);
}

bool C3DMap::RegionItemAdd(int rx, int rz, _ZONE_ITEM* pItem)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return false;

	if (pItem == nullptr)
		return false;

	EnterCriticalSection(&g_region_critical);

	m_ppRegion[rx][rz].m_RegionItemArray.PutData(pItem->bundle_index, pItem);

	m_wBundle++;
	if (m_wBundle > ZONEITEM_MAX)
		m_wBundle = 1;

	LeaveCriticalSection(&g_region_critical);

	return true;
}

bool C3DMap::RegionItemRemove(int rx, int rz, int bundle_index, int itemid, int count)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return false;

	_ZONE_ITEM* pItem = nullptr;
	CRegion* region = &m_ppRegion[rx][rz];
	bool bFind = false;
	short t_count = 0;

	EnterCriticalSection(&g_region_critical);

	pItem = region->m_RegionItemArray.GetData(bundle_index);
	if (pItem != nullptr)
	{
		for (int j = 0; j < 6; j++)
		{
			if (pItem->itemid[j] == itemid
				&& pItem->count[j] == count)
			{
				pItem->itemid[j] = 0; pItem->count[j] = 0;
				bFind = true;
				break;
			}
		}

		if (bFind)
		{
			for (int j = 0; j < 6; j++)
			{
				if (pItem->itemid[j] != 0)
					t_count++;
			}

			if (t_count == 0)
				region->m_RegionItemArray.DeleteData(bundle_index);
		}
	}

	LeaveCriticalSection(&g_region_critical);

	return bFind;
}

void C3DMap::RegionUserAdd(int rx, int rz, int uid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return;

	int* pInt = new int;
	*pInt = uid;

	EnterCriticalSection(&g_region_critical);

	if (!m_ppRegion[rx][rz].m_RegionUserArray.PutData(uid, pInt))
		delete pInt;

	LeaveCriticalSection(&g_region_critical);

	//TRACE(_T("++++ Region Add(%d) : x=%d, z=%d, uid=%d ++++\n"), m_nZoneNumber, rx, rz, uid);
}

void C3DMap::RegionUserRemove(int rx, int rz, int uid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);
	region->m_RegionUserArray.DeleteData(uid);
	LeaveCriticalSection(&g_region_critical);

	//TRACE(_T("---- Region Remove(%d) : x=%d, z=%d, uid=%d ----\n"), m_nZoneNumber, rx, rz, uid);
}

void C3DMap::RegionNpcAdd(int rx, int rz, int nid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return;

	CRegion* region = &m_ppRegion[rx][rz];

	int* pInt = new int;
	*pInt = nid;

	EnterCriticalSection(&g_region_critical);

	if (!region->m_RegionNpcArray.PutData(nid, pInt))
		delete pInt;

	LeaveCriticalSection(&g_region_critical);
}

void C3DMap::RegionNpcRemove(int rx, int rz, int nid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_nXRegion
		|| rz >= m_nZRegion)
		return;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);
	region->m_RegionNpcArray.DeleteData(nid);
	LeaveCriticalSection(&g_region_critical);
}

bool C3DMap::CheckEvent(float x, float z, CUser* pUser)
{
	CGameEvent* pEvent = nullptr;
	int iX, iZ, event_index = 0;

	iX = (int) (x / m_fUnitDist);
	iZ = (int) (z / m_fUnitDist);
	if (iX < 0
		|| iX >= m_nMapSize
		|| iZ < 0
		|| iZ >= m_nMapSize)
		return false;

	event_index = m_ppnEvent[iX][iZ];
	if (event_index < 2)
		return false;

	pEvent = m_EventArray.GetData(event_index);
	if (pEvent != nullptr)
	{
		if (pEvent->m_bType == 1
			&& pEvent->m_iExec[0] == ZONE_BATTLE
			&& m_pMain->m_byBattleOpen != NATION_BATTLE)
			return false;

		if (pEvent->m_bType == 1
			&& pEvent->m_iExec[0] == ZONE_SNOW_BATTLE
			&& m_pMain->m_byBattleOpen != SNOW_BATTLE)
			return false;

		if (pUser->m_pUserData->m_bNation == KARUS
			&& pEvent->m_iExec[0] == ZONE_BATTLE)
		{
			if (m_pMain->m_sKarusCount > MAX_BATTLE_ZONE_USERS)
			{
				spdlog::error("Map::CheckEvent: BattleZone: karus full users [users={} charId={}]",
					m_pMain->m_sKarusCount, pUser->m_pUserData->m_id);
				return false;
			}
		}
		else if (pUser->m_pUserData->m_bNation == ELMORAD
			&& pEvent->m_iExec[0] == ZONE_BATTLE)
		{
			if (m_pMain->m_sElmoradCount > MAX_BATTLE_ZONE_USERS)
			{
				spdlog::error("Map::CheckEvent: BattleZone: elmorad full users [users={} charId={}]",
					m_pMain->m_sElmoradCount, pUser->m_pUserData->m_id);
				return false;
			}
		}

		pEvent->RunEvent(pUser);
		return true;
	}

	return false;
}

bool C3DMap::LoadEvent()
{
	using ModelType = model::Event;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([this](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			if (row.ZoneNumber != m_nZoneNumber)
				continue;

			CGameEvent* pEvent = new CGameEvent();

			pEvent->m_sIndex = row.EventNumber;
			pEvent->m_bType = row.EventType;

			pEvent->m_iExec[0] = atoi(row.Execute1.c_str());
			pEvent->m_iExec[1] = atoi(row.Execute2.c_str());
			pEvent->m_iExec[2] = atoi(row.Execute3.c_str());
			pEvent->m_iExec[3] = atoi(row.Execute4.c_str());
			pEvent->m_iExec[4] = atoi(row.Execute5.c_str());

			if (!m_EventArray.PutData(pEvent->m_sIndex, pEvent))
			{
				spdlog::error("Map::LoadEvent: EventArray put failed [eventId={} zoneId={}]",
					pEvent->m_sIndex, m_nZoneNumber);
				delete pEvent;
			}
		}
		while (recordset.next());
	});

	if (!loader.Load_ForbidEmpty())
	{
		m_pMain->ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool C3DMap::IsValidPosition(float x, float z, float y) const
{
	if (x >= m_N3ShapeMgr.Width())
		return false;

	if (z >= m_N3ShapeMgr.Width())
		return false;

	return true;
}
