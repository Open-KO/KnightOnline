// N3FXBundle.cpp: implementation of the CN3FXBundle class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXBundle.h"
#include "N3FXPartParticles.h"
#include "N3FXPartBillBoard.h"
#include "N3FXPartMesh.h"
#include "N3FXPartBottomBoard.h"

#include "N3SndMgr.h"
#include "N3SndObj.h"

float CN3FXBundle::m_fEffectSndDist = 48.0f;

CN3FXBundle::CN3FXBundle()
{
	m_iVersion = SUPPORTED_BUNDLE_VERSION;
	for (int i = 0; i < MAX_FX_PART; i++)
		m_pPart[i] = nullptr;
	m_fLife0  = 0.0f;

	m_dwState = FX_BUNDLE_STATE_DEAD;
	m_fLife   = 0.0f;

	m_vPos.Set(0.0f, 0.0f, 0.0f);
	m_vDir.Set(0.0f, 0.0f, 1.0f);
	m_vDestPos.Set(0.0f, 0.0f, 0.0f);

	m_iMoveType    = FX_BUNDLE_MOVE_NONE;
	m_fVelocity    = 0.0f;

	m_iSourceID    = 0;
	m_iTargetID    = 0;
	m_iTargetJoint = 0;
	m_iSourceJoint = 0;

	m_bDependScale = false;

	//m_vTargetScale.Set(1,1,1);
	m_fTargetScale = 1.0f;

	m_vPrePos      = {};

	m_bStatic      = false;

	m_pSndObj      = nullptr;
}

CN3FXBundle::~CN3FXBundle()
{
	m_strName.erase();

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i])
		{
			if (m_pPart[i]->pPart)
			{
				delete m_pPart[i]->pPart;
				m_pPart[i]->pPart = nullptr;
			}
			delete m_pPart[i];
			m_pPart[i] = nullptr;
		}
	}

#ifdef _N3GAME
	if (m_pSndObj)
		CN3Base::s_SndMgr.ReleaseObj(&m_pSndObj);
#endif
}

//
//	decode script file..
//	스크립트 파일 읽고 해석시킴...
//
#ifdef _N3TOOL
bool CN3FXBundle::DecodeScriptFile(const char* lpPathName)
{
	FILE* stream = fopen(lpPathName, "r");
	if (stream == nullptr)
		return false;

	char szGamePathName[_MAX_PATH];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpPathName, szDrive, szDir, szFName, szExt);
	_makepath(szGamePathName, szDrive, szDir, szFName, "fxb");

	CN3BaseFileAccess::FileNameSet(szGamePathName);

	char szLine[512] = "", szCommand[80] = "", szBuf[4][80] = { "", "", "", "" };
	char* pResult   = fgets(szLine, 512, stream);
	int argsScanned = sscanf(
		szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
	if (argsScanned <= 0)
	{
		fclose(stream);
		return false;
	}

	if (lstrcmpi(szCommand, "<n3fxbundle>"))
	{
		fclose(stream);
		return false;
	}

	while (!feof(stream))
	{
		char* pResult = fgets(szLine, 512, stream);
		if (pResult == nullptr)
			continue;

		ZeroMemory(szCommand, 80);
		ZeroMemory(szBuf[0], 80);
		ZeroMemory(szBuf[1], 80);
		ZeroMemory(szBuf[2], 80);
		ZeroMemory(szBuf[3], 80);

		argsScanned = sscanf(
			szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		if (argsScanned <= 0)
			continue;

		if (lstrcmpi(szCommand, "<name>") == 0)
		{
			m_strName = szBuf[0];
			continue;
		}

		if (lstrcmpi(szCommand, "<part>") == 0)
		{
			//full path 만들기..
			std::string szFullPath     = fmt::format("{}{}", CN3Base::PathGet(), szBuf[0]);

			FXPARTWITHSTARTTIME* pPart = new FXPARTWITHSTARTTIME;
			pPart->fStartTime          = static_cast<float>(atof(szBuf[1]));

			pPart->pPart               = SetPart(szFullPath.c_str());

			if (pPart->pPart == nullptr)
			{
				delete pPart;
				continue;
			}

			for (int i = 0; i < MAX_FX_PART; i++)
			{
				if (m_pPart[i] == nullptr)
				{
					m_pPart[i] = pPart;
					break;
				}
			}

			continue;
		}

		if (lstrcmpi(szCommand, "<velocity>") == 0)
		{
			m_fVelocity = static_cast<float>(atof(szBuf[0]));
			continue;
		}

		if (lstrcmpi(szCommand, "<depend_scale>") == 0)
		{
			if (lstrcmpi(szBuf[0], "true") == 0)
				m_bDependScale = true;
			else
				m_bDependScale = false;

			continue;
		}

		if (lstrcmpi(szCommand, "<Static_Pos>") == 0)
		{
			if (lstrcmpi(szBuf[0], "true") == 0)
				m_bStatic = true;
			else
				m_bStatic = false;

			continue;
		}
	}

	fclose(stream);

	Init();

	return true;
}
#endif // end of _N3TOOL

//
//	GetPartType...
//	파트의 파일이름으로 타입을 알아내자..
//
#ifdef _N3TOOL
CN3FXPartBase* CN3FXBundle::SetPart(const char* pFileName)
{
	int PartType = FX_PART_TYPE_NONE;

	FILE* stream = fopen(pFileName, "r");
	if (!stream)
		return nullptr;

	char szLine[512] = "", szCommand[80] = "", szBuf[4][80] = { "", "", "", "" };
	char* pResult = fgets(szLine, 512, stream);
	sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

	if (lstrcmpi(szCommand, "<n3fxPart>"))
	{
		fclose(stream);
		return nullptr;
	}

	while (!feof(stream))
	{
		char* pResult = fgets(szLine, 512, stream);
		if (pResult == nullptr)
			continue;

		ZeroMemory(szCommand, 80);
		ZeroMemory(szBuf[0], 80);
		ZeroMemory(szBuf[1], 80);
		ZeroMemory(szBuf[2], 80);
		ZeroMemory(szBuf[3], 80);
		sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

		if (lstrcmpi(szCommand, "<type>") == 0)
		{
			if (lstrcmpi(szBuf[0], "particle") == 0)
				PartType = FX_PART_TYPE_PARTICLE;
			else if (lstrcmpi(szBuf[0], "board") == 0)
				PartType = FX_PART_TYPE_BOARD;
			else if (lstrcmpi(szBuf[0], "mesh") == 0)
				PartType = FX_PART_TYPE_MESH;
			else if (lstrcmpi(szBuf[0], "ground") == 0)
				PartType = FX_PART_TYPE_BOTTOMBOARD;
			//^^v 더 넣을꺼 있으면 넣어라..
		}
	}
	fclose(stream);

	CN3FXPartBase* pPart;
	if (PartType == FX_PART_TYPE_PARTICLE)
	{
		pPart                 = new CN3FXPartParticles;
		pPart->m_pRefBundle   = this;
		pPart->m_pRefPrevPart = nullptr;
		pPart->DecodeScriptFile(pFileName);
		return pPart;
	}
	else if (PartType == FX_PART_TYPE_BOARD)
	{
		pPart                 = new CN3FXPartBillBoard;
		pPart->m_pRefBundle   = this;
		pPart->m_pRefPrevPart = nullptr;
		pPart->DecodeScriptFile(pFileName);
		return pPart;
	}
	else if (PartType == FX_PART_TYPE_MESH)
	{
		pPart                 = new CN3FXPartMesh;
		pPart->m_pRefBundle   = this;
		pPart->m_pRefPrevPart = nullptr;
		pPart->DecodeScriptFile(pFileName);
		return pPart;
	}
	else if (PartType == FX_PART_TYPE_BOTTOMBOARD)
	{
		pPart                 = new CN3FXPartBottomBoard;
		pPart->m_pRefBundle   = this;
		pPart->m_pRefPrevPart = nullptr;
		pPart->DecodeScriptFile(pFileName);
		return pPart;
	}
	return nullptr;
}
#endif // end of _N3TOOL

//
//	Init..
//
void CN3FXBundle::Init()
{
	m_fLife   = 0.0f;
	m_dwState = FX_BUNDLE_STATE_DEAD;

	m_vPos.Set(0.0f, 0.0f, 0.0f);
	m_vDir.Set(0.0f, 0.0f, 1.0f);

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] && m_pPart[i]->pPart)
		{
			m_pPart[i]->pPart->Init();
		}
	}
}

int CN3FXBundle::GetPartCountForVersion() const
{
	if (m_iVersion < 0)
		return 0;

	if (m_iVersion == 0)
		return MAX_FX_PART_V0;

	return MAX_FX_PART_V1;
}

//
//
//
bool CN3FXBundle::Load(File& file)
{
	file.Read(&m_iVersion, sizeof(int));

	// NOTE: This should ideally just be an assertion, but we'll continue to allow it to run
	// and otherwise be broken for now.
#if defined(_DEBUG)
	if (m_iVersion > SUPPORTED_BUNDLE_VERSION)
	{
		TRACE("!!! WARNING: CN3FXBundle::Load({}) encountered bundle version {}. Needs support!",
			FileName(), m_iVersion);
	}
#endif

	file.Read(&m_fLife0, sizeof(float));
	if (m_fLife0 > 10.0f)
		m_fLife0 = 10.0f;

	file.Read(&m_fVelocity, sizeof(float));
	file.Read(&m_bDependScale, sizeof(bool));

	const int iPartCount = GetPartCountForVersion();
	for (int i = 0; i < iPartCount; i++)
	{
		int iType = FX_PART_TYPE_NONE;
		file.Read(&iType, sizeof(int));

		if (iType == FX_PART_TYPE_NONE)
			continue;

		CN3FXPartBase* part = AllocatePart(iType);
		if (part == nullptr)
		{
			TRACE("!!! WARNING: CN3FXBundle::Load({}) encountered invalid part type {} at index "
				  "{}. Ending parsing here.",
				FileName(), iType, i);
			break;
		}

		float fStartTime = 0.0f;
		file.Read(&fStartTime, sizeof(float));

		m_pPart[i]                        = new FXPARTWITHSTARTTIME;
		m_pPart[i]->fStartTime            = fStartTime;
		m_pPart[i]->pPart                 = part;
		m_pPart[i]->pPart->m_pRefBundle   = this;
		m_pPart[i]->pPart->m_pRefPrevPart = nullptr;
		m_pPart[i]->pPart->m_iType        = iType;
		m_pPart[i]->pPart->Load(file);
	}

	if (m_iVersion >= 2)
		file.Read(&m_bStatic, sizeof(bool));

	return true;
}

CN3FXPartBase* CN3FXBundle::AllocatePart(int iPartType) const
{
	switch (iPartType)
	{
		case FX_PART_TYPE_PARTICLE:
			return new CN3FXPartParticles();

		case FX_PART_TYPE_BOARD:
			return new CN3FXPartBillBoard();

		case FX_PART_TYPE_MESH:
			return new CN3FXPartMesh();

		case FX_PART_TYPE_BOTTOMBOARD:
			return new CN3FXPartBottomBoard();

		default:
			return nullptr;
	}
}

//
//
//
bool CN3FXBundle::Save(File& file)
{
	file.Write(&m_iVersion, sizeof(int));
	file.Write(&m_fLife0, sizeof(float));
	file.Write(&m_fVelocity, sizeof(float));

	file.Write(&m_bDependScale, sizeof(bool));

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] != nullptr && m_pPart[i]->pPart != nullptr)
		{
			file.Write(&m_pPart[i]->pPart->m_iType, sizeof(int));
			file.Write(&m_pPart[i]->fStartTime, sizeof(float));
			m_pPart[i]->pPart->Save(file);
		}
		else
		{
			int Type = FX_PART_TYPE_NONE;
			file.Write(&Type, sizeof(int));
		}
	}

	file.Write(&m_bStatic, sizeof(bool));

	return true;
}

//
//	Tick...
//
bool CN3FXBundle::Tick()
{
	if (m_dwState == FX_BUNDLE_STATE_DEAD)
		return false;

	m_fLife += CN3Base::s_fSecPerFrm;

	if (m_dwState == FX_BUNDLE_STATE_DYING || m_dwState == FX_BUNDLE_STATE_LIVE)
	{
		if (CheckAllPartsDead() || (m_fLife0 != 0.0f && m_fLife > m_fLife0))
		{
			m_dwState = FX_BUNDLE_STATE_DEAD;
			Init();
			return false;
		}
	}

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] && m_pPart[i]->pPart)
		{
			if (m_pPart[i]->fStartTime <= m_fLife
				&& m_pPart[i]->pPart->m_dwState == FX_PART_STATE_READY)
			{
				m_pPart[i]->pPart->Start();
			}
			m_pPart[i]->pPart->Tick();
		}
	}
	return true;
}

//
//	Render...
//
void CN3FXBundle::Render()
{
	if (m_dwState == FX_BUNDLE_STATE_DEAD)
		return;

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] && m_pPart[i]->pPart && (m_pPart[i]->pPart->m_dwState != FX_PART_STATE_DEAD)
			&& (m_pPart[i]->pPart->m_dwState != FX_PART_STATE_READY))
		{
			m_pPart[i]->pPart->Render();
		}
	}
}

//
//	Trigger...
//
void CN3FXBundle::Trigger(int iSourceID, int iTargetID, int iTargetJoint, int iSndID)
{
	m_dwState      = FX_BUNDLE_STATE_LIVE;

	m_iSourceID    = iSourceID;
	m_iTargetID    = iTargetID;
	m_iTargetJoint = iTargetJoint;

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] && m_pPart[i]->pPart)
		{
			m_pPart[i]->pPart->m_dwState = FX_PART_STATE_READY;
		}
	}

#ifdef _N3GAME
	if (iSndID >= 0)
	{
		float fDist = (s_CameraData.vEye - m_vPos).Magnitude();
		//		if(fDist < 48.0f)
		if (fDist < m_fEffectSndDist) //this_Snd
			CN3Base::s_SndMgr.PlayOnceAndRelease(iSndID, &m_vPos);
	}

	//	if(iSndID >= 0 && nullptr == m_pSndObj)
	//		m_pSndObj = CN3Base::s_SndMgr.CreateObj(iSndID);
	//	if(m_pSndObj) m_pSndObj->Play(&m_vPos);

#endif
}

//
//	Stop...
//	멈추는 단계를 시작한다..
//	정말 멈추는 기능은 tick에서 모든 파트가 다 죽었다고 판단될때 tick에서 수행한다.
//
void CN3FXBundle::Stop(bool immediately)
{
	if (m_dwState == FX_BUNDLE_STATE_DEAD)
		return;

	if (!immediately)
	{
		m_dwState = FX_BUNDLE_STATE_DYING;
		for (int i = 0; i < MAX_FX_PART; i++)
		{
			if (m_pPart[i] && m_pPart[i]->pPart)
			{
				m_pPart[i]->pPart->Stop();
			}
		}
	}
	else
	{
		m_dwState = FX_BUNDLE_STATE_DYING;
		Init();
	}
}

//
//	check all parts are dead..
//	if dead return true..
//
bool CN3FXBundle::CheckAllPartsDead()
{
	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i] && m_pPart[i]->pPart)
		{
			if (m_pPart[i]->pPart->m_dwState != FX_PART_STATE_DEAD)
				return false;
		}
	}
	return true;
}

CN3FXPartBase* CN3FXBundle::GetPart(int i)
{
	if (i < 0 || i >= MAX_FX_PART)
		return nullptr;

	if (m_pPart[i])
		return m_pPart[i]->pPart;

	return nullptr;
}

float CN3FXBundle::GetPartSTime(int i)
{
	if (i < 0 || i >= MAX_FX_PART)
		return -1.0f;

	if (m_pPart[i])
		return m_pPart[i]->fStartTime;

	return -1.0f;
}

void CN3FXBundle::SetPartSTime(int i, float time)
{
	if (i < 0 || i >= MAX_FX_PART)
		return;

	if (m_pPart[i])
		m_pPart[i]->fStartTime = time;
	return;
}

void CN3FXBundle::Duplicate(CN3FXBundle* pDestBundle)
{
	pDestBundle->FileNameSet(this->FileName());

	pDestBundle->m_iVersion     = m_iVersion;
	pDestBundle->m_fLife0       = m_fLife0;

	pDestBundle->m_fVelocity    = m_fVelocity;
	pDestBundle->m_bDependScale = m_bDependScale;
	pDestBundle->m_bStatic      = pDestBundle->m_bStatic;

	for (int i = 0; i < MAX_FX_PART; i++)
	{
		if (m_pPart[i])
		{
			if (m_pPart[i]->pPart->m_iType == FX_PART_TYPE_PARTICLE)
			{
				pDestBundle->m_pPart[i]             = new FXPARTWITHSTARTTIME;

				pDestBundle->m_pPart[i]->fStartTime = m_pPart[i]->fStartTime;

				pDestBundle->m_pPart[i]->pPart      = new CN3FXPartParticles;
				CN3FXPartParticles* pPart = (CN3FXPartParticles*) pDestBundle->m_pPart[i]->pPart;

				pPart->m_pRefBundle       = pDestBundle;
				pPart->m_pRefPrevPart     = nullptr;
				pPart->m_iType            = FX_PART_TYPE_PARTICLE;

				pPart->Duplicate((CN3FXPartParticles*) m_pPart[i]->pPart);
			}
			else if (m_pPart[i]->pPart->m_iType == FX_PART_TYPE_BOARD)
			{
				pDestBundle->m_pPart[i]             = new FXPARTWITHSTARTTIME;

				pDestBundle->m_pPart[i]->fStartTime = m_pPart[i]->fStartTime;

				pDestBundle->m_pPart[i]->pPart      = new CN3FXPartBillBoard;
				CN3FXPartBillBoard* pPart = (CN3FXPartBillBoard*) pDestBundle->m_pPart[i]->pPart;

				pPart->m_pRefBundle       = pDestBundle;
				pPart->m_pRefPrevPart     = nullptr;
				pPart->m_iType            = FX_PART_TYPE_BOARD;

				pPart->Duplicate((CN3FXPartBillBoard*) m_pPart[i]->pPart);
			}
			else if (m_pPart[i]->pPart->m_iType == FX_PART_TYPE_MESH)
			{
				pDestBundle->m_pPart[i]             = new FXPARTWITHSTARTTIME;

				pDestBundle->m_pPart[i]->fStartTime = m_pPart[i]->fStartTime;

				pDestBundle->m_pPart[i]->pPart      = new CN3FXPartMesh;
				CN3FXPartMesh* pPart  = (CN3FXPartMesh*) pDestBundle->m_pPart[i]->pPart;

				pPart->m_pRefBundle   = pDestBundle;
				pPart->m_pRefPrevPart = nullptr;
				pPart->m_iType        = FX_PART_TYPE_MESH;

				pPart->Duplicate((CN3FXPartMesh*) m_pPart[i]->pPart);
			}
			else if (m_pPart[i]->pPart->m_iType == FX_PART_TYPE_BOTTOMBOARD)
			{
				pDestBundle->m_pPart[i]             = new FXPARTWITHSTARTTIME;

				pDestBundle->m_pPart[i]->fStartTime = m_pPart[i]->fStartTime;

				pDestBundle->m_pPart[i]->pPart      = new CN3FXPartBottomBoard;
				CN3FXPartBottomBoard* pPart = (CN3FXPartBottomBoard*) pDestBundle->m_pPart[i]
												  ->pPart;

				pPart->m_pRefBundle   = pDestBundle;
				pPart->m_pRefPrevPart = nullptr;
				pPart->m_iType        = FX_PART_TYPE_BOTTOMBOARD;

				pPart->Duplicate((CN3FXPartBottomBoard*) m_pPart[i]->pPart);
			}
		}
	}
}
