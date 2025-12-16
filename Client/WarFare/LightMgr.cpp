// LightMgr.cpp: implementation of the CLightMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LightMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLightMgr::CLightMgr()
{
	m_Lights.clear();
	for(int i=0;i<LGT_MAX;i++) m_pActiveLight[i] = nullptr;
}

CLightMgr::~CLightMgr()
{
	std::list<CN3Light*>::iterator it;
	for(it=m_Lights.begin(); it!=m_Lights.end(); it++)
	{
		CN3Light* pLgt = (*it);
		delete pLgt;
	}
	m_Lights.clear();
	for(int i=0;i<LGT_MAX;i++)
	{
		if(m_pActiveLight[i]) delete m_pActiveLight[i];
	}
}

void CLightMgr::Release()
{
	/////////////////////////////////////////////
	// Release...
	std::list<CN3Light*>::iterator it;
	for(it=m_Lights.begin(); it!=m_Lights.end(); it++)
	{
		CN3Light* pLgt = (*it);
		delete pLgt;
	}
	m_Lights.clear();
	for(int i=0;i<LGT_MAX;i++)
	{
		if(m_pActiveLight[i]) delete m_pActiveLight[i];
		m_pActiveLight[i] = nullptr;
	}
	//	Release..
	/////////////////////////////////////////////
		
	///////////////////////////////////////////////////////////////
	// 기본 라이트 세팅
	__ColorValue crLgt;

	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.8f;
	CN3Light* pLightGlobal = new CN3Light(); // 전체를 비출 라이트..
	pLightGlobal->m_Data.InitDirection(LGT_DEFAULT0, { 0, -1, 0 }, crLgt);
	m_pActiveLight[LGT_DEFAULT0] = pLightGlobal;


	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.5f;
	CN3Light* pLightGlobal2 = new CN3Light(); // 반대 편에서 전체를 비출 라이트..
	pLightGlobal2->m_Data.InitDirection(LGT_DEFAULT1, { 0, 1, 0}, crLgt);
	m_pActiveLight[LGT_DEFAULT1] = pLightGlobal2;

	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.3f;
	CN3Light* pLight = new CN3Light(); // 카메라와 붙어 다닌다...
	pLight->m_Data.InitPoint(LGT_DEFAULT2, { 0, 0, 0 }, crLgt, 32.0f);
	m_pActiveLight[LGT_DEFAULT2] = pLight;
	// 기본 라이트 세팅
	///////////////////////////////////////////////////////////////	
}

void CLightMgr::Tick()
{
	int i;
	//거리에 따라 추려내고...
	int NumSlotEmpty = 0;
	float LimitLeft, LimitRight, LimitUp, LimitDown;
	LimitLeft = CN3Base::s_CameraData.vEye.x - LIGHT_VALIDRANGE;
	LimitRight = CN3Base::s_CameraData.vEye.x + LIGHT_VALIDRANGE;
	LimitUp = CN3Base::s_CameraData.vEye.z + LIGHT_VALIDRANGE;
	LimitDown = CN3Base::s_CameraData.vEye.z - LIGHT_VALIDRANGE;

	__Vector3 vPosTmp;
	for(i=LGT_ADDITIONAL0;i<LGT_MAX;i++)
	{
		if(!m_pActiveLight[i])
		{
			NumSlotEmpty++;
			continue;
		}

		vPosTmp = m_pActiveLight[i]->Pos();
		if(vPosTmp.x < LimitLeft || vPosTmp.x > LimitRight || vPosTmp.z < LimitDown || vPosTmp.z > LimitUp)
		{
			m_pActiveLight[i]->m_Data.bOn = false;
			m_pActiveLight[i]->Apply();

			AddLight(m_pActiveLight[i]);
			m_pActiveLight[i] = nullptr;
			NumSlotEmpty++;

			continue;
		}
	}

	std::list<CN3Light*>::iterator it = m_Lights.begin();
	while(NumSlotEmpty>0 && it!=m_Lights.end())
	{
		CN3Light* pLgt = (*it);
		vPosTmp = pLgt->Pos();
		if(vPosTmp.x > LimitLeft && vPosTmp.x < LimitRight && vPosTmp.z > LimitDown && vPosTmp.z < LimitUp)
		{
			for(i=LGT_ADDITIONAL0;i<LGT_MAX;i++)
			{
				if(!m_pActiveLight[i])
				{
					m_pActiveLight[i] = pLgt;
					m_pActiveLight[i]->m_Data.bOn = true;
					m_pActiveLight[i]->m_Data.nNumber = i;
					NumSlotEmpty--;
					break;
				}
			}
			it = m_Lights.erase(it);
		}
		else it++;
	}

	//tick돌려라..
	for(i=0;i<LGT_MAX;i++)
	{
		if(m_pActiveLight[i])
		{
			m_pActiveLight[i]->Tick();
			m_pActiveLight[i]->Apply();
		}
	}
}

void CLightMgr::AddLight(CN3Light* pLgt)
{
	if(!pLgt) return;
	m_Lights.push_back(pLgt);	
}

void CLightMgr::LoadZoneLight(const char* szFN)
{
	if(!szFN) return;
	
	DWORD dwRWC;
	HANDLE hFile = CreateFile(szFN, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(INVALID_HANDLE_VALUE == hFile) return;

	int iVersion;
	ReadFile(hFile, &iVersion, sizeof(int), &dwRWC, nullptr);

	int cnt;
	ReadFile(hFile, &cnt, sizeof(int), &dwRWC, nullptr);
	for(int i=0;i<cnt;i++)
	{
		CN3Light* pLgt = new CN3Light;
		pLgt->m_iFileFormatVersion = N3FORMAT_VER_DEFAULT;

		pLgt->Load(hFile);
		AddLight(pLgt);
	}
	CloseHandle(hFile);
}
