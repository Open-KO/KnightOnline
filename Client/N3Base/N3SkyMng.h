// N3SkyMng.h: interface for the CN3SkyMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SKYMNG_H__30DC78FB_6563_43BD_841E_B90928E850CD__INCLUDED_)
#define AFX_N3SKYMNG_H__30DC78FB_6563_43BD_841E_B90928E850CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3BaseFileAccess.h"
#include <vector>

const float TIME_REAL_PER_GAME = 10.0f;						// 실제 시간과 game시간의 비율(현재 게임은 실제 시간보다 열배 빨리 돌아간다..)
const float TIME_GAME_PER_REAL = 1.0f/TIME_REAL_PER_GAME;	// game시간과 실제 시간의 비율
#define CONVERT_SEC(h, m, s) ((h)*3600 + (m)*60 + (s))

// 시간 관리
enum		eSKY_DAYCHANGE {	SDC_SKYCOLOR=0,		// 하늘색
								SDC_FOGCOLOR,		// 안개색
								SDC_STARCOUNT,		// 별 수
								SDC_MOONPHASE,		// 달 모양
								SDC_SUNCOLOR,		// 해 색, 크기 - 1000 이 뷰포트에서 꽉차는 크기.
								SDC_GLOWCOLOR,		// 해의 glow색, 크기 - 1000 이 뷰포트에서 꽉차는 크기.
								SDC_FLARECOLOR,		// 해의 flare색, 크기 - 1000 이 뷰포트에서 꽉차는 크기.
								SDC_CLOUD1COLOR,	// 구름층 1의 색
								SDC_CLOUD2COLOR,	// 구름층 2의 색
								SDC_CLOUDTEX,		// 구름 텍스쳐 바꾸기
								
								SDC_LIGHT0COLOR,	// Direction Light 0
								SDC_LIGHT1COLOR,	// Direction Light 1
								SDC_LIGHT2COLOR,	// 플레이어 주위의 Light
								
								NUM_SKYDAYCHANGE,
								
								SDC_UNKNOWN = 0xffffffff
};

struct		__SKY_DAYCHANGE
{
	std::string		szName;			// 이름 붙이기..
	eSKY_DAYCHANGE	eSkyDayChange;	// 어떤 변화인가?
	uint32_t			dwWhen;			// 언제 변화시작인가?(0시부터 초단위로 나타낸 시간)
	uint32_t			dwParam1;		// parameter 1
	uint32_t			dwParam2;		// parameter 2
	float			fHowLong;		// 얼마나 걸릴것인가?(실시간 초)

	void Init()
	{
		szName = "";
		eSkyDayChange = SDC_UNKNOWN;
		dwWhen = dwParam1 = dwParam2 = 0;
		fHowLong = 0;
	}

	void Init(const std::string& szName_Arg, eSKY_DAYCHANGE eSDC_Arg, uint32_t dwWhen_Arg, uint32_t dwParam1_Arg, uint32_t dwParam2_Arg, float fHowLong_Arg)
	{
		szName = szName_Arg;
		eSkyDayChange = eSDC_Arg;
		dwWhen = dwWhen_Arg;
		dwParam1 = dwParam1_Arg;
		dwParam2 = dwParam2_Arg;
		fHowLong = fHowLong_Arg;
	}

	bool Load(HANDLE hFile)
	{
		szName = "";

		DWORD dwRWC = 0;
		int nL = 0;
		ReadFile(hFile, &nL, 4, &dwRWC, NULL);
		if(nL > 0) 
		{
			std::vector<char> buffer(nL+1, NULL);
			ReadFile(hFile, &buffer[0], nL, &dwRWC, NULL);
			szName = &buffer[0];
		}

		ReadFile(hFile, &eSkyDayChange, 4, &dwRWC, NULL);
		ReadFile(hFile, &dwWhen, 4, &dwRWC, NULL);
		ReadFile(hFile, &dwParam1, 4, &dwRWC, NULL);
		ReadFile(hFile, &dwParam2, 4, &dwRWC, NULL);
		ReadFile(hFile, &fHowLong, 4, &dwRWC, NULL);

		return true;
	}
	
	bool Save(HANDLE hFile)
	{
		DWORD dwRWC = 0;

		int nL = szName.size();
		WriteFile(hFile, &nL, 4, &dwRWC, NULL);
		if(nL > 0) WriteFile(hFile, szName.c_str(), nL, &dwRWC, NULL);

		WriteFile(hFile, &eSkyDayChange, 4, &dwRWC, NULL);
		WriteFile(hFile, &dwWhen, 4, &dwRWC, NULL);
		WriteFile(hFile, &dwParam1, 4, &dwRWC, NULL);
		WriteFile(hFile, &dwParam2, 4, &dwRWC, NULL);
		WriteFile(hFile, &fHowLong, 4, &dwRWC, NULL);

		return true;
	}

	__SKY_DAYCHANGE() { this->Init(); }
};

const int MAX_GAME_LIGHT = 3;

class CN3SkyMng : public CN3BaseFileAccess
{
public:
	CN3SkyMng();
	virtual ~CN3SkyMng();
// Attributes
public:
	enum		eSKY_WEATHER {  SW_CLEAR=1,	// 맑음
								SW_RAINY,	// 비
								SW_SNOW};	// 눈
protected:
	class CN3Sky*			m_pSky;
	class CN3Moon*			m_pMoon;
	class CN3Sun*			m_pSun;
	class CN3Cloud*			m_pCloud;
	class CN3Star*			m_pStar;
	class CN3ColorChange*	m_pLightColorDiffuses[MAX_GAME_LIGHT];
	class CN3ColorChange*	m_pLightColorAmbients[MAX_GAME_LIGHT];

	std::vector<__SKY_DAYCHANGE> m_DayChanges;		// 정보입력후에 qsort하자
	int						m_iDayChangeCurPos;

	std::vector<__SKY_DAYCHANGE> m_WeatherChanges;		// 정보입력후에 qsort하자
	int						m_iWeatherChangeCurPos;

	uint32_t		m_dwCheckTick;	// 서버에서 시간을 받을때의 윈도우TickCount(실시간) (게임시간으로 24시에 다시 설정하기도 한다.)
	uint32_t		m_dwCheckGameTime;	// 서버에서 내려받은 시간(게임 시간 초단위) 0 ~ (24*60*60)
	eSKY_WEATHER	m_eWeather;

	int			m_iYear;
	int			m_iMonth;
	int			m_iDay;

	int			m_iHourFix;		// 해,달 방향 고정시 시간으로 나타내는 방향.


	float			m_fCellSize;	// 내부 셀 크기
	POINT			m_CurCellPos;	// 현재 내부셀 좌표
	__Vector3		m_vPos[9];		// 주변 셀 9칸의 중심좌표들

	// 효과들..
	class CN3GERain*		m_pGERain;		// 비
	class CN3GESnow*		m_pGESnow;		// 눈
#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	class CN3SndObj*		m_pSnd_Weather_Rain;
	class CN3SndObj*		m_pSnd_Weather_Snow;
#endif // #ifdef _N3GAME

// Operations
public:
#ifdef _N3TOOL
	void InitToDefaultHardCoding();
	bool LoadFromTextFile(const char* szIniFN);
	bool SaveToTextFile(const char* szIniFN);
	bool DayChangeParse(FILE* fp, __SKY_DAYCHANGE* pDayChange);
	bool DayChangeWrite(FILE* fp, __SKY_DAYCHANGE* pDayChange);

	int					DayChangeCount() { return m_DayChanges.size(); };
	__SKY_DAYCHANGE*	DayChangeGet(int iIndex);
	__SKY_DAYCHANGE*	DayChangeAdd();
	__SKY_DAYCHANGE*	DayChangeInsert(int iIndex);
	bool				DayChangeDelete(int iIndex);

	CN3Texture*			SunTextureSet(int iIndex, const char* szPath);
	CN3Texture*			MoonTextureSet(const char* szPath);
	CN3Texture*			CloudTextureSet(int iIndex, const char* szPath);
	
	CN3Texture*			SunTextureGet(int iIndex);
	CN3Texture*			MoonTextureGet();
	CN3Texture*			CloudTextureGet(int iIndex);
	const char*			CloudTextureFileName(int iIndex);
#endif

	bool Load(HANDLE hFile) override;

#ifdef _N3GAME // 게임이 아닌 툴에서는 필요없다...
	void ReleaseSound();
#endif // #ifdef _N3GAME

#ifdef _N3TOOL
	bool Save(HANDLE hFile);
#endif

	D3DCOLOR	GetFogColor();
	D3DCOLOR	GetSkyColor();
	D3DCOLOR	GetSunColor();
	D3DCOLOR	GetGlowColor();
	D3DCOLOR	GetFlareColor();
	float		GetSunRatio();
	float		GetGlowRatio();
	float		GetFlareRatio();

	D3DCOLOR	GetCloud1Color();
	D3DCOLOR	GetCloud2Color();
	
	float		GetSunAngleByRadin();

	D3DCOLOR	GetLightDiffuseColor(int iIndex);
	D3DCOLOR	GetLightAmbientColor(int iIndex);

	void	Release() override;
	void	Render(); // 하늘만 렌더링..
	void	RenderWeather(); // 비나 눈등의 날씨 렌더링..
	void	Tick();

	// 시간 관리
	void	GetGameTime(int* piYear, int* piMonth, int* piDay, int* piHour, int*piMin);
	void	SetGameTime(int iYear, int iMonth, int iDay, int iHour, int iMin);
	void	SetWeather(eSKY_WEATHER eWeather, int iPercentage);	//	날씨 세팅
	
	void	SunAndMoonDirectionFixByHour(int iHour); // 해와 달 각도 관리

//	By : Ecli666 ( On 2002-04-04 오전 10:53:58 )
//
	CN3Sun*	GetSunPointer()	{	return m_pSun;	}
//	~(By Ecli666 On 2002-04-04 오전 10:53:58 )
	
protected:
	void		SetCheckGameTime(uint32_t dwCheckGameTime);//	CheckGameTime을 정해주고 현재시간을 다시 세팅한다.(특정 시간으로 강제적으로 만들때 호출한다.)
	int			GetLatestChange(eSKY_DAYCHANGE eSDC, int iPos);// m_pDayChangeQueues에서 지정된 위치(iPos) 이전의 가장 최근에 변화하는 위치 얻어오기
	void		ChangeSky(__SKY_DAYCHANGE* pSDC, float fTakeTime);
	int			GetDayChangePos_AfterNSec(uint32_t dwCurGameTime, float fSec);		// 실시간 N초 후에 DayChangeQueue의 위치 구하기
	static int		CompareTime(const void* pArg1, const void* pArg2);

};

#endif // !defined(AFX_N3SKYMNG_H__30DC78FB_6563_43BD_841E_B90928E850CD__INCLUDED_)
