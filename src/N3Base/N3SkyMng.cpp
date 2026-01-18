// N3SkyMng.cpp: implementation of the CN3SkyMng class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3SkyMng.h"
#include "N3Sky.h"
#include "N3Moon.h"
#include "N3Sun.h"
#include "N3Cloud.h"
#include "N3Star.h"
#include "mmsystem.h"
#include "N3Texture.h"

#include "N3GERain.h"
#include "N3GESnow.h"

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
#include "N3SndObj.h"
#include "N3SndMgr.h"
#endif         // #ifdef _N3GAME

typedef std::vector<__SKY_DAYCHANGE>::iterator it_SDC;

CN3SkyMng::CN3SkyMng()
{
	m_eWeather    = SW_CLEAR;
	m_dwCheckTick = timeGetTime();

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	m_pSnd_Weather_Snow = nullptr;
	m_pSnd_Weather_Rain = nullptr;
#endif         // #ifdef _N3GAME
}

CN3SkyMng::~CN3SkyMng()
{
	delete m_pSky;
	m_pSky = nullptr;

	delete m_pMoon;
	m_pMoon = nullptr;

	delete m_pSun;
	m_pSun = nullptr;

	delete m_pCloud;
	m_pCloud = nullptr;

	delete m_pStar;
	m_pStar = nullptr;

	for (int i = 0; i < MAX_GAME_LIGHT; i++)
	{
		delete m_pLightColorDiffuses[i];
		m_pLightColorDiffuses[i] = nullptr;

		delete m_pLightColorAmbients[i];
		m_pLightColorAmbients[i] = nullptr;
	}

	m_fCellSize    = 0.0f;
	m_CurCellPos.x = m_CurCellPos.y = -1;

	delete m_pGERain;
	m_pGERain = nullptr;

	delete m_pGESnow;
	m_pGESnow = nullptr;

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Snow);
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Rain);
#endif         // #ifdef _N3GAME
}

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
void CN3SkyMng::ReleaseSound()
{
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Snow);
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Rain);
}
#endif // #ifdef _N3GAME

void CN3SkyMng::Release()
{
	CN3BaseFileAccess::Release();

	delete m_pSky;
	m_pSky = nullptr;

	delete m_pMoon;
	m_pMoon = nullptr;

	delete m_pSun;
	m_pSun = nullptr;

	delete m_pCloud;
	m_pCloud = nullptr;

	delete m_pStar;
	m_pStar = nullptr;

	for (int i = 0; i < MAX_GAME_LIGHT; i++)
	{
		delete m_pLightColorDiffuses[i];
		m_pLightColorDiffuses[i] = nullptr;
		delete m_pLightColorAmbients[i];
		m_pLightColorAmbients[i] = nullptr;
	}

	m_DayChanges.clear();
	m_iDayChangeCurPos = 0;

	m_WeatherChanges.clear();
	m_iWeatherChangeCurPos = 0;
	m_eWeather             = SW_CLEAR;

	delete m_pGERain;
	m_pGERain = nullptr;

	delete m_pGESnow;
	m_pGESnow = nullptr;

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Snow);
	s_SndMgr.ReleaseObj(&m_pSnd_Weather_Rain);
#endif         // #ifdef _N3GAME
}

void CN3SkyMng::Render()
{
	// Disable zbuffer, center view matrix, and set FOV to 72 degrees
	__Matrix44 matView = s_CameraData.mtxView;
	matView.m[3][0] = matView.m[3][1] = matView.m[3][2] = 0.0f;

	s_lpD3DDev->SetTransform(D3DTS_VIEW, matView.toD3D());

	// backup render state
	DWORD dwAlphaBlend = 0, dwSrcBlend = 0, dwDestBlend = 0, dwZEnable = 0, dwFog = 0,
		  dwLighting = 0;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	// set render state
	if (TRUE != dwAlphaBlend)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (D3DBLEND_SRCALPHA != dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if (D3DZB_FALSE != dwZEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	if (FALSE != dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if (FALSE != dwLighting)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // default 수치 이다.
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // default 수치 이다.

	if (m_pSky)
		m_pSky->Render();                                                   // 하늘
	if (m_pStar)
		m_pStar->Render();                                                  // 별
	if (m_pMoon)
		m_pMoon->Render(matView, s_CameraData.mtxProjection);               // 달
	if (m_pSun)
		m_pSun->Render(matView, s_CameraData.mtxProjection);                // 해
	if (m_pCloud)
		m_pCloud->Render();                                                 // 구름

	// Restore the modified renderstates
	if (TRUE != dwAlphaBlend)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	if (D3DBLEND_SRCALPHA != dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	if (D3DZB_FALSE != dwZEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	if (FALSE != dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if (FALSE != dwLighting)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	s_lpD3DDev->SetTransform(D3DTS_VIEW, s_CameraData.mtxView.toD3D());
}

void CN3SkyMng::RenderWeather()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (m_pGERain != nullptr)
				m_pGERain->Render(m_vPos[j * 3 + i]);

			if (m_pGESnow != nullptr)
				m_pGESnow->Render(m_vPos[j * 3 + i]);
		}
	}
}

void CN3SkyMng::Tick()
{
	uint32_t dwCurTickCount = timeGetTime();
	__ASSERT(dwCurTickCount >= m_dwCheckTick, "음수이다.");
	uint32_t dwCurGameTime = m_dwCheckGameTime
							 + (uint32_t) ((dwCurTickCount - m_dwCheckTick) * TIME_REAL_PER_GAME
										   * 0.001f);
	if (!m_DayChanges.empty())
	{
		// dwCurGameTime 가 24*60*60을 넘었을 경우
		if (dwCurGameTime > 86400)
		{
			m_iDayChangeCurPos = 0;
			__ASSERT(m_dwCheckGameTime < 86400, "시간이 이상해요");
			m_dwCheckTick     += (int) ((86400 - m_dwCheckGameTime)
                                    * TIME_GAME_PER_REAL); // CheckTick을 게임 0시에 맞게 다시 설정
			dwCurGameTime     -= 86400;
			m_dwCheckGameTime  = 0;                            // 기준 게임시간을 0시로..

			m_iDay++;                                          // 날짜 증가..
			m_pMoon->SetMoonPhase(m_iMonth * 30 + m_iDay);     // 달모양을 변화시키고..
			if (m_iDay > 30)
			{
				m_iDay = 0;
				m_iMonth++;
				if (m_iMonth > 12)
				{
					m_iMonth = 0;
					m_iYear++;
				}
			}
		}

		// 해의 위치 계산
		float fAngleTime = 0;
		if (m_iHourFix > 0)
			fAngleTime = (m_iHourFix * 3600.0f / 86400.0f)
						 * 360.0f; // 해와 달의 위치가 고정돼야 한다면..
		else
			fAngleTime = (dwCurGameTime / 86400.0f) * 360.0f;

		if (m_pSun)
			m_pSun->SetCurAngle(fAngleTime + 270.0f);
		if (m_pMoon)
			m_pMoon->SetCurAngle(fAngleTime + 90.0f);

		while (m_iDayChangeCurPos >= 0 && m_iDayChangeCurPos < static_cast<int>(m_DayChanges.size())
			   && m_DayChanges[m_iDayChangeCurPos].dwWhen < dwCurGameTime)
		{
			__SKY_DAYCHANGE* pSDC = &(m_DayChanges[m_iDayChangeCurPos]);
			// 실행할 명령의 시간과 현재 시간 차이
			uint32_t dwDiffTime   = dwCurGameTime - pSDC->dwWhen;

			// 변화에 걸리는 시간 조정
			float fTakeTime       = pSDC->fHowLong - dwDiffTime * TIME_GAME_PER_REAL; // 걸리는 시간
			if (fTakeTime < 0.0f)
			{
				//TRACE("!!!!! The sky changed instantly !!!!!");
				fTakeTime = 0.0f; // 0보다 작으면 즉시 변화하게 하자
			}

			// 하늘 변화명령 실행하기
			if (SW_CLEAR == m_eWeather || // 맑은 날씨이거나
				(SDC_SKYCOLOR != pSDC->eSkyDayChange && SDC_FOGCOLOR != pSDC->eSkyDayChange
					&& SDC_SUNCOLOR != pSDC->eSkyDayChange && SDC_FLARECOLOR != pSDC->eSkyDayChange
					&& SDC_GLOWCOLOR != pSDC->eSkyDayChange
					&& SDC_CLOUD1COLOR != pSDC->eSkyDayChange
					&& SDC_CLOUD2COLOR != pSDC->eSkyDayChange
					&& SDC_CLOUDTEX != pSDC->eSkyDayChange)) // 날씨 변화요소가 아닐경우
			{
				ChangeSky(pSDC, fTakeTime);
			}
			m_iDayChangeCurPos++;
		}

		// 날씨 변화에 따른 하늘 변화명령 실행하기
		if (!m_WeatherChanges.empty())
		{
			while (m_iWeatherChangeCurPos >= 0
				   && m_iWeatherChangeCurPos < static_cast<int>(m_WeatherChanges.size())
				   && m_WeatherChanges[m_iWeatherChangeCurPos].dwWhen < dwCurGameTime)
			{
				__SKY_DAYCHANGE* pSDC = &m_WeatherChanges[m_iWeatherChangeCurPos];
				// 실행할 명령의 시간과 현재 시간 차이
				uint32_t dwDiffTime   = dwCurGameTime - pSDC->dwWhen;

				// 변화에 걸리는 시간 조정
				float fTakeTime = pSDC->fHowLong - dwDiffTime * TIME_GAME_PER_REAL; // 걸리는 시간
				if (fTakeTime < 0.0f)
					fTakeTime = 0.0f;       // 0보다 작으면 즉시 변화하게 하자
				ChangeSky(pSDC, fTakeTime); // 변화시키기
				m_iWeatherChangeCurPos++;
			}

			// 날씨 변화 queue 삭제하기
			if (m_iWeatherChangeCurPos >= static_cast<int>(m_WeatherChanges.size()))
			{
				m_WeatherChanges.clear();
				m_iWeatherChangeCurPos = 0;
			}
		}
	}
	if (m_pSky)
		m_pSky->Tick();
	//	if (m_pMoon) m_pMoon->Tick();
	if (m_pSun)
		m_pSun->Tick();
	if (m_pCloud)
		m_pCloud->Tick();
	if (m_pStar)
		m_pStar->Tick();

	for (int i = 0; i < MAX_GAME_LIGHT; i++)
	{
		m_pLightColorDiffuses[i]->Tick();
		m_pLightColorAmbients[i]->Tick();
	}

	if (m_fCellSize > 0.0f)
	{
		POINT NewCellPos;
		NewCellPos.x = int(s_CameraData.vEye.x / m_fCellSize);
		NewCellPos.y = int(s_CameraData.vEye.z / m_fCellSize);
		if (NewCellPos.x != m_CurCellPos.x || NewCellPos.y != m_CurCellPos.y)
		{
			m_CurCellPos = NewCellPos;
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					m_vPos[j * 3 + i].Set((m_CurCellPos.x + i - 0.5f) * m_fCellSize, 0,
						(m_CurCellPos.y + j - 0.5f) * m_fCellSize);
				}
			}
		}

		if (m_pGERain)
		{
			m_pGERain->Tick();
			if (m_pGERain->NeedDelete())
			{
				delete m_pGERain;
				m_pGERain = nullptr;
			}
		}
		if (m_pGESnow)
		{
			m_pGESnow->Tick();
			if (m_pGESnow->NeedDelete())
			{
				delete m_pGESnow;
				m_pGESnow = nullptr;
			}
		}
	}
}

#ifdef _N3TOOL
bool CN3SkyMng::DayChangeParse(FILE* fp, __SKY_DAYCHANGE* pDayChange)
{
	if (nullptr == fp || nullptr == pDayChange)
		return false;
	char szLine0[512] = "", szLine1[512] = "";
	char* pResult0 = fgets(szLine0, 256, fp);
	char* pResult1 = fgets(szLine1, 256, fp);
	if (nullptr == pResult0 || nullptr == pResult1)
		return false;

	pDayChange->Init(); // 초기화 해주고..

	pDayChange->szName = szLine0;
	if (pDayChange->szName.size() >= 2)
		pDayChange->szName = pDayChange->szName.substr(0, pDayChange->szName.size() - 2);
	int iSkyDayChange = 0;
	sscanf(szLine1, "Type : [%d], Time : [%d], Parameter1 : [%d], Parameter2 : [%d], Elapse : [%f]",
		&iSkyDayChange, &pDayChange->dwWhen, &pDayChange->dwParam1, &pDayChange->dwParam2,
		&pDayChange->fHowLong);

	pDayChange->eSkyDayChange = static_cast<eSKY_DAYCHANGE>(iSkyDayChange);

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
bool CN3SkyMng::DayChangeWrite(FILE* fp, __SKY_DAYCHANGE* pDayChange)
{
	if (nullptr == fp || nullptr == pDayChange)
		return false;

	char szLine[512] = "";
	fprintf(fp, "%s\r\n", pDayChange->szName.c_str());
	fprintf(fp, "Type : [%d], Time : [%d], Parameter1 : [%d], Parameter2 : [%d], Elapse : [%f]\r\n",
		pDayChange->eSkyDayChange, pDayChange->dwWhen, pDayChange->dwParam1, pDayChange->dwParam2,
		pDayChange->fHowLong);

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
bool CN3SkyMng::LoadFromTextFile(const char* szIniFN)
{
	if (nullptr == szIniFN || strlen(szIniFN) <= 0)
		return false;

	FILE* fp = fopen(szIniFN, "r");
	if (!fp)
		return false;

	std::string szMoon;
	std::string szSuns[NUM_SUNPART];
	std::string szClouds[NUM_CLOUD];

	char szLine[512] = "", szBuff[256] = "";
	char* pResult = fgets(szLine, 512, fp);
	if (pResult)
	{
		sscanf(szLine, "Moon : %s", szBuff);
		szMoon = szBuff;
	}

	int i;
	for (i = 0; i < NUM_SUNPART; i++)
	{
		pResult = fgets(szLine, 512, fp);
		if (pResult)
		{
			sscanf(szLine, "Sun : %s", szBuff);
			szSuns[i] = szBuff;
		}
	}

	for (i = 0; i < NUM_CLOUD; i++)
	{
		pResult = fgets(szLine, 512, fp);
		if (pResult)
		{
			sscanf(szLine, "Cloud : %s", szBuff);
			szClouds[i] = szBuff;
		}
	}

	int iSDCC = 0;
	pResult   = fgets(szLine, 512, fp);
	if (pResult)
	{
		sscanf(szLine, "DayChange Count : %d", &iSDCC);

		if (iSDCC > 0)
		{
			m_DayChanges.resize(iSDCC); //m_DayChanges.assign(iSDCC);
			for (int i = 0; i < iSDCC; i++)
			{
				if (false == this->DayChangeParse(fp, &(m_DayChanges[i])))
				{
					std::string szErrLine = fmt::format(
						"From \"DayChage Count : \" -> Line : {}", i);
					MessageBoxA(CN3Base::s_hWndBase, szErrLine.c_str(),
						"하늘 환경설정 데이터 Parsing 실패", MB_OK); //CN3Base::s_hWndBase
					this->Release();
					return false;
				}
			}
		}
	}

	fclose(fp); // 파일 닫기..

	if (nullptr == m_pSky)
		m_pSky = new CN3Sky();
	m_pSky->Init();

	if (nullptr == m_pStar)
		m_pStar = new CN3Star();
	m_pStar->Init();

	if (nullptr == m_pSun)
		m_pSun = new CN3Sun();
	m_pSun->Init(szSuns);

	if (nullptr == m_pCloud)
		m_pCloud = new CN3Cloud();
	m_pCloud->Init(szClouds);

	if (nullptr == m_pMoon)
		m_pMoon = new CN3Moon();
	m_pMoon->Init(szMoon);

	for (i = 0; i < MAX_GAME_LIGHT; i++)
	{
		if (nullptr == m_pLightColorDiffuses[i])
			m_pLightColorDiffuses[i] = new CN3ColorChange();
		if (nullptr == m_pLightColorAmbients[i])
			m_pLightColorAmbients[i] = new CN3ColorChange();
	}

	std::vector<__SKY_DAYCHANGE>(m_DayChanges).swap(m_DayChanges); // 용량을 딱 맞추기..

	qsort(&(m_DayChanges[0]), m_DayChanges.size(), sizeof(__SKY_DAYCHANGE), CompareTime);
	SetCheckGameTime(CONVERT_SEC(10, 0, 0));

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
bool CN3SkyMng::SaveToTextFile(const char* szIniFN)
{
	if (nullptr == szIniFN || strlen(szIniFN) <= 0)
		return false;

	FILE* fp = fopen(szIniFN, "w");
	if (!fp)
		return false;

	char szBuff[256] = "";

	if (this->MoonTextureGet())
		fprintf(fp, "Moon : %s\r\n", this->MoonTextureGet()->FileName().c_str());
	else
		fprintf(fp, "Moon : \r\n");

	int i = 0;
	for (i = 0; i < NUM_SUNPART; i++)
	{
		if (this->SunTextureGet(i))
			fprintf(fp, "Sun : %s\r\n", this->SunTextureGet(i)->FileName().c_str());
		else
			fprintf(fp, "Sun : \r\n");
	}

	for (i = 0; i < NUM_CLOUD; i++)
	{
		if (this->CloudTextureFileName(i))
			fprintf(fp, "Cloud : %s\r\n", this->CloudTextureFileName(i));
		else
			fprintf(fp, "Cloud : \r\n");
	}

	fprintf(fp, "DayChange Count : %zu\r\n", m_DayChanges.size());
	for (__SKY_DAYCHANGE& dayChange : m_DayChanges)
		DayChangeWrite(fp, &dayChange);

	fclose(fp); // 파일 닫기..

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3SkyMng::InitToDefaultHardCoding()
{
	this->Release();

	m_pSky = new CN3Sky();
	m_pSky->Init();

	m_pMoon = new CN3Moon();
	m_pMoon->Init("misc\\sky\\phases.tga");

	std::string szSuns[NUM_SUNPART] = { "misc\\sky\\sundisk.bmp", "misc\\sky\\sunglow.bmp",
		"misc\\sky\\sunflare.bmp" };
	m_pSun                          = new CN3Sun();
	m_pSun->Init(szSuns);

	std::string szClouds[NUM_CLOUD] = { "misc\\sky\\wisps.tga", "misc\\sky\\puffs.tga",
		"misc\\sky\\tatters.tga", "misc\\sky\\streaks.tga", "misc\\sky\\dense.tga",
		"misc\\sky\\overcast.tga" };
	m_pCloud                        = new CN3Cloud();
	m_pCloud->Init(szClouds);

	m_pStar = new CN3Star();
	m_pStar->Init();

	for (int i = 0; i < MAX_GAME_LIGHT; i++)
	{
		m_pLightColorDiffuses[i] = new CN3ColorChange();
		m_pLightColorAmbients[i] = new CN3ColorChange();
	}

	// 임시 hard coding
	__SKY_DAYCHANGE tmpDayChange;
	m_DayChanges.reserve(64);
	uint32_t dwTime = 0;

	// 해뜨기..
	dwTime          = CONVERT_SEC(5, 0, 0); // 5시에 해가 뜬다..

	tmpDayChange.Init(
		"SunRise - SkyColor", SDC_SKYCOLOR, dwTime, D3DCOLOR_ARGB(255, 165, 115, 85), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - FogColor", SDC_FOGCOLOR, dwTime, D3DCOLOR_ARGB(255, 80, 110, 160), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - SunColor", SDC_SUNCOLOR, dwTime, D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - GlowColor", SDC_GLOWCOLOR, dwTime, D3DCOLOR_ARGB(255, 165, 122, 110), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - FlareColor", SDC_FLARECOLOR, dwTime, D3DCOLOR_ARGB(255, 122, 80, 50), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("SunRise - Cloud1Color", SDC_CLOUD1COLOR, dwTime,
		D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("SunRise - Cloud2Color", SDC_CLOUD2COLOR, dwTime,
		D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - Light0", SDC_LIGHT0COLOR, dwTime, D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - Light1", SDC_LIGHT1COLOR, dwTime, D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunRise - Light2", SDC_LIGHT2COLOR, dwTime, D3DCOLOR_ARGB(255, 165, 115, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	// 한낮..
	dwTime = CONVERT_SEC(6, 0, 0); // 6시부터 낮이다..

	tmpDayChange.Init("Noon - StarCount", SDC_STARCOUNT, dwTime, 0, 0, 300.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - SkyColor", SDC_SKYCOLOR, dwTime, D3DCOLOR_ARGB(255, 46, 97, 189), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - FogColor", SDC_FOGCOLOR, dwTime, D3DCOLOR_ARGB(255, 169, 203, 215), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - SunColor", SDC_SUNCOLOR, dwTime, D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - GlowColor", SDC_GLOWCOLOR, dwTime, D3DCOLOR_ARGB(255, 224, 224, 192), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - FlareColor", SDC_FLARECOLOR, dwTime, D3DCOLOR_ARGB(255, 96, 96, 96), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("Noon - Cloud1Color", SDC_CLOUD1COLOR, dwTime,
		D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("Noon - Cloud2Color", SDC_CLOUD2COLOR, dwTime,
		D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - Light0", SDC_LIGHT0COLOR, dwTime, D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - Light1", SDC_LIGHT1COLOR, dwTime, D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"Noon - Light2", SDC_LIGHT2COLOR, dwTime, D3DCOLOR_ARGB(255, 255, 255, 255), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("Noon - MoonPhase", SDC_MOONPHASE, CONVERT_SEC(12, 0, 0), 0, 0,
		0); // 낮 12시에 달 모양 변화
	m_DayChanges.push_back(tmpDayChange);

	// 해질때..
	dwTime = CONVERT_SEC(20, 0, 0); // 저녁 8시에 해가 지기 시작한다..

	tmpDayChange.Init(
		"SunSet - SkyColor", SDC_SKYCOLOR, dwTime, D3DCOLOR_ARGB(255, 98, 115, 125), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - FogColor", SDC_FOGCOLOR, dwTime, D3DCOLOR_ARGB(255, 135, 162, 159), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - SunColor", SDC_SUNCOLOR, dwTime, D3DCOLOR_ARGB(255, 203, 132, 85), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - GlowColor", SDC_GLOWCOLOR, dwTime, D3DCOLOR_ARGB(255, 192, 128, 128), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - FlareColor", SDC_FLARECOLOR, dwTime, D3DCOLOR_ARGB(255, 204, 128, 40), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("SunSet - Cloud1Color", SDC_CLOUD1COLOR, dwTime,
		D3DCOLOR_ARGB(255, 182, 152, 116), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("SunSet - Cloud2Color", SDC_CLOUD2COLOR, dwTime,
		D3DCOLOR_ARGB(255, 162, 145, 107), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - Light0", SDC_LIGHT0COLOR, dwTime, D3DCOLOR_ARGB(255, 135, 162, 159), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - Light1", SDC_LIGHT1COLOR, dwTime, D3DCOLOR_ARGB(255, 135, 162, 159), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"SunSet - Light2", SDC_LIGHT2COLOR, dwTime, D3DCOLOR_ARGB(255, 135, 162, 159), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	// 완전히 해가 짐..
	dwTime = CONVERT_SEC(21, 0, 0); // 저녁 9시에 완전히 해가 진다.

	tmpDayChange.Init(
		"MidNight - SkyColor", SDC_SKYCOLOR, dwTime, D3DCOLOR_ARGB(255, 15, 18, 49), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"MidNight - FogColor", SDC_FOGCOLOR, dwTime, D3DCOLOR_ARGB(255, 39, 52, 95), 0, 180.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - StarCount", SDC_STARCOUNT, dwTime, MAX_STAR, 0, 300.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - SunColor", SDC_SUNCOLOR, dwTime, D3DCOLOR_ARGB(255, 30, 36, 85),
		0, 150.0f); // 푸르스름
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - GlowColor", SDC_GLOWCOLOR, dwTime, D3DCOLOR_ARGB(255, 15, 18, 49),
		0, 150.0f); // 하늘과 동
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - FlareColor", SDC_FLARECOLOR, dwTime,
		D3DCOLOR_ARGB(255, 15, 18, 49), 0, 150.0f); // 하늘과 동
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - Cloud1Color", SDC_CLOUD1COLOR, dwTime,
		D3DCOLOR_ARGB(255, 151, 144, 160), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init("MidNight - Cloud2Color", SDC_CLOUD2COLOR, dwTime,
		D3DCOLOR_ARGB(255, 79, 78, 92), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"MidNight - Light0", SDC_LIGHT0COLOR, dwTime, D3DCOLOR_ARGB(255, 15, 18, 49), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"MidNight - Light1", SDC_LIGHT1COLOR, dwTime, D3DCOLOR_ARGB(255, 15, 18, 49), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	tmpDayChange.Init(
		"MidNight - Light2", SDC_LIGHT2COLOR, dwTime, D3DCOLOR_ARGB(255, 15, 18, 49), 0, 150.0f);
	m_DayChanges.push_back(tmpDayChange);

	std::vector<__SKY_DAYCHANGE>(m_DayChanges).swap(m_DayChanges); // 용량을 딱 맞추기..

	qsort(&(m_DayChanges[0]), m_DayChanges.size(), sizeof(__SKY_DAYCHANGE), CompareTime);
	SetCheckGameTime(CONVERT_SEC(10, 0, 0));
}
#endif // #ifdef _N3TOOL

//	CheckGameTime을 정해주고 현재시간을 다시 세팅한다.(특정 시간으로 강제적으로 만들때 호출한다.)
void CN3SkyMng::SetCheckGameTime(uint32_t dwCheckGameTime)
{
	dwCheckGameTime      %= 86400;
	uint32_t dwCheckTick  = timeGetTime();
	m_dwCheckGameTime     = dwCheckGameTime;
	m_dwCheckTick         = dwCheckTick;

	// 해와 달의 각도 계산.
	m_pSun->SetCurAngle(dwCheckGameTime / 86400.0f * 360.0f + 270.0f);
	m_pMoon->SetCurAngle(dwCheckGameTime / 86400.0f * 360.0f + 90.0f);

	if (m_DayChanges.empty())
		return;

	// 큐에서 현재 게임시간에 맞는 순서를 찾는다.
	m_iDayChangeCurPos = 0;
	int iDCC           = static_cast<int>(m_DayChanges.size());
	while (m_iDayChangeCurPos >= 0 && m_iDayChangeCurPos < iDCC
		   && m_DayChanges[m_iDayChangeCurPos].dwWhen < dwCheckGameTime)
		++m_iDayChangeCurPos;

	if (m_iDayChangeCurPos >= iDCC)
		m_iDayChangeCurPos = iDCC - 1;

	// 현재 게임시간에서 각 sky상태별로 가장 최근에 변경된 값을 찾아서 값을 지정해준다.
	for (int i = 0; i < NUM_SKYDAYCHANGE; ++i)
	{
		if (i == SDC_MOONPHASE)
			continue;

		int iPos = GetLatestChange((eSKY_DAYCHANGE) i, m_iDayChangeCurPos);
		if (iPos < 0)
			continue; // 한바퀴를 다 돌았는데도 변화값을 찾을 수 없다.

		__SKY_DAYCHANGE* pSDC = &(m_DayChanges[iPos]);
		uint32_t dwEnd        = pSDC->dwWhen
						 + (uint32_t) (TIME_REAL_PER_GAME * pSDC->fHowLong); // 변화가 끝나는 시간
		if (dwEnd > 86400)
			dwEnd -= 86400; // 24시간이 넘었을경우 24시간을 빼준다.
		if (dwEnd < dwCheckGameTime)
		{                   // 현재 겜시간에서 변화가 이미 끝났을 경우
			ChangeSky(pSDC, 0.0f);
		}
		else
		{ // 현재 겜시간에서 변화가 진행중인 경우
			int iPrevPos = GetLatestChange((eSKY_DAYCHANGE) i, iPos);
			__ASSERT(iPrevPos >= 0, "여기에 올리가 없다");
			__SKY_DAYCHANGE* pPrevSDC = &(m_DayChanges[iPrevPos]);
			ChangeSky(pPrevSDC, 0.0f); // 이전 상태로 만들기

			// 변화상태로 만들기
			// 실행할 명령의 시간과 현재 시간 차이
			uint32_t dwDiffTime = dwCheckGameTime - pSDC->dwWhen;
			// 변화에 걸리는 시간 조정
			float fTakeTime     = pSDC->fHowLong - dwDiffTime * TIME_GAME_PER_REAL; // 걸리는 시간
			if (fTakeTime < 0.0f)
				fTakeTime = 0.0f; // 0보다 작으면 즉시 변화하게 하자
			ChangeSky(pSDC, fTakeTime);
		}
	}
}

// m_DayChanges에서 지정된 위치(iPos) 이전의 가장 최근에 변화하는 위치 얻어오기
int CN3SkyMng::GetLatestChange(eSKY_DAYCHANGE eSDC, int iPos)
{
	int iFind = iPos - 1;
	if (iFind < 0 || iFind >= static_cast<int>(m_DayChanges.size()))
		iFind = static_cast<int>(m_DayChanges.size()) - 1;

	while (iFind >= 0)
	{
		if (m_DayChanges[iFind].eSkyDayChange == eSDC)
			break; // 가장 최근의 변화를 찾았다.

		--iFind;
	}

	if (iFind < 0)
	{
		// 맨 뒤에서부터 다시 검색
		iFind = static_cast<int>(m_DayChanges.size()) - 1;
		while (iFind >= 0)
		{
			if (m_DayChanges[iFind].eSkyDayChange == eSDC)
				break; // 가장 최근의 변화를 찾았다.

			// 한바퀴를 다 돌았는데도 변화값을 찾을 수 없다.
			if (iPos > iFind)
			{
				iFind = -1;
				break;
			}

			--iFind;
		}
	}
	return iFind;
}

// (It allows to calculate the precise rate of change by comparing the pSDC-> How Long fTakeTime value and the value.)
// Changing the status of the sky
void CN3SkyMng::ChangeSky(__SKY_DAYCHANGE* pSDC, float fTakeTime)
{
	if (nullptr == pSDC)
		return;
	float fPercentage = 1.0f - fTakeTime / pSDC->fHowLong;

	// Execute command changes
	switch (pSDC->eSkyDayChange)
	{
		case SDC_SKYCOLOR:
			m_pSky->m_SkyColor.ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pSky->m_SkyColor.SetPercentage(fPercentage);
			break;
		case SDC_FOGCOLOR:
			m_pSky->m_FogColor.ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pSky->m_FogColor.SetPercentage(fPercentage);
			break;
		case SDC_STARCOUNT:
			m_pStar->SetStar(pSDC->dwParam1, fTakeTime);
			break;
		case SDC_MOONPHASE:
			m_pMoon->SetMoonPhase(
				m_pMoon->GetMoonPhaseIndex() + 1); // The phase index increased one.
			break;
		case SDC_SUNCOLOR:
			m_pSun->m_Parts[SUNPART_SUN].Color.ChangeColor(pSDC->dwParam1, pSDC->fHowLong); // color
			m_pSun->m_Parts[SUNPART_SUN].Color.SetPercentage(fPercentage);
			m_pSun->m_Parts[SUNPART_SUN].Delta.ChangeDelta(
				pSDC->dwParam2 / 1000.0f, pSDC->fHowLong); // size..
			m_pSun->m_Parts[SUNPART_SUN].Delta.SetPercentage(fPercentage);
			break;
		case SDC_GLOWCOLOR:
			m_pSun->m_Parts[SUNPART_GLOW].Color.ChangeColor(
				pSDC->dwParam1, pSDC->fHowLong);           // color
			m_pSun->m_Parts[SUNPART_GLOW].Color.SetPercentage(fPercentage);
			m_pSun->m_Parts[SUNPART_GLOW].Delta.ChangeDelta(
				pSDC->dwParam2 / 1000.0f, pSDC->fHowLong); // size..
			m_pSun->m_Parts[SUNPART_GLOW].Delta.SetPercentage(fPercentage);
			break;
		case SDC_FLARECOLOR:
			m_pSun->m_Parts[SUNPART_FLARE].Color.ChangeColor(
				pSDC->dwParam1, pSDC->fHowLong);           // color
			m_pSun->m_Parts[SUNPART_FLARE].Color.SetPercentage(fPercentage);
			m_pSun->m_Parts[SUNPART_FLARE].Delta.ChangeDelta(
				pSDC->dwParam2 / 1000.0f, pSDC->fHowLong); // size..
			m_pSun->m_Parts[SUNPART_FLARE].Delta.SetPercentage(fPercentage);
			break;
		case SDC_CLOUD1COLOR:
			m_pCloud->m_Color1.ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pCloud->m_Color1.SetPercentage(fPercentage);
			break;
		case SDC_CLOUD2COLOR:
			m_pCloud->m_Color2.ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pCloud->m_Color2.SetPercentage(fPercentage);
			break;
		case SDC_CLOUDTEX:
			m_pCloud->SetCloud((e_CLOUDTEX) pSDC->dwParam1, (e_CLOUDTEX) pSDC->dwParam2, fTakeTime);
			break;
		case SDC_LIGHT0COLOR:
			m_pLightColorDiffuses[0]->ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pLightColorDiffuses[0]->SetPercentage(fPercentage);
			m_pLightColorAmbients[0]->ChangeColor(pSDC->dwParam2, pSDC->fHowLong);
			m_pLightColorAmbients[0]->SetPercentage(fPercentage);
			break;
		case SDC_LIGHT1COLOR:
			m_pLightColorDiffuses[1]->ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pLightColorDiffuses[1]->SetPercentage(fPercentage);
			m_pLightColorAmbients[1]->ChangeColor(pSDC->dwParam2, pSDC->fHowLong);
			m_pLightColorAmbients[1]->SetPercentage(fPercentage);
			break;
		case SDC_LIGHT2COLOR:
			m_pLightColorDiffuses[2]->ChangeColor(pSDC->dwParam1, pSDC->fHowLong);
			m_pLightColorDiffuses[2]->SetPercentage(fPercentage);
			m_pLightColorAmbients[2]->ChangeColor(pSDC->dwParam2, pSDC->fHowLong);
			m_pLightColorAmbients[2]->SetPercentage(fPercentage);
			break;
		default:
			break;
	}
}

float CN3SkyMng::GetSunAngleByRadin()
{
	if (m_pSun)
		return m_pSun->m_fCurRadian;
	else
		return 0;
}

int CN3SkyMng::CompareTime(const void* pArg1, const void* pArg2)
{
	const __SKY_DAYCHANGE* pSDC1 = (__SKY_DAYCHANGE*) pArg1;
	const __SKY_DAYCHANGE* pSDC2 = (__SKY_DAYCHANGE*) pArg2;
	return (int) pSDC1->dwWhen - (int) pSDC2->dwWhen;
}

void CN3SkyMng::SetWeather(eSKY_WEATHER eWeather, int iPercentage)
{
	// 날씨에 변하는 것들 : SDC_SKYCOLOR, SDC_FOGCOLOR, SDC_CLOUD1COLOR, SDC_CLOUD2COLOR, SDC_CLOUDTEX
	eSKY_WEATHER ePrevWeather = m_eWeather; // 이전 날씨
	m_eWeather                = eWeather;

	// 현재 게임 시간 구하기
	uint32_t dwCurTickCount   = timeGetTime();
	__ASSERT(dwCurTickCount >= m_dwCheckTick, "음수이다.");
	uint32_t dwCurGameTime = m_dwCheckGameTime
							 + (uint32_t) ((dwCurTickCount - m_dwCheckTick) * TIME_REAL_PER_GAME
										   * 0.001f);
	// bool IsNight = (dwCurGameTime < CONVERT_SEC(6, 0, 0) || dwCurGameTime > CONVERT_SEC(19, 0, 0));

	if (SW_CLEAR == m_eWeather) // 맑은 날씨. 퍼센트는 안개...
	{
		if (SW_CLEAR == ePrevWeather)
			return;             // 이전 날씨가 맑았으면 변화시키지 않는다.
		int iAfterNSecPos = GetDayChangePos_AfterNSec(
			dwCurGameTime, 10); // 60초 후에 DayChangePos구하기
		float fHowLong    = 10.0f;
		uint32_t dwWhen   = dwCurGameTime + CONVERT_SEC(0, 0, 0);
		uint32_t dwParam1 = 0, dwParam2 = 0;

		// 날씨 변화 큐 만들기
		m_WeatherChanges.clear();
		m_iWeatherChangeCurPos = 0;
		m_WeatherChanges.reserve(16);
		__SKY_DAYCHANGE tmpWeatherChange;
		int iPos = 0;

		// 해
		iPos     = GetLatestChange(
            SDC_SUNCOLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Sun", SDC_SUNCOLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 해의 flare
		iPos = GetLatestChange(
			SDC_FLARECOLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Flare", SDC_FLARECOLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 해의 glow
		iPos = GetLatestChange(
			SDC_GLOWCOLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Glow", SDC_GLOWCOLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 하늘색
		iPos = GetLatestChange(
			SDC_SKYCOLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Sky", SDC_SKYCOLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 안개색
		iPos = GetLatestChange(
			SDC_FOGCOLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Fog", SDC_FOGCOLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름 1 색
		iPos = GetLatestChange(
			SDC_CLOUD1COLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Cloud1", SDC_CLOUD1COLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름 2색
		iPos = GetLatestChange(
			SDC_CLOUD2COLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Cloud2", SDC_CLOUD2COLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름 텍스쳐 - 미리 바뀌게 한다..
		iPos = GetLatestChange(
			SDC_CLOUDTEX, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = CLOUD_WISPS;
			dwParam2 = CLOUD_PUFFS;
		}
		tmpWeatherChange.Init("CloudTex", SDC_CLOUDTEX, dwCurGameTime, dwParam1, dwParam2, 10.0f);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 0
		iPos = GetLatestChange(
			SDC_LIGHT0COLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Light0", SDC_LIGHT0COLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 1
		iPos = GetLatestChange(
			SDC_LIGHT1COLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Light1", SDC_LIGHT1COLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 2
		iPos = GetLatestChange(
			SDC_LIGHT2COLOR, iAfterNSecPos); // N초 후에 가장 최근에 변한 하늘 변화 위치 구하기
		if (iPos >= 0)
		{
			dwParam1 = m_DayChanges[iPos].dwParam1;
			dwParam2 = m_DayChanges[iPos].dwParam2;
		}
		else
		{
			dwParam1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			dwParam2 = 100;
		}
		tmpWeatherChange.Init("Light2", SDC_LIGHT2COLOR, dwWhen, dwParam1, dwParam2, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 원래대로 날씨 돌리기..
		//iAfterNSecPos = GetDayChangePos_AfterNSec(
		//	dwCurGameTime, 10); // 10초 후에 DayChangePos구하기

		std::vector<__SKY_DAYCHANGE>(m_WeatherChanges).swap(m_WeatherChanges); // 용량을 딱 맞추기..
		qsort(
			&(m_WeatherChanges[0]), m_WeatherChanges.size(), sizeof(__SKY_DAYCHANGE), CompareTime);

		// 날씨 좋다~
		if (m_pGESnow)
			m_pGESnow->FadeSet(3.0f, false);
		if (m_pGERain)
			m_pGERain->FadeSet(3.0f, false);
#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
		if (m_pSnd_Weather_Rain)
			m_pSnd_Weather_Rain->Stop(5.0f);
		if (m_pSnd_Weather_Snow)
			m_pSnd_Weather_Snow->Stop(5.0f);
#endif                                                        // #ifdef _N3GAME
	}
	else if (SW_RAINY == m_eWeather || SW_SNOW == m_eWeather) // 비, 눈 옴
	{
		if (SW_CLEAR != ePrevWeather)
			return; // 이전 날씨가 맑은날이 아니면 변화시키지 않는다.

		float fHowLong = 10.0f;

		// 날씨 변화 큐 만들기
		m_WeatherChanges.clear();
		m_iWeatherChangeCurPos = 0;
		m_WeatherChanges.reserve(16);
		__SKY_DAYCHANGE tmpWeatherChange;
		__ColorValue crTmp1, crTmp2;
		float fDelta    = (0.5f + 0.3f * (100 - iPercentage) / 100.0f);
		uint32_t dwWhen = dwCurGameTime + CONVERT_SEC(0, 0, 10);

		// 라이트 세팅 변수 구하기..
		__ColorValue crLgt(1, 1, 1, 1);

		// 60초 후에 DayChangePos구하기
		int iAfterNSecPos = GetDayChangePos_AfterNSec(dwCurGameTime, 10);

		int iPos          = GetLatestChange(SDC_LIGHT1COLOR, iAfterNSecPos); // 지형에 내리쬐는 빛
		if (iPos >= 0)
			crLgt = m_DayChanges[iPos].dwParam1;

		float fDelta2 = (crLgt.r + crLgt.g + crLgt.b) / 3.0f;
		fDelta2       = 1.0f - fDelta2 * 0.8f * (iPercentage / 100.0f);

		// 해
		tmpWeatherChange.Init(
			"Sun", SDC_SUNCOLOR, dwWhen, 0xff000000, (uint32_t) (GetSunRatio() * 1000), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 해의 flare
		tmpWeatherChange.Init("Flare", SDC_FLARECOLOR, dwWhen, 0xff000000,
			(uint32_t) (GetFlareRatio() * 1000), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 해의 glow
		tmpWeatherChange.Init("Glow", SDC_GLOWCOLOR, dwWhen, 0xff000000,
			(uint32_t) (GetGlowRatio() * 1000), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		//  하늘색
		crTmp1    = GetSkyColor();
		crTmp1   *= fDelta;
		crTmp1.a  = 1.0f;
		tmpWeatherChange.Init("Sky", SDC_SKYCOLOR, dwWhen, crTmp1.ToD3DCOLOR(), 0, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		//  안개색
		crTmp1    = GetFogColor();
		crTmp1   *= fDelta2;
		crTmp1.a  = 1.0f;
		tmpWeatherChange.Init("Fog", SDC_FOGCOLOR, dwWhen, crTmp1.ToD3DCOLOR(), 0, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름색1
		crTmp1    = GetCloud1Color();
		crTmp1   *= fDelta * 0.75f;
		crTmp1.a  = 1.0f;
		tmpWeatherChange.Init(
			"Cloud1Color", SDC_CLOUD1COLOR, dwWhen, crTmp1.ToD3DCOLOR(), 0, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름색2 - 좀더 어둡게 한다..
		crTmp1    = GetCloud2Color();
		crTmp1   *= fDelta * 0.5f;
		crTmp1.a  = 1.0f;
		tmpWeatherChange.Init(
			"Cloud2Color", SDC_CLOUD2COLOR, dwWhen, crTmp1.ToD3DCOLOR(), 0, fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 구름 텍스쳐
		uint32_t dwTex1 = 0, dwTex2 = 0;
		if (iPercentage > 80)
		{
			dwTex1 = CLOUD_OVERCAST;
			dwTex2 = CLOUD_DENSE;
		}
		else if (iPercentage > 60)
		{
			dwTex1 = CLOUD_DENSE;
			dwTex2 = CLOUD_STREAKS;
		}
		else if (iPercentage > 40)
		{
			dwTex1 = CLOUD_STREAKS;
			dwTex2 = CLOUD_TATTERS;
		}
		else
		{
			dwTex1 = CLOUD_TATTERS;
			dwTex2 = CLOUD_PUFFS;
		}
		tmpWeatherChange.Init("CloudTex", SDC_CLOUDTEX, dwCurGameTime, dwTex1, dwTex2, 10.0f);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 0
		crTmp1    = this->GetLightDiffuseColor(0);
		crTmp1   *= fDelta2;
		crTmp1.a  = 1.0f;
		crTmp2    = this->GetLightAmbientColor(0);
		crTmp2   *= fDelta2;
		crTmp2.a  = 1.0f;
		tmpWeatherChange.Init(
			"Light0", SDC_LIGHT0COLOR, dwWhen, crTmp1.ToD3DCOLOR(), crTmp2.ToD3DCOLOR(), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 1
		crTmp1    = this->GetLightDiffuseColor(1);
		crTmp1   *= fDelta2;
		crTmp1.a  = 1.0f;
		crTmp2    = this->GetLightAmbientColor(1);
		crTmp2   *= fDelta2;
		crTmp2.a  = 1.0f;
		tmpWeatherChange.Init(
			"Light1", SDC_LIGHT1COLOR, dwWhen, crTmp1.ToD3DCOLOR(), crTmp2.ToD3DCOLOR(), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		// 라이트 2
		crTmp1    = this->GetLightDiffuseColor(2);
		crTmp1   *= fDelta2;
		crTmp1.a  = 1.0f;
		crTmp2    = this->GetLightAmbientColor(2);
		crTmp2   *= fDelta2;
		crTmp2.a  = 1.0f;
		tmpWeatherChange.Init(
			"Light2", SDC_LIGHT2COLOR, dwWhen, crTmp1.ToD3DCOLOR(), crTmp2.ToD3DCOLOR(), fHowLong);
		m_WeatherChanges.push_back(tmpWeatherChange);

		std::vector<__SKY_DAYCHANGE>(m_WeatherChanges).swap(m_WeatherChanges); // 용량을 딱 맞추기..
		qsort(&(m_WeatherChanges[0]), m_WeatherChanges.size(), sizeof(m_WeatherChanges[0]),
			CompareTime);

		float fHeight  = 20.0f;
		float fPercent = iPercentage / 100.0f;
		//		float fDensity = fPercent * 0.1f;
		float fDensity = fPercent * 0.03f;
		if (SW_RAINY == m_eWeather)
		{
			if (m_pGESnow)
				m_pGESnow->FadeSet(3.0f, false);
			if (m_pGERain == nullptr)
				m_pGERain = new CN3GERain;

			__Vector3 vVelocity(
				3.0f * ((50 - rand() % 100) / 50.0f), -(10.0f + 8.0f * fPercent), 0);
			float fRainLength = 0.4f + 0.6f * fPercent;

			m_fCellSize       = 20.0f;
			m_pGERain->Create(fDensity, m_fCellSize, fHeight, fRainLength, vVelocity, 10.0f); // 비
			m_pGERain->SetActive(TRUE);

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
			if (m_pSnd_Weather_Snow)
				m_pSnd_Weather_Snow->Stop(5.0f);
			CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Weather_Rain);
			m_pSnd_Weather_Rain = CN3Base::s_SndMgr.CreateObj(201); // 비오는 소리..
			if (m_pSnd_Weather_Rain)
			{
				float maxVolume = fPercent * 0.7f;
				m_pSnd_Weather_Rain->SetMaxVolume(maxVolume);
				m_pSnd_Weather_Rain->Looping(true);
				m_pSnd_Weather_Rain->Play(nullptr, 0.0f, 2.0f);
			}
#endif // #ifdef _N3GAME
		}
		else if (SW_SNOW == m_eWeather)
		{
			if (m_pGERain)
				m_pGERain->FadeSet(3.0f, false);
			if (m_pGESnow == nullptr)
				m_pGESnow = new CN3GESnow;

			float fHorz = (3.0f * fPercent) + (3.0f * ((50 - rand() % 100) / 50.0f));
			__Vector3 vVelocity(fHorz, -(2.0f + 2.0f * fPercent), 0);
			float fSnowSize = 0.1f + 0.1f * fPercent;

			m_fCellSize     = 20.0f;
			m_pGESnow->Create(fDensity, m_fCellSize, fHeight, fSnowSize, vVelocity, 10.0f); // 비
			m_pGESnow->SetActive(TRUE);

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
			if (m_pSnd_Weather_Rain)
				m_pSnd_Weather_Rain->Stop(5.0f);
			CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Weather_Snow);
			m_pSnd_Weather_Snow = CN3Base::s_SndMgr.CreateObj(200); // 바람소리..
			if (m_pSnd_Weather_Snow)
			{
				float maxVolume = fPercent * 0.7f;
				m_pSnd_Weather_Snow->SetMaxVolume(maxVolume);
				m_pSnd_Weather_Snow->Looping(true);
				m_pSnd_Weather_Snow->Play(nullptr, 0.0f, 2.0f);
			}
#endif // #ifdef _N3GAME
		}
	}
}

void CN3SkyMng::SunAndMoonDirectionFixByHour(int iHour) // 해와 달 각도 관리
{
	m_iHourFix = iHour;
}

void CN3SkyMng::GetGameTime(int* piYear, int* piMonth, int* piDay, int* piHour, int* piMin)
{
	uint32_t dwCurTickCount = timeGetTime();
	__ASSERT(dwCurTickCount >= m_dwCheckTick, "음수이다.");
	uint32_t dwCurGameTime = m_dwCheckGameTime
							 + (uint32_t) ((dwCurTickCount - m_dwCheckTick) * TIME_REAL_PER_GAME
										   * 0.001f);

	// dwCurGameTime - 초
	int iSecond = dwCurGameTime % 3600;

	if (piYear)
		*piYear = m_iYear;
	if (piMonth)
		*piMonth = m_iMonth;
	if (piDay)
		*piDay = m_iDay;
	if (piHour)
		*piHour = dwCurGameTime / 3600;
	if (piMin)
		*piMin = iSecond / 60;
}

void CN3SkyMng::SetGameTime(int iYear, int iMonth, int iDay, int iHour, int iMin)
{
	m_iYear  = iYear;
	m_iMonth = iMonth;
	m_iDay   = iDay;

	__ASSERT(m_pMoon, "null pointer");
	m_pMoon->SetMoonPhase(iMonth * 30 + iDay);
	SetCheckGameTime(CONVERT_SEC(iHour, iMin, 0));
}

int CN3SkyMng::GetDayChangePos_AfterNSec(uint32_t dwCurGameTime, float fSec) const
{
	// n초 후의 체크할 게임 시간을 계산
	// 150초 후 게임시간
	uint32_t dwCheckGameTime  = dwCurGameTime + (uint32_t) (fSec * TIME_REAL_PER_GAME);
	int iCheckDayChangeCurPos = m_iDayChangeCurPos;
	if (dwCheckGameTime > 86400) // 체크 시간이 게임시간의 24시를 넘으면
	{
		iCheckDayChangeCurPos  = 0;
		dwCheckGameTime       %= 86400;
	}

	// n초후의 queue의 위치 찾기
	while (iCheckDayChangeCurPos >= 0
		   && iCheckDayChangeCurPos < static_cast<int>(m_DayChanges.size())
		   && m_DayChanges[m_iDayChangeCurPos].dwWhen < dwCheckGameTime)
		++iCheckDayChangeCurPos;

	if (iCheckDayChangeCurPos >= static_cast<int>(m_DayChanges.size()))
		iCheckDayChangeCurPos = static_cast<int>(m_DayChanges.size()) - 1;

	return iCheckDayChangeCurPos;
}

D3DCOLOR CN3SkyMng::GetFogColor()
{
	if (m_pSky)
		return m_pSky->m_FogColor.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetSkyColor()
{
	if (m_pSky)
		return m_pSky->m_SkyColor.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetSunColor()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_SUN].Color.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetGlowColor()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_GLOW].Color.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetFlareColor()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_FLARE].Color.GetCurColor();
	else
		return 0;
}

float CN3SkyMng::GetSunRatio()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_SUN].Delta.GetCurDelta();
	else
		return 0;
}

float CN3SkyMng::GetGlowRatio()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_GLOW].Delta.GetCurDelta();
	else
		return 0;
}

float CN3SkyMng::GetFlareRatio()
{
	if (m_pSun)
		return m_pSun->m_Parts[SUNPART_FLARE].Delta.GetCurDelta();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetCloud1Color()
{
	if (m_pCloud)
		return m_pCloud->m_Color1.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetCloud2Color()
{
	if (m_pCloud)
		return m_pCloud->m_Color2.GetCurColor();
	else
		return 0;
}

D3DCOLOR CN3SkyMng::GetLightDiffuseColor(int iIndex)
{
	if (iIndex < 0 || iIndex >= MAX_GAME_LIGHT)
		return 0;
	if (nullptr == m_pLightColorDiffuses[iIndex])
		return 0;

	return m_pLightColorDiffuses[iIndex]->GetCurColor();
}

D3DCOLOR CN3SkyMng::GetLightAmbientColor(int iIndex)
{
	if (iIndex < 0 || iIndex >= MAX_GAME_LIGHT)
		return 0;
	if (nullptr == m_pLightColorAmbients[iIndex])
		return 0;

	return m_pLightColorAmbients[iIndex]->GetCurColor();
}

bool CN3SkyMng::Load(File& file)
{
	std::string szSuns[NUM_SUNPART];
	std::string szClouds[NUM_CLOUD];
	std::string szMoon;

	int i = 0;
	for (i = 0; i < NUM_SUNPART; i++)
	{
		int iL = 0;
		file.Read(&iL, 4);
		if (iL > 0)
		{
			szSuns[i].assign(iL, ' ');
			file.Read(&(szSuns[i][0]), iL);
		}
	}

	for (i = 0; i < NUM_CLOUD; i++)
	{
		int iL = 0;
		file.Read(&iL, 4);
		if (iL > 0)
		{
			szClouds[i].assign(iL, ' ');
			file.Read(&(szClouds[i][0]), iL);
		}
	}

	int iL = 0;
	file.Read(&iL, 4);
	if (iL > 0)
	{
		szMoon.assign(iL, ' ');
		file.Read(&(szMoon[0]), iL);
	}

	if (nullptr == m_pSky)
		m_pSky = new CN3Sky();
	m_pSky->Init();

	if (nullptr == m_pStar)
		m_pStar = new CN3Star();
	m_pStar->Init();

	if (nullptr == m_pSun)
		m_pSun = new CN3Sun();
	m_pSun->Init(szSuns);

	if (nullptr == m_pCloud)
		m_pCloud = new CN3Cloud();
	m_pCloud->Init(szClouds);

	if (nullptr == m_pMoon)
		m_pMoon = new CN3Moon();
	m_pMoon->Init(szMoon);

	for (i = 0; i < MAX_GAME_LIGHT; i++)
	{
		if (nullptr == m_pLightColorDiffuses[i])
			m_pLightColorDiffuses[i] = new CN3ColorChange();
		if (nullptr == m_pLightColorAmbients[i])
			m_pLightColorAmbients[i] = new CN3ColorChange();
	}

	// Day Change .....
	m_DayChanges.clear();
	int iSDCC = 0;
	file.Read(&iSDCC, 4);
	if (iSDCC > 0)
	{
		m_DayChanges.assign(iSDCC, __SKY_DAYCHANGE());
		for (i = 0; i < iSDCC; i++)
		{
			m_DayChanges[i].Load(file);
		}

		qsort(&(m_DayChanges[0]), m_DayChanges.size(), sizeof(__SKY_DAYCHANGE),
			CompareTime); // 시간순으로 정렬
	}

	SetCheckGameTime(CONVERT_SEC(10, 0, 0));
	return true;
}

#ifdef _N3TOOL
bool CN3SkyMng::Save(File& file)
{
	std::string szSuns[NUM_SUNPART];
	std::string szClouds[NUM_CLOUD];
	std::string szMoon;

	for (int i = 0; i < NUM_SUNPART; i++)
	{
		if (m_pSun != nullptr && m_pSun->m_Parts[i].pTex != nullptr)
			szSuns[i] = m_pSun->m_Parts[i].pTex->FileName();
	}

	for (int i = 0; i < NUM_CLOUD; i++)
	{
		if (m_pCloud != nullptr)
			szClouds[i] = m_pCloud->m_szTextures[i];
	}

	if (m_pMoon != nullptr && m_pMoon->m_pTexture != nullptr)
		szMoon = m_pMoon->m_pTexture->FileName();

	for (int i = 0; i < NUM_SUNPART; i++)
	{
		int iL = static_cast<int>(szSuns[i].size());
		file.Write(&iL, 4);
		if (iL > 0)
			file.Write(szSuns[i].c_str(), iL);
	}

	for (int i = 0; i < NUM_CLOUD; i++)
	{
		int iL = static_cast<int>(szClouds[i].size());
		file.Write(&iL, 4);
		if (iL > 0)
			file.Write(szClouds[i].c_str(), iL);
	}

	int iL = static_cast<int>(szMoon.size());
	file.Write(&iL, 4);
	if (iL > 0)
		file.Write(szMoon.c_str(), iL);

	// Day Change .....
	int iSDCC = static_cast<int>(m_DayChanges.size());
	file.Write(&iSDCC, 4);
	for (int i = 0; i < iSDCC; i++)
		m_DayChanges[i].Save(file);

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
__SKY_DAYCHANGE* CN3SkyMng::DayChangeGet(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_DayChanges.size()))
		return nullptr;

	return &m_DayChanges[iIndex];
}

__SKY_DAYCHANGE* CN3SkyMng::DayChangeAdd()
{
	__SKY_DAYCHANGE SDC;
	m_DayChanges.push_back(SDC);
	return &(m_DayChanges[m_DayChanges.size() - 1]);
}

__SKY_DAYCHANGE* CN3SkyMng::DayChangeInsert(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_DayChanges.size()))
		return nullptr;

	it_SDC it = m_DayChanges.begin();
	for (int i = 0; i < iIndex; i++, it++)
		;

	// TODO(srmeier): need to figure this out
	return nullptr; //it = m_DayChanges.insert(it);

	return &(*it);
}

bool CN3SkyMng::DayChangeDelete(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_DayChanges.size()))
		return false;

	it_SDC it = m_DayChanges.begin();
	for (int i = 0; i < iIndex; i++, it++)
		;

	it = m_DayChanges.erase(it);
	return true;
}

CN3Texture* CN3SkyMng::SunTextureSet(int iIndex, const char* szPath)
{
	if (nullptr == szPath || nullptr == m_pSun || iIndex < 0 || iIndex >= NUM_SUNPART)
		return nullptr;

	s_MngTex.Delete(&(m_pSun->m_Parts[iIndex].pTex));
	m_pSun->m_Parts[iIndex].pTex = s_MngTex.Get(szPath);

	return m_pSun->m_Parts[iIndex].pTex;
}

CN3Texture* CN3SkyMng::MoonTextureSet(const char* szPath)
{
	if (nullptr == szPath || nullptr == m_pMoon)
		return nullptr;

	s_MngTex.Delete(&(m_pMoon->m_pTexture));
	m_pMoon->m_pTexture = s_MngTex.Get(szPath);

	return m_pMoon->m_pTexture;
}

CN3Texture* CN3SkyMng::CloudTextureSet(int iIndex, const char* szPath)
{
	if (nullptr == szPath || nullptr == m_pCloud || iIndex < 0 || iIndex >= NUM_CLOUD)
		return nullptr;

	s_MngTex.Delete(&(m_pCloud->m_pTextures[iIndex]));
	m_pCloud->m_pTextures[iIndex]  = s_MngTex.Get(szPath);
	m_pCloud->m_szTextures[iIndex] = "";
	if (m_pCloud->m_pTextures[iIndex])
		m_pCloud->m_szTextures[iIndex] = m_pCloud->m_pTextures[iIndex]->FileName();

	return m_pCloud->m_pTextures[iIndex];
}

CN3Texture* CN3SkyMng::SunTextureGet(int iIndex)
{
	if (nullptr == m_pSun || iIndex < 0 || iIndex >= NUM_CLOUD)
		return nullptr;
	return m_pSun->m_Parts[iIndex].pTex;
}

CN3Texture* CN3SkyMng::MoonTextureGet()
{
	if (nullptr == m_pMoon)
		return nullptr;
	return m_pMoon->m_pTexture;
}

CN3Texture* CN3SkyMng::CloudTextureGet(int iIndex)
{
	if (nullptr == m_pCloud || iIndex < 0 || iIndex >= NUM_CLOUD)
		return nullptr;
	return m_pCloud->m_pTextures[iIndex];
}

const char* CN3SkyMng::CloudTextureFileName(int iIndex)
{
	if (nullptr == m_pCloud || iIndex < 0 || iIndex >= NUM_CLOUD)
		return nullptr;
	return m_pCloud->m_szTextures[iIndex].c_str();
}
#endif

/*
void CN3SkyMng::ColorDeltaSet(int iPercentage, float fHowLong) // 현재 하늘, 안개 색을 퍼센트 단위로 변화시킨다.. 비, 눈 올때 쓴다..
{
	if(iPercentage < 0) iPercentage = 0;
	if(iPercentage > 100) iPercentage = 100;

	D3DCOLOR crFinalFog = m_pSky->m_FogColor.GetCurColor();
	D3DCOLOR crFinalSky = m_pSky->m_SkyColor.GetCurColor();

	crFinalFog = 	((((crFinalFog & 0x00ff0000) >> 16) * iPercentage / 100) << 16) | 
					((((crFinalFog & 0x0000ff00) >> 8) * iPercentage / 100) << 8) | 
					((crFinalFog & 0x000000ff) * iPercentage / 100);
	
	crFinalSky = 	((((crFinalSky & 0x00ff0000) >> 16) * iPercentage / 100) << 16) | 
					((((crFinalSky & 0x0000ff00) >> 8) * iPercentage / 100) << 8) | 
					((crFinalSky & 0x000000ff) * iPercentage / 100);
	
	m_pSky->m_FogColor.ChangeColor(crFinalFog, fHowLong);
	m_pSky->m_SkyColor.ChangeColor(crFinalSky, fHowLong);

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	if(m_pSnd_Weather_Snow) m_pSnd_Weather_Snow->Stop(5.0f);
	if(m_pSnd_Weather_Rain) m_pSnd_Weather_Rain->Stop(5.0f);
#endif // #ifdef _N3GAME
}
*/
