// N3Scene.cpp: implementation of the CN3Scene class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Scene.h"

CN3Scene::CN3Scene()
{
	m_dwType |= OBJ_SCENE;

	memset(&m_pCameras, 0, sizeof(m_pCameras));
	memset(&m_pLights, 0, sizeof(m_pLights));

	m_nCameraActive        = 0;

	m_fFrmCur              = 0.0f;    // Animation Frame;
	m_fFrmStart            = 0.0f;    // 전체 프레임.
	m_fFrmEnd              = 1000.0f; // 기본값 프레임.

	m_nCameraCount         = 0;
	m_nLightCount          = 0;

	m_bDisableDefaultLight = false;

	m_AmbientLightColor    = 0;
}

CN3Scene::~CN3Scene()
{
	int i = 0;
	for (i = 0; i < MAX_SCENE_CAMERA; i++)
	{
		if (m_pCameras[i])
		{
			delete m_pCameras[i];
			m_pCameras[i] = nullptr;
		}
	}
	for (i = 0; i < MAX_SCENE_LIGHT; i++)
	{
		if (m_pLights[i])
		{
			delete m_pLights[i];
			m_pLights[i] = nullptr;
		}
	}

	this->ShapeRelease();
	this->ChrRelease();
}

void CN3Scene::Release()
{
	m_nCameraActive = 0;

	m_fFrmCur       = 0.0f;    // Animation Frame;
	m_fFrmStart     = 0.0f;    // 전체 프레임.
	m_fFrmEnd       = 1000.0f; // 기본값 프레임.

	int i           = 0;
	for (i = 0; i < MAX_SCENE_CAMERA; i++)
	{
		if (m_pCameras[i])
		{
			delete m_pCameras[i];
			m_pCameras[i] = nullptr;
		}
	}
	for (i = 0; i < MAX_SCENE_LIGHT; i++)
	{
		if (m_pLights[i])
		{
			delete m_pLights[i];
			m_pLights[i] = nullptr;
		}
	}
	this->ShapeRelease();
	this->ChrRelease();

	m_nCameraCount         = 0;
	m_nLightCount          = 0;

	m_bDisableDefaultLight = false;
}

bool CN3Scene::Load(File& file)
{
	file.Read(&m_nCameraActive, 4);
	file.Read(&m_fFrmCur, 4);   // Animation Frame;
	file.Read(&m_fFrmStart, 4); // 전체 프레임.
	file.Read(&m_fFrmEnd, 4);   // 전체 프레임.

	int i = 0, nL = 0;
	char szName[512] = "";

	int nCC          = 0;
	file.Read(&nCC, 4); // 카메라..
	for (i = 0; i < nCC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL]         = '\0';

		CN3Camera* pCamera = new CN3Camera();
		if (false == pCamera->LoadFromFile(szName))
		{
			delete pCamera;
			continue;
		}

		this->CameraAdd(pCamera);
	}

	int nLC = 0;
	file.Read(&nLC, 4); // 카메라..
	for (i = 0; i < nLC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL]       = '\0';

		CN3Light* pLight = new CN3Light();
		if (false == pLight->LoadFromFile(szName))
		{
			delete pLight;
			continue;
		}

		this->LightAdd(pLight);
	}

	int nSC = 0;
	file.Read(&nSC, 4); // Shapes..
	for (i = 0; i < nSC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL]       = '\0';

		CN3Shape* pShape = new CN3Shape();
		if (false == pShape->LoadFromFile(szName))
		{
			delete pShape;
			continue;
		}

		this->ShapeAdd(pShape);
	}

	int nChrC = 0;
	file.Read(&nChrC, 4); // 캐릭터
	for (i = 0; i < nChrC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL]   = '\0';

		CN3Chr* pChr = new CN3Chr();
		if (false == pChr->LoadFromFile(szName))
		{
			delete pChr;
			continue;
		}

		this->ChrAdd(pChr);
	}

	if (m_nCameraCount <= 0)
		this->DefaultCameraAdd();
	if (m_nLightCount <= 0)
		this->DefaultLightAdd();

	return true;
}

bool CN3Scene::Save(File& file)
{
	std::error_code ec;
	std::filesystem::create_directory(PathGet() + "/Data", ec);
	std::filesystem::create_directory(PathGet() + "/Chr", ec);
	std::filesystem::create_directory(PathGet() + "/Object", ec);
	std::filesystem::create_directory(PathGet() + "/Item", ec);

	file.Write(&m_nCameraActive, 4);
	file.Write(&m_fFrmCur, 4);      // Animation Frame;
	file.Write(&m_fFrmStart, 4);    // 전체 프레임.
	file.Write(&m_fFrmEnd, 4);      // 전체 프레임.

	file.Write(&m_nCameraCount, 4); // 카메라..
	for (CN3Camera* camera : m_pCameras)
	{
		int nL = static_cast<int>(camera->FileName().size());
		file.Write(&nL, 4);
		file.Write(camera->FileName().c_str(), nL);
		camera->SaveToFile();
	}

	file.Write(&m_nLightCount, 4); // 카메라..
	for (CN3Light* light : m_pLights)
	{
		int nL = static_cast<int>(light->FileName().size());
		file.Write(&nL, 4);
		file.Write(light->FileName().c_str(), nL);
		light->SaveToFile();
	}

	int iSC = static_cast<int>(m_Shapes.size());
	file.Write(&iSC, 4); // Shapes..
	for (CN3Shape* shape : m_Shapes)
	{
		int nL = static_cast<int>(shape->FileName().size());
		file.Write(&nL, 4);
		if (nL <= 0)
			continue;

		file.Write(shape->FileName().c_str(), nL);
		shape->SaveToFile();
	}

	int iCC = static_cast<int>(m_Chrs.size());
	file.Write(&iCC, 4); // 캐릭터
	for (CN3Chr* chr : m_Chrs)
	{
		int nL = static_cast<int>(chr->FileName().size());
		file.Write(&nL, 4);
		if (nL <= 0)
			continue;

		file.Write(chr->FileName().c_str(), nL);
		chr->SaveToFile();
	}

	CN3Base::SaveResrc(); // Resource 를 파일로 저장한다..
	return true;
}

void CN3Scene::Render()
{
	//	for(int i = 0; i < m_nCameraCount; i++)
	//	{
	//		__ASSERT(m_pCameras[i], "Camera pointer is NULL");
	//		if(m_nCameraActive != i) m_pCameras[i]->Render();
	//	}

	//	for(int i = 0; i < m_nLightCount; i++)
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
	if (FRAME_SELFPLAY == fFrm || fFrm < m_fFrmStart || fFrm > m_fFrmEnd)
	{
		// 일정하게 움직이도록 시간에 따라 움직이는 양을 조절..
		m_fFrmCur += 30.0f / s_fFrmPerSec;
		if (m_fFrmCur > m_fFrmEnd)
			m_fFrmCur = m_fFrmStart;
	}
	else
	{
		m_fFrmCur = fFrm;
	}

	TickCameras();
	TickLights();
	TickShapes();
	TickChrs();
}

void CN3Scene::TickCameras()
{
	for (int i = 0; i < m_nCameraCount; i++)
	{
		m_pCameras[i]->Tick(m_fFrmCur);
		if (m_nCameraActive == i)
			m_pCameras[i]->Apply(); // 카메라 데이터 값을 적용한다..
	}
}

void CN3Scene::TickLights()
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

void CN3Scene::TickShapes()
{
	for (CN3Shape* shape : m_Shapes)
		shape->Tick(m_fFrmCur);
}

void CN3Scene::TickChrs()
{
	for (CN3Chr* chr : m_Chrs)
		chr->Tick(m_fFrmCur);
}

int CN3Scene::CameraAdd(CN3Camera* pCamera)
{
	if (m_nCameraCount < 0 || m_nCameraCount >= MAX_SCENE_CAMERA)
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
	if (iIndex < 0 || iIndex >= m_nCameraCount)
		return;

	delete m_pCameras[iIndex];
	m_pCameras[iIndex] = nullptr;

	m_nCameraCount--;
	for (int i = iIndex; i < m_nCameraCount; i++)
		m_pCameras[i] = m_pCameras[i + 1];
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
	if (iIndex < 0 || iIndex >= m_nCameraCount)
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
	if (iIndex < 0 || iIndex >= m_nLightCount)
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
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Shapes.size()))
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
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Chrs.size()))
		return;

	auto it = m_Chrs.begin();
	std::advance(it, iIndex);

	delete *it;
	m_Chrs.erase(it);
}

void CN3Scene::ChrDelete(CN3Chr* pChr)
{
	auto it = m_Chrs.begin(), itEnd = m_Chrs.end();
	for (; it != itEnd; it++)
	{
		CN3Chr* pChrSrc = *it;
		if (pChr == pChrSrc)
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
	if (szFN.empty())
		return false;

	char szPath[512] = "", szDrv[_MAX_DRIVE] = "", szDir[_MAX_DIR] = "";
	::_splitpath(szFN.c_str(), szDrv, szDir, nullptr, nullptr);
	::_makepath(szPath, szDrv, szDir, nullptr, nullptr);

	this->Release();
	this->PathSet(szPath);
	return LoadFromFile(szFN);
}

bool CN3Scene::SaveDataAndResourcesToFile(const std::string& szFN)
{
	if (szFN.empty())
		return false;

	char szPath[512] = "", szDrv[_MAX_DRIVE] = "", szDir[_MAX_DIR] = "";
	::_splitpath(szFN.c_str(), szDrv, szDir, nullptr, nullptr);
	::_makepath(szPath, szDrv, szDir, nullptr, nullptr);

	this->PathSet(szPath);
	return SaveToFile(szFN);
}

void CN3Scene::DefaultCameraAdd()
{
	CN3Camera* pCamera = new CN3Camera();
	pCamera->m_szName  = "DefaultCamera";
	pCamera->FileNameSet("Data\\DefaultCamera.N3Camera");
	this->CameraAdd(pCamera);
}
void CN3Scene::DefaultLightAdd()
{
	// Light 초기화..
	CN3Light* pLight = new CN3Light();
	pLight->m_szName = "DefaultLight";
	pLight->FileNameSet("Data\\DefaultLight.N3Light");
	int nLight           = this->LightAdd(pLight) - 1;

	__ColorValue ltColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	pLight->m_Data.InitDirection(0, { -1.0f, -1.0f, 0.5f }, ltColor);
	pLight->PosSet(1000.0f, 1000.0f, -1000.0f);
	pLight->m_Data.bOn     = TRUE;
	pLight->m_Data.nNumber = nLight;
}

