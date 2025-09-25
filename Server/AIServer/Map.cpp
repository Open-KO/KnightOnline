// Map.cpp: implementation of the CMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Map.h"
#include "Serverdlg.h"
#include "Region.h"
#include "Npc.h"
#include "User.h"
#include "RoomEvent.h"

#include <shared/globals.h>
#include <shared/StringConversion.h>
#include <spdlog/spdlog.h>

#include <filesystem>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION g_region_critical;

CMapInfo::CMapInfo()
{
	m_sEvent = 0;
}

CMapInfo::~CMapInfo()
{
}

MAP::MAP()
{
	m_pMain = (CServerDlg*) AfxGetApp()->GetMainWnd();
	m_nMapSize = 0;
	m_fUnitDist = 0.0f;
	m_fHeight = nullptr;

	m_sizeRegion.cx = 0;
	m_sizeRegion.cy = 0;
	m_sizeMap.cx = 0;
	m_sizeMap.cy = 0;

	m_ppRegion = nullptr;
	//m_pRoomEvent = nullptr;
	m_nZoneNumber = 0;
	m_byRoomType = 0;
	m_byRoomEvent = 0;
	m_byRoomStatus = 1;
	m_byInitRoomCount = 0;
	m_sKarusRoom = 0;
	m_sElmoradRoom = 0;
//	for(int i=0; i<MAX_DUNGEON_BOSS_MONSTER; i++)
//		m_arDungeonBossMonster[i] = 1;
}

MAP::~MAP()
{
	RemoveMapData();
}

void MAP::RemoveMapData()
{
//	int i, j, k;

	if (m_ppRegion != nullptr)
	{
		for (int i = 0; i < m_sizeRegion.cx; i++)
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

	if (m_pMap != nullptr)
	{
		for (int i = 0; i < m_sizeMap.cx; i++)
		{
			delete[] m_pMap[i];
			m_pMap[i] = nullptr;
		}

		delete[] m_pMap;
		m_pMap = nullptr;
	}

	if (!m_ObjectEventArray.IsEmpty())
		m_ObjectEventArray.DeleteAllData();

	if (!m_arRoomEventArray.IsEmpty())
		m_arRoomEventArray.DeleteAllData();
}

bool MAP::IsMovable(int dest_x, int dest_y) const
{
	if (dest_x < 0
		|| dest_y < 0)
		return false;

	if (m_pMap == nullptr)
		return false;

	if (dest_x >= m_sizeMap.cx
		|| dest_y >= m_sizeMap.cy)
		return false;

	return m_pMap[dest_x][dest_y].m_sEvent == 0;
	//return m_pMap[dest_x][dest_y].m_bMove;
}

///////////////////////////////////////////////////////////////////////
//	각 서버가 담당하고 있는 zone의 Map을 로드한다.
//
bool MAP::LoadMap(HANDLE hFile)
{
	LoadTerrain(hFile);

	m_N3ShapeMgr.Create(
		(m_nMapSize - 1) * m_fUnitDist,
		(m_nMapSize - 1) * m_fUnitDist);

	if (!m_N3ShapeMgr.LoadCollisionData(hFile))
		return false;

	if ((m_nMapSize - 1) * m_fUnitDist != m_N3ShapeMgr.Width()
		|| (m_nMapSize - 1) * m_fUnitDist != m_N3ShapeMgr.Height())
		return false;

	int mapwidth = (int) m_N3ShapeMgr.Width();

	m_sizeRegion.cx = (int) (mapwidth / VIEW_DIST) + 1;
	m_sizeRegion.cy = (int) (mapwidth / VIEW_DIST) + 1;

	m_sizeMap.cx = m_nMapSize;
	m_sizeMap.cy = m_nMapSize;

	m_ppRegion = new CRegion* [m_sizeRegion.cx];
	for (int i = 0; i < m_sizeRegion.cx; i++)
	{
		m_ppRegion[i] = new CRegion[m_sizeRegion.cy];
		m_ppRegion[i]->m_byMoving = 0;
	}

	LoadObjectEvent(hFile);
	LoadMapTile(hFile);

	return true;
}

void MAP::LoadTerrain(HANDLE hFile)
{
	DWORD dwRWC;
	ReadFile(hFile, &m_nMapSize, sizeof(int), &dwRWC, nullptr);	// 가로세로 정보가 몇개씩인가?
	ReadFile(hFile, &m_fUnitDist, sizeof(float), &dwRWC, nullptr);

	m_fHeight = new float* [m_nMapSize];
	for (int i = 0; i < m_nMapSize; i++)
		m_fHeight[i] = new float[m_nMapSize];

	for (int z = 0; z < m_nMapSize; z++)
	{
		for (int x = 0; x < m_nMapSize; x++)
			ReadFile(hFile, &m_fHeight[x][z], sizeof(float), &dwRWC, nullptr);	// 높이값 읽어오기
	}
}

float MAP::GetHeight(float x, float z)
{
	int iX, iZ;
	iX = (int) (x / m_fUnitDist);
	iZ = (int) (z / m_fUnitDist);
	//_ASSERT( iX, iZ가 범위내에 있는 값인지 체크하기);

	float y;
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
			y = h32 + (h12 - h32) * ((dZ) / (1.0f - dX));	// 찾고자 하는 높이값
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
			y = h13 + (h12 - h13) * ((1.0f - dZ) / (dX));	// 찾고자 하는 높이값
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
			y = h12 + (h32 - h12) * ((1.0f - dZ) / (1.0f - dX));	// 찾고자 하는 높이값
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
			y = h12 + (h13 - h12) * ((dZ) / (dX));	// 찾고자 하는 높이값
		}
	}

	return y;
}

bool MAP::ObjectIntersect(float x1, float z1, float y1, float x2, float z2, float y2)
{
	__Vector3 vec1(x1, y1, z1), vec2(x2, y2, z2);
	__Vector3 vDir = vec2 - vec1;
	float fSpeed = vDir.Magnitude();
	vDir.Normalize();

	return m_N3ShapeMgr.CheckCollision(vec1, vDir, fSpeed);
}

void MAP::RegionUserAdd(int rx, int rz, int uid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_sizeRegion.cx
		|| rz >= m_sizeRegion.cy)
		return;

	int* pInt = new int;
	*pInt = uid;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);
	
	if (!region->m_RegionUserArray.PutData(uid, pInt))
		delete pInt;

	LeaveCriticalSection(&g_region_critical);
}

void MAP::RegionUserRemove(int rx, int rz, int uid)
{
	if (rx < 0
		|| rz < 0
		|| rx > m_sizeRegion.cx
		|| rz > m_sizeRegion.cy)
		return;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);

	region->m_RegionUserArray.DeleteData(uid);

	LeaveCriticalSection(&g_region_critical);
}

void MAP::RegionNpcAdd(int rx, int rz, int nid)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_sizeRegion.cx
		|| rz >= m_sizeRegion.cy)
		return;

	int* pInt = new int;
	*pInt = nid;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);

	if (!region->m_RegionNpcArray.PutData(nid, pInt))
	{
		delete pInt;
		spdlog::error("Map::RegionNpcAdd: RegionNpcArray put failed [x={} z={} npcId={}]", rx, rz, nid);
	}

	int nSize = m_ppRegion[rx][rz].m_RegionNpcArray.GetSize();
	//TRACE(_T("+++ Map - RegionNpcAdd : x=%d,z=%d, nid=%d, total=%d \n"), rx,rz,nid, nSize);

	LeaveCriticalSection(&g_region_critical);
}

void MAP::RegionNpcRemove(int rx, int rz, int nid)
{
	if (rx < 0
		|| rz < 0
		|| rx > m_sizeRegion.cx
		|| rz > m_sizeRegion.cy)
		return;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);

	region->m_RegionNpcArray.DeleteData(nid);

	LeaveCriticalSection(&g_region_critical);
}

void MAP::LoadMapTile(HANDLE hFile)
{
	//MapTile속성 읽기..
	//	속성이 0이면 못 가는 곳.
	//	1이면 그냥 가는 곳...
	//	그외는 이벤트 ID.
	//
	int x1 = m_sizeMap.cx;
	int z1 = m_sizeMap.cy;
	DWORD dwNum;
	int16_t** pEvent = new int16_t* [m_sizeMap.cx];

	// 잠시 막아놓고..
	for (int x = 0; x < m_sizeMap.cx; x++)
	{
		pEvent[x] = new int16_t[m_sizeMap.cx];
		ReadFile(hFile, pEvent[x], sizeof(int16_t) * m_sizeMap.cy, &dwNum, nullptr);
	}

	m_pMap = new CMapInfo* [m_sizeMap.cx];

	for (int i = 0; i < m_sizeMap.cx; i++)
		m_pMap[i] = new CMapInfo[m_sizeMap.cy];

	int count = 0;
	for (int i = 0; i < m_sizeMap.cy; i++)
	{
		for (int j = 0; j < m_sizeMap.cx; j++)
		{
			m_pMap[j][i].m_sEvent = (int16_t) pEvent[j][i];

			// NOTE: The SMDs don't have the correct data.
			// Since we can't trust their data, we must assume every tile is movable.
			m_pMap[j][i].m_sEvent = 1;
			//

			if (m_pMap[j][i].m_sEvent >= 1)
				count++;
		//	m_pMap[j][i].m_lUser	= 0;
		//	m_pMap[j][i].m_dwType = 0;
		}
	}
	spdlog::trace("Map::LoadMapTile: move count={}", count);

/*	FILE* stream = fopen("c:\\move1.txt", "w");

	for(int z=m_sizeMap.cy-1; z>=0; z--)
	{
		for(int x=0; x<m_sizeMap.cx; x++)
		{
			int v = m_pMap[x][z].m_sEvent;
			fprintf(stream, "%d",v);
		}
		fprintf(stream, "\n");
	}
	fclose(stream);	*/


	if (pEvent != nullptr)
	{
		for (int i = 0; i < m_sizeMap.cx; i++)
		{
			delete[] pEvent[i];
			pEvent[i] = nullptr;
		}

		delete[] pEvent;
		pEvent = nullptr;
	}
}

int MAP::GetRegionUserSize(int rx, int rz)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_sizeRegion.cx
		|| rz >= m_sizeRegion.cy)
		return 0;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);
	int nRet = region->m_RegionUserArray.GetSize();
	LeaveCriticalSection(&g_region_critical);

	return nRet;
}

int  MAP::GetRegionNpcSize(int rx, int rz)
{
	if (rx < 0
		|| rz < 0
		|| rx >= m_sizeRegion.cx
		|| rz >= m_sizeRegion.cy)
		return 0;

	CRegion* region = &m_ppRegion[rx][rz];

	EnterCriticalSection(&g_region_critical);
	int nRet = region->m_RegionNpcArray.GetSize();
	LeaveCriticalSection(&g_region_critical);

	return nRet;
}

void MAP::LoadObjectEvent(HANDLE hFile)
{
	int iEventObjectCount = 0, zonenum = 0;
	__Vector3 vPos(0, 0, 0);
	DWORD	dwNum;
	_OBJECT_EVENT* pEvent = nullptr;

	ReadFile(hFile, &iEventObjectCount, 4, &dwNum, nullptr);
	for (int i = 0; i < iEventObjectCount; i++)
	{
		pEvent = new _OBJECT_EVENT;
		ReadFile(hFile, &pEvent->sBelong, 4, &dwNum, nullptr);					// 소속 
		ReadFile(hFile, &pEvent->sIndex, 2, &dwNum, nullptr);				// Event Index
		ReadFile(hFile, &pEvent->sType, 2, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->sControlNpcID, 2, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->sStatus, 2, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosX, 4, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosY, 4, &dwNum, nullptr);
		ReadFile(hFile, &pEvent->fPosZ, 4, &dwNum, nullptr);

		//TRACE(_T("Object - belong=%d, index=%d, type=%d, con=%d, sta=%d\n"), pEvent->sBelong, pEvent->sIndex, pEvent->sType, pEvent->sControlNpcID, pEvent->sStatus);

		// 작업할것 : 맵데이터가 바뀌면 Param1이 2이면 성문인것을 판단..  3이면 레버..
		if (pEvent->sType == OBJECT_TYPE_GATE
			|| pEvent->sType == OBJECT_TYPE_DOOR_TOPDOWN
			|| pEvent->sType == OBJECT_TYPE_GATE_LEVER
			|| pEvent->sType == OBJECT_TYPE_BARRICADE
			|| pEvent->sType == OBJECT_TYPE_REMOVE_BIND
			|| pEvent->sType == OBJECT_TYPE_ANVIL
			|| pEvent->sType == OBJECT_TYPE_ARTIFACT)
			m_pMain->AddObjectEventNpc(pEvent, m_nZoneNumber);

		if (pEvent->sIndex <= 0)
			continue;

		if (!m_ObjectEventArray.PutData(pEvent->sIndex, pEvent))
		{
			spdlog::error("Map::LoadObjectEvent: ObjectEventArray put failed [eventId={} zoneId={}]",
				pEvent->sIndex, m_nZoneNumber);
			delete pEvent;
			pEvent = nullptr;
		}
	}
}

bool MAP::LoadRoomEvent(int zone_number)
{
	CString		filename;
	CFile		pFile;
	uint8_t		byte;
	char		buf[4096];
	char		first[1024];
	char		temp[1024];
	int			index = 0;
	int			t_index = 0, logic = 0, exec = 0;
	int			event_num = 0, nation = 0;

	CRoomEvent* pEvent = nullptr;

	// Build the base MAP directory
	std::filesystem::path evtPath(GetProgPath().GetString());
	evtPath /= MAP_DIR;
	evtPath /= std::to_wstring(zone_number) + L".evt";

	if (!std::filesystem::exists(evtPath))
		return true;

	// Resolve it to strip the relative references to be nice.
	// NOTE: Requires the file to exist.
	evtPath = std::filesystem::canonical(evtPath);

	filename.Format(_T("%ls"), evtPath.c_str());

	if (!pFile.Open(filename, CFile::modeRead))
		return false;

	std::wstring filenameWide = evtPath.wstring();

	uint64_t length = pFile.GetLength();

	CArchive in(&pFile, CArchive::load);
	int lineNumber = 0;
	uint64_t count = 0;

	while (count < length)
	{
		in >> byte;
		++count;

		if ((char) byte != '\r'
			&& (char) byte != '\n')
			buf[index++] = byte;

		if ((char) byte == '\n'
			|| count == length)
		{
			++lineNumber;

			if (index <= 1)
				continue;

			buf[index] = (uint8_t) 0;
			t_index = 0;

			// 주석에 대한 처리
			if (buf[t_index] == ';'
				|| buf[t_index] == '/')
			{
				index = 0;
				continue;
			}

			t_index += ParseSpace(first, buf + t_index);

			if (0 == strcmp(first, "ROOM"))
			{
				logic = 0;
				exec = 0;

				t_index += ParseSpace(temp, buf + t_index);
				event_num = atoi(temp);

				if (m_arRoomEventArray.GetData(event_num) != nullptr)
				{
					spdlog::error("Map::LoadRoomEvent: Duplicate event definition [eventId={} zoneId={}]",
						event_num, m_nZoneNumber);
					goto cancel_event_load;
				}

				pEvent = SetRoomEvent(event_num);
			}
			else if (0 == strcmp(first, "TYPE"))
			{
				t_index += ParseSpace(temp, buf + t_index);
				m_byRoomType = atoi(temp);
			}
			else if (0 == strcmp(first, "L"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;
			}
			else if (0 == strcmp(first, "E"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Exec[exec].sNumber = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Exec[exec].sOption_1 = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Exec[exec].sOption_2 = atoi(temp);

				exec++;
			}
			else if (0 == strcmp(first, "A"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Logic[logic].sNumber = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Logic[logic].sOption_1 = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_Logic[logic].sOption_2 = atoi(temp);

				logic++;
				pEvent->m_byCheck = logic;
			}
			else if (0 == strcmp(first, "O"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;
			}
			else if (0 == strcmp(first, "NATION"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;

				t_index += ParseSpace(temp, buf + t_index);
				nation = atoi(temp);

				if (nation == KARUS_ZONE)
					++m_sKarusRoom;
				else if (nation == ELMORAD_ZONE)
					++m_sElmoradRoom;
			}
			else if (0 == strcmp(first, "POS"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iInitMinX = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iInitMinZ = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iInitMaxX = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iInitMaxZ = atoi(temp);
			}
			else if (0 == strcmp(first, "POSEND"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iEndMinX = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iEndMinZ = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iEndMaxX = atoi(temp);

				t_index += ParseSpace(temp, buf + t_index);
				pEvent->m_iEndMaxZ = atoi(temp);
			}
			else if (0 == strcmp(first, "END"))
			{
				if (pEvent == nullptr)
					goto cancel_event_load;
			}
			else if (isalnum(first[0]))
			{
				spdlog::warn(
					"MAP::LoadRoomEvent({}): unhandled opcode '{}' ({}:{})",
					zone_number, first, WideToUtf8(filenameWide), lineNumber);
			}

			index = 0;
		}
	}

	in.Close();
	pFile.Close();

	return true;

cancel_event_load:
	CString str;
	str.Format(_T("LoadRoomEvent Failed [zoneId=%d eventId=%d]"), zone_number, event_num);
	AfxMessageBox(str);
	in.Close();
	pFile.Close();
//	DeleteAll();
	return false;
	//return true;
}

int MAP::IsRoomCheck(float fx, float fz)
{
	// dungeion work
	// 현재의 존이 던젼인지를 판단, 아니면 리턴처리

	CRoomEvent* pRoom = nullptr;
	char notify[100] = {};

	int nSize = m_arRoomEventArray.GetSize();
	int nX = (int) fx;
	int nZ = (int) fz;
	int minX = 0, minZ = 0, maxX = 0, maxZ = 0;
	int room_number = 0;

	for (int i = 1; i < nSize + 1; i++)
	{
		pRoom = m_arRoomEventArray.GetData(i);
		if (pRoom == nullptr)
			continue;

		// 방이 실행중이거나 깬(clear) 상태라면 검색하지 않음
		if (pRoom->m_byStatus == 3)
			continue;

		bool bFlag_1 = false;
		bool bFlag_2 = false;

		// 방이 초기화 상태
		if (pRoom->m_byStatus == 1)
		{
			minX = pRoom->m_iInitMinX;
			minZ = pRoom->m_iInitMinZ;
			maxX = pRoom->m_iInitMaxX;
			maxZ = pRoom->m_iInitMaxZ;
		}
		// 진행중인 상태
		else if (pRoom->m_byStatus == 2)
		{
			// 목표지점까지 이동하는게 아니라면,,
			if (pRoom->m_Logic[0].sNumber != 4)
				continue;

			minX = pRoom->m_iEndMinX;
			minZ = pRoom->m_iEndMinZ;
			maxX = pRoom->m_iEndMaxX;
			maxZ = pRoom->m_iEndMaxZ;
		}

		if (minX < maxX)
		{
			if (COMPARE(nX, minX, maxX))
				bFlag_1 = true;
		}
		else
		{
			if (COMPARE(nX, maxX, minX))
				bFlag_1 = true;
		}

		if (minZ < maxZ)
		{
			if (COMPARE(nZ, minZ, maxZ))
				bFlag_2 = true;
		}
		else
		{
			if (COMPARE(nZ, maxZ, minZ))
				bFlag_2 = true;
		}

		if (bFlag_1
			&& bFlag_2)
		{
			// 방이 초기화 상태
			if (pRoom->m_byStatus == 1)
			{
				pRoom->m_byStatus = 2;	// 진행중 상태로 방상태 변환
				pRoom->m_fDelayTime = TimeGet();
				room_number = i;
				spdlog::trace("Map::IsRoomCheck: [roomEventId={} zoneId={} x={} z={}]",
					i, m_nZoneNumber, nX, nZ);
				//wsprintf(notify, "** 알림 : [%d Zone][%d] 방에 들어오신것을 환영합니다 **", m_nZoneNumber, pRoom->m_sRoomNumber);
				//m_pMain->SendSystemMsg( notify, m_nZoneNumber, PUBLIC_CHAT, SEND_ALL);
			}
			// 진행중인 상태
			else if (pRoom->m_byStatus == 2)
			{
				pRoom->m_byStatus = 3;					// 클리어 상태로
				//wsprintf(notify, "** 알림 : [%d Zone][%d] 목표지점까지 도착해서 클리어 됩니다ㅇ **", m_nZoneNumber, pRoom->m_sRoomNumber);
				//m_pMain->SendSystemMsg( notify, m_nZoneNumber, PUBLIC_CHAT, SEND_ALL);
			}

			return room_number;
		}
	}

	return room_number;
}

CRoomEvent* MAP::SetRoomEvent(int number)
{
	CRoomEvent* pEvent = m_arRoomEventArray.GetData(number);
	if (pEvent != nullptr)
	{
		spdlog::error("Map::SetRoomEvent: RoomEvent duplicate definition [roomEventId={} zoneId={}]",
			number, m_nZoneNumber);
		return nullptr;
	}

	pEvent = new CRoomEvent;
	pEvent->m_iZoneNumber = m_nZoneNumber;
	pEvent->m_sRoomNumber = number;
	if (!m_arRoomEventArray.PutData(pEvent->m_sRoomNumber, pEvent))
	{
		delete pEvent;
		pEvent = nullptr;
		return nullptr;
	}

	return pEvent;
}

bool MAP::IsRoomStatusCheck()
{
	CRoomEvent* pRoom = nullptr;
	int nTotalRoom = m_arRoomEventArray.GetSize() + 1;
	int nClearRoom = 1;

	// 방을 초기화중
	if (m_byRoomStatus == 2)
		m_byInitRoomCount++;

	for (int i = 1; i < nTotalRoom; i++)
	{
		pRoom = m_arRoomEventArray.GetData(i);
		if (pRoom == nullptr)
		{
			spdlog::warn("Map::IsRoomStatusCheck: RoomEvent null [roomEventId={} zoneId={}",
				i, m_nZoneNumber);
			continue;
			//return nullptr;
		}

		// 방 진행중
		if (m_byRoomStatus == 1)
		{
			if (pRoom->m_byStatus == 3)
				nClearRoom += 1;

			if (m_byRoomType == 0)
			{
				// 방이 다 클리어 되었어여.. 초기화 해줘여,,
				if (nTotalRoom == nClearRoom)
				{
					m_byRoomStatus = 2;
					spdlog::trace("Map::IsRoomStatusCheck: all rooms cleared [zoneId={} roomType={} roomStatus={}]",
						m_nZoneNumber, m_byRoomType, m_byRoomStatus);
					return true;
				}
			}
		}
		// 방을 초기화중
		else if (m_byRoomStatus == 2)
		{
			if (m_byInitRoomCount >= 10)
			{
				pRoom->InitializeRoom();		// 실제 방을 초기화
				nClearRoom += 1;

				// 방이 초기화 되었어여.. 
				if (nTotalRoom == nClearRoom)
				{
					m_byRoomStatus = 3;
					spdlog::trace("Map::IsRoomStatusCheck: room initialized [zoneId={} roomType={} roomStatus={}]",
						m_nZoneNumber, m_byRoomType, m_byRoomStatus);
					return true;
				}
			}
		}
		// 방 초기화 완료
		else if (m_byRoomStatus == 3)
		{
			m_byRoomStatus = 1;
			m_byInitRoomCount = 0;
			spdlog::trace("Map::IsRoomStatusCheck: room restarted [zoneId={} roomType={} roomStatus={}]",
						m_nZoneNumber, m_byRoomType, m_byRoomStatus);
			return true;
		}
	}

	return false;
}

void MAP::InitializeRoom()
{
	CRoomEvent* pRoom = nullptr;
	int nTotalRoom = m_arRoomEventArray.GetSize() + 1;

	for (int i = 1; i < nTotalRoom; i++)
	{
		pRoom = m_arRoomEventArray.GetData(i);
		if (pRoom == nullptr)
		{
			spdlog::error("Map::InitializeRoom: RoomEvent null [roomEventId={} zoneId={}]",
				i, m_nZoneNumber);
			continue;
		}

		pRoom->InitializeRoom();		// 실제 방을 초기화
		m_byRoomStatus = 1;
		m_byInitRoomCount = 0;
	}
}

/// \brief Checks if a position is valid for the map
bool MAP::IsValidPosition(float x, float z) const
{
	int mapMaxX = static_cast<int>((m_sizeMap.cx - 1) * m_fUnitDist);
	int mapMaxZ = static_cast<int>((m_sizeMap.cy - 1) * m_fUnitDist);

	if (x < 0 || x > mapMaxX)
		return false;

	if (z < 0 || z > mapMaxZ)
		return false;

	return true;
}
