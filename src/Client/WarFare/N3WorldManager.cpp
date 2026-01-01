// N3WorldManager.cpp: implementation of the CN3WorldManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3WorldManager.h"
#include "N3TerrainManager.h"
#include "DungeonManager.h"
#include "APISocket.h"
#include "GameProcedure.h"
#include "PlayerMySelf.h"
#include "GameEng.h"

//#include "text_resources.h"

#include <N3Base/LogWriter.h>
#include <N3Base/N3SndObj.h>
#include <N3Base/N3SndMgr.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3WorldManager::CN3WorldManager()
{
	m_pActiveWorld = nullptr;
	m_bIndoor = true;
}

CN3WorldManager::~CN3WorldManager()
{
	if (m_pActiveWorld)
		delete m_pActiveWorld;
	m_pActiveWorld = nullptr;
}

void CN3WorldManager::InitWorld(int iZoneID, const __Vector3& vPosPlayer)
{
	CLogWriter::Write("CN3WorldManager::InitWorld Pre delete"); // TmpLog_11_22

	if (m_pActiveWorld)
		delete m_pActiveWorld;

	// Zone 선택..
	if (iZoneID != 51)	// N3Terrain..
	{
		CLogWriter::Write("CN3WorldManager::InitWorld Pre new Terrain ZoneID({})", iZoneID); // TmpLog_11_22

		m_pActiveWorld = new CN3TerrainManager();	
		m_bIndoor = false;
	}
	else
	{
		CLogWriter::Write("CN3WorldManager::InitWorld Pre new Dungeon ZoneID({})", iZoneID); // TmpLog_11_22

		m_pActiveWorld = new CDungeonManager();
		m_bIndoor = true;
	}

	// Zone 초기화..
	m_pActiveWorld->InitWorld(iZoneID, vPosPlayer);
}

void CN3WorldManager::Tick()
{
	if (m_pActiveWorld)
		m_pActiveWorld->Tick();
}







