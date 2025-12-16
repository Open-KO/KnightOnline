// N3Scene.cpp: implementation of the CN3Scene class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Scene.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3Scene::CN3Scene()
{
	m_dwType |= OBJ_SCENE;

	memset(m_pCameras, 0, sizeof(m_pCameras));
	memset(m_pLights, 0, sizeof(m_pLights));

	m_nCameraActive = 0;

	m_fFrmCur = 0.0f; // Animation Frame;
	m_fFrmStart = 0.0f; // 전체 프레임.
	m_fFrmEnd = 1000.0f; // 기본값 프레임.

	m_nCameraCount = 0; 
	m_nLightCount = 0;

	m_bDisableDefaultLight = false;

	m_AmbientLightColor = 0;
}

CN3Scene::~CN3Scene()
{
	int i = 0;
	for(i = 0; i < MAX_SCENE_CAMERA; i++) { if(m_pCameras[i]) { delete m_pCameras[i]; m_pCameras[i] = nullptr; } }
	for(i = 0; i < MAX_SCENE_LIGHT; i++) { if(m_pLights[i]) { delete m_pLights[i]; m_pLights[i] = nullptr; } }
	
	this->ShapeRelease();
	this->ChrRelease();
}

void CN3Scene::Release()
{
	m_nCameraActive = 0;

	m_fFrmCur = 0.0f; // Animation Frame;
	m_fFrmStart = 0.0f; // 전체 프레임.
	m_fFrmEnd = 1000.0f; // 기본값 프레임.

	int i = 0;
	for(i = 0; i < MAX_SCENE_CAMERA; i++) { if(m_pCameras[i]) { delete m_pCameras[i]; m_pCameras[i] = nullptr; } }
	for(i = 0; i < MAX_SCENE_LIGHT; i++) { if(m_pLights[i]) { delete m_pLights[i]; m_pLights[i] = nullptr; } }
	this->ShapeRelease();
	this->ChrRelease();

	m_nCameraCount = 0; 
	m_nLightCount = 0;

	m_bDisableDefaultLight = false;
}

bool CN3Scene::Load(HANDLE hFile)
{
	DWORD dwRWC = 0;
	
	ReadFile(hFile, &m_nCameraActive, 4, &dwRWC, nullptr);
	ReadFile(hFile, &m_fFrmCur, 4, &dwRWC, nullptr); // Animation Frame;
	ReadFile(hFile, &m_fFrmStart, 4, &dwRWC, nullptr); // 전체 프레임.
	ReadFile(hFile, &m_fFrmEnd, 4, &dwRWC, nullptr); // 전체 프레임.

	int i = 0, nL = 0;
	char szName[512] = "";

	int nCC = 0;
	ReadFile(hFile, &nCC, 4, &dwRWC, nullptr); // 카메라..
	for(i = 0; i < nCC; i++)
	{
		ReadFile(hFile, &nL, 4, &dwRWC, nullptr);
		if(nL <= 0) continue;

		ReadFile(hFile, szName, nL, &dwRWC, nullptr);
		szName[nL] = '\0';

		CN3Camera* pCamera = new CN3Camera();
		if(false == pCamera->LoadFromFile(szName))
		{
			delete pCamera;
			continue;
		}

		this->CameraAdd(pCamera);
	}

	int nLC = 0;
	ReadFile(hFile, &nLC, 4, &dwRWC, nullptr); // 카메라..
	for(i = 0; i < nLC; i++) 
	{
		ReadFile(hFile, &nL, 4, &dwRWC, nullptr);
		if(nL <= 0) continue;

		ReadFile(hFile, szName, nL, &dwRWC, nullptr);
		szName[nL] = '\0';

		CN3Light* pLight = new CN3Light();
		if(false == pLight->LoadFromFile(szName))
		{
			delete pLight;
			continue;
		}
		
		this->LightAdd(pLight);
	}

	int nSC = 0;
	ReadFile(hFile, &nSC, 4, &dwRWC, nullptr); // Shapes..
	for(i = 0; i < nSC; i++)
	{
		ReadFile(hFile, &nL, 4, &dwRWC, nullptr);
		if(nL <= 0) continue;

		ReadFile(hFile, szName, nL, &dwRWC, nullptr);
		szName[nL] = '\0';

		CN3Shape* pShape = new CN3Shape();
		if(false == pShape->LoadFromFile(szName))
		{
			delete pShape;
			continue;
		}

		this->ShapeAdd(pShape);
	}

	int nChrC = 0;
	ReadFile(hFile, &nChrC, 4, &dwRWC, nullptr); // 캐릭터
	for(i = 0; i < nChrC; i++)
	{
		ReadFile(hFile, &nL, 4, &dwRWC, nullptr);
		if(nL <= 0) continue;

		ReadFile(hFile, szName, nL, &dwRWC, nullptr);
		szName[nL] = '\0';

		CN3Chr* pChr = new CN3Chr();
		if(false == pChr->LoadFromFile(szName))
		{
			delete pChr;
			continue;
		}

		this->ChrAdd(pChr);
	}

	if(m_nCameraCount <= 0) this->DefaultCameraAdd();
	if(m_nLightCount <= 0) this->DefaultLightAdd();

	return true;
}

bool CN3Scene::Save(HANDLE hFile)
{
	::CreateDirectory("Data", nullptr);
	::CreateDirectory("Chr", nullptr);
	::CreateDirectory("Object", nullptr);
	::CreateDirectory("Item", nullptr);

	DWORD dwRWC = 0;
	
	WriteFile(hFile, &m_nCameraActive, 4, &dwRWC, nullptr);
	WriteFile(hFile, &m_fFrmCur, 4, &dwRWC, nullptr); // Animation Frame;
	WriteFile(hFile, &m_fFrmStart, 4, &dwRWC, nullptr); // 전체 프레임.
	WriteFile(hFile, &m_fFrmEnd, 4, &dwRWC, nullptr); // 전체 프레임.
	
	WriteFile(hFile, &m_nCameraCount, 4, &dwRWC, nullptr); // 카메라..
	for (CN3Camera* camera : m_pCameras)
	{
		int nL = static_cast<int>(camera->FileName().size());
		WriteFile(hFile, &nL, 4, &dwRWC, nullptr);
		WriteFile(hFile, camera->FileName().c_str(), nL, &dwRWC, nullptr);
		camera->SaveToFile();
	}

	WriteFile(hFile, &m_nLightCount, 4, &dwRWC, nullptr); // 카메라..
	for (CN3Light* light : m_pLights)
	{
		int nL = static_cast<int>(light->FileName().size());
		WriteFile(hFile, &nL, 4, &dwRWC, nullptr);
		WriteFile(hFile, light->FileName().c_str(), nL, &dwRWC, nullptr);
		light->SaveToFile();
	}

	int iSC = static_cast<int>(m_Shapes.size());
	WriteFile(hFile, &iSC, 4, &dwRWC, nullptr); // Shapes..
	for (CN3Shape* shape : m_Shapes)
	{
		int nL = static_cast<int>(shape->FileName().size());
		WriteFile(hFile, &nL, 4, &dwRWC, nullptr);
		if (nL <= 0)
			continue;

		WriteFile(hFile, shape->FileName().c_str(), nL, &dwRWC, nullptr);
		shape->SaveToFile();
	}

	int iCC = static_cast<int>(m_Chrs.size());
	WriteFile(hFile, &iCC, 4, &dwRWC, nullptr); // 캐릭터
	for (CN3Chr* chr : m_Chrs)
	{
		int nL = static_cast<int>(chr->FileName().size());
		WriteFile(hFile, &nL, 4, &dwRWC, nullptr);
		if (nL <= 0)
			continue;

		WriteFile(hFile, chr->FileName().c_str(), nL, &dwRWC, nullptr);
		chr->SaveToFile();
	}

	CN3Base::SaveResrc(); // Resource 를 파일로 저장한다..
	return true;
}

void CN3Scene::Render()
{
	int i = 0;
//	for(i = 0; i < m_nCameraCount; i++)
//	{
//		__ASSERT(m_pCameras[i], "Camera pointer is NULL");
//		if(m_nCameraActive != i) m_pCameras[i]->Render();
//	}

//	for(i = 0; i < m_nLightCount; i++)
//	{
//		__ASSERT(m_pLights[i], "Light pointer is NULL");
//		m_pLights[i]->Render(nullptr, 0.5f);
//	}
	s_lpD3DDev->SetRenderState(D3DRS_AMBIENT, m_AmbientLightColor);

	for (CN3Shape* shape : m_Shapes)
		shape->Render();

	for (CN3Chr* chr : m_Chrs)
		chr->Render();
}

void CN3Scene::Tick(float fFrm)
{
	if (FRAME_SELFPLAY == fFrm
		|| fFrm < m_fFrmStart
		|| fFrm > m_fFrmEnd)
	{
		m_fFrmCur += 30.0f / CN3Base::s_fFrmPerSec; // 일정하게 움직이도록 시간에 따라 움직이는 양을 조절..
		if (m_fFrmCur > m_fFrmEnd)
			m_fFrmCur = m_fFrmStart;
	}
	else
	{
		m_fFrmCur = fFrm;
	}

	TickCameras(m_fFrmCur);
	TickLights(m_fFrmCur);
	TickShapes(m_fFrmCur);
	TickChrs(m_fFrmCur);
}

void CN3Scene::TickCameras(float fFrm)
{
	for (int i = 0; i < m_nCameraCount; i++)
	{
		m_pCameras[i]->Tick(m_fFrmCur);
		if (m_nCameraActive == i)
			m_pCameras[i]->Apply(); // 카메라 데이터 값을 적용한다..
	}
}

void CN3Scene::TickLights(float fFrm)
{
	for (int i = 0; i < 8; i++)
		s_lpD3DDev->LightEnable(i, FALSE); // 일단 라이트 다 끄고..

	for (int i = 0; i < m_nLightCount; i++)
	{
		m_pLights[i]->Tick(m_fFrmCur);
		m_pLights[i]->Apply(); // 라이트 적용
	}

	// 라이트가 항상 카메라를 따라오게 만든다..
	if (!m_bDisableDefaultLight)
	{
		__Vector3 vDir = s_CameraData.vAt - s_CameraData.vEye;
		vDir.Normalize();

		__ColorValue crLgt = { 1.0f, 1.0f, 1.0f, 1.0f };

		CN3Light::__Light lgt;
		lgt.InitDirection(7, vDir, crLgt);

		s_lpD3DDev->LightEnable(7, TRUE);
		s_lpD3DDev->SetLight(7, lgt.toD3D());
	}

	// Ambient Light 바꾸기..
//	uint32_t dwAmbient =	0xff000000 | 
//						(((uint32_t)(m_pLights[i]->m_Data.Diffuse.r * 255 * 0.5f)) << 16) | 
//						(((uint32_t)(m_pLights[i]->m_Data.Diffuse.g * 255 * 0.5f)) << 8) | 
//						(((uint32_t)(m_pLights[i]->m_Data.Diffuse.b * 255 * 0.5f)) << 0);
//	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_AMBIENT, dwAmbient);
}

void CN3Scene::TickShapes(float fFrm)
{
	for (CN3Shape* shape : m_Shapes)
		shape->Tick(m_fFrmCur);
}

void CN3Scene::TickChrs(float fFrm)
{
	for (CN3Chr* chr : m_Chrs)
		chr->Tick(m_fFrmCur);
}

int CN3Scene::CameraAdd(CN3Camera* pCamera)
{
	if (m_nCameraCount < 0
		|| m_nCameraCount >= MAX_SCENE_CAMERA)
		return -1;

	if (pCamera == nullptr)
		return -1;

	delete m_pCameras[m_nCameraCount];
	m_pCameras[m_nCameraCount] = pCamera;

	m_nCameraCount++;
	return m_nCameraCount;
}

void CN3Scene::CameraDelete(int iIndex)
{
	if(iIndex < 0 || iIndex >= m_nCameraCount) return;

	delete m_pCameras[iIndex];
	m_pCameras[iIndex] = nullptr;
	
	m_nCameraCount--;
	for(int i = iIndex; i < m_nCameraCount; i++) m_pCameras[i] = m_pCameras[i+1];
	m_pCameras[m_nCameraCount] = nullptr;
}

void CN3Scene::CameraDelete(CN3Camera* pCamera)
{
	for (int i = 0; i < m_nCameraCount; i++)
	{
		if (m_pCameras[i] == pCamera)
		{
			CameraDelete(i);
			break;
		}
	}
}

void CN3Scene::CameraSetActive(int iIndex)
{
	if (iIndex < 0
		|| iIndex >= m_nCameraCount)
		return;

	m_nCameraActive = iIndex;
}

int CN3Scene::LightAdd(CN3Light* pLight)
{
	if (pLight == nullptr)
		return -1;

	delete m_pLights[m_nLightCount];
	m_pLights[m_nLightCount] = pLight;

	m_nLightCount++;
	return m_nLightCount;
}

void CN3Scene::LightDelete(int iIndex)
{
	if (iIndex < 0
		|| iIndex >= m_nLightCount)
		return;

	delete m_pLights[iIndex];
	m_pLights[iIndex] = nullptr;

	m_nLightCount--;
	for (int i = iIndex; i < m_nLightCount; i++)
		m_pLights[i] = m_pLights[i + 1];

	m_pLights[m_nLightCount] = nullptr;
}

void CN3Scene::LightDelete(CN3Light* pLight)
{
	for (int i = 0; i < m_nLightCount; i++)
	{
		if (m_pLights[i] == pLight)
		{
			LightDelete(i);
			break;
		}
	}
}

int CN3Scene::ShapeAdd(CN3Shape* pShape)
{
	if (pShape == nullptr)
		return -1;

	m_Shapes.push_back(pShape);
	return static_cast<int>(m_Shapes.size());
}

void CN3Scene::ShapeDelete(int iIndex)
{
	if (iIndex < 0
		|| iIndex >= static_cast<int>(m_Shapes.size()))
		return;

	auto it = m_Shapes.begin();
	std::advance(it, iIndex);

	delete *it;
	m_Shapes.erase(it);
}

void CN3Scene::ShapeDelete(CN3Shape* pShape)
{
	auto it = m_Shapes.begin(), itEnd = m_Shapes.end();
	for (; it != itEnd; it++)
	{
		CN3Shape* pShapeSrc = *it;
		if (pShapeSrc == pShape)
		{
			delete pShapeSrc;
			it = m_Shapes.erase(it);
			return;
		}
	}
}

void CN3Scene::ShapeRelease()
{
	for (CN3Shape* shape : m_Shapes)
		delete shape;

	m_Shapes.clear();
}

int CN3Scene::ChrAdd(CN3Chr* pChr)
{
	if (pChr == nullptr)
		return -1;

	m_Chrs.push_back(pChr);
	return static_cast<int>(m_Chrs.size());
}

void CN3Scene::ChrDelete(int iIndex)
{
	if (iIndex < 0
		|| iIndex >= static_cast<int>(m_Chrs.size()))
		return;

	auto it = m_Chrs.begin();
	std::advance(it, iIndex);

	delete *it;
	m_Chrs.erase(it);
}

void CN3Scene::ChrDelete(CN3Chr* pChr)
{
	it_Chr it = m_Chrs.begin(), itEnd = m_Chrs.end();
	CN3Chr* pChrSrc;
	for(; it != itEnd; it++);
	{
		pChrSrc = *it;
		if(pChr == pChrSrc) 
		{
			delete pChrSrc;
			m_Chrs.erase(it);
			return;
		}
	}
}

void CN3Scene::ChrRelease()
{
	for (CN3Chr* pChr : m_Chrs)
		delete pChr;
	m_Chrs.clear();
}

bool CN3Scene::LoadDataAndResourcesFromFile(const std::string& szFN)
{
	if(szFN.empty()) return false;

	char szPath[512] = "", szDrv[_MAX_DRIVE] = "", szDir[_MAX_DIR] = "";
	::_splitpath(szFN.c_str(), szDrv, szDir, nullptr, nullptr);
	::_makepath(szPath, szDrv, szDir, nullptr, nullptr);

	this->Release();
	this->PathSet(szPath);
	return LoadFromFile(szFN);
}

bool CN3Scene::SaveDataAndResourcesToFile(const std::string& szFN)
{
	if(szFN.empty()) return false;

	char szPath[512] = "", szDrv[_MAX_DRIVE] = "", szDir[_MAX_DIR] = "";
	::_splitpath(szFN.c_str(), szDrv, szDir, nullptr, nullptr);
	::_makepath(szPath, szDrv, szDir, nullptr, nullptr);

	this->PathSet(szPath);
	return SaveToFile(szFN);
}

void CN3Scene::DefaultCameraAdd()
{
	CN3Camera* pCamera = new CN3Camera();
	pCamera->m_szName = "DefaultCamera";
	pCamera->FileNameSet("Data\\DefaultCamera.N3Camera");
	this->CameraAdd(pCamera);
}
void CN3Scene::DefaultLightAdd()
{
	// Light 초기화..
	CN3Light* pLight = new CN3Light();
	pLight->m_szName = "DefaultLight";
	pLight->FileNameSet("Data\\DefaultLight.N3Light");
	int nLight = this->LightAdd(pLight) - 1;

	__ColorValue ltColor = { 0.7f, 0.7f, 0.7f, 1.0f};
	pLight->m_Data.InitDirection(0, { -1.0f, -1.0f, 0.5f }, ltColor);
	pLight->PosSet(1000.0f, 1000.0f, -1000.0f);
	pLight->m_Data.bOn = TRUE;
	pLight->m_Data.nNumber = nLight;
}

