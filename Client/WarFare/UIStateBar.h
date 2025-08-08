// UIStateBar.h: interface for the CUIStateBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
#define AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

struct __PositionInfo
{
	__Vector3	vPos;
	int			iID;
	D3DCOLOR	crType; // 종류에 따른 색깔... 
};

struct __DurationMagicImg
{
	uint32_t		dwSkillID;
	class CN3UIDBCLButton* pIcon;
	float		fDuration;
};

typedef std::list<__PositionInfo>::iterator it_PositionInfo;
typedef std::list<__DurationMagicImg*>::iterator it_MagicImg;

struct __TABLE_UPC_SKILL;
class CUIStateBar : public CN3UIBase  
{
protected:
	CN3UIString*	m_pText_HP;
	CN3UIString*	m_pText_MP;
	CN3UIString*	m_pText_Exp;

	CN3UIString*	m_pText_Position;
	CN3UIProgress*	m_pProgress_HP;
	CN3UIProgress*	m_pProgress_MSP;
	CN3UIProgress*	m_pProgress_ExpC;
	CN3UIProgress*	m_pProgress_ExpP;

	// 미니맵...
	CN3UIBase*		m_pGroup_MiniMap;
	CN3UIImage*		m_pImage_Map; // 이 이미지에 미니맵 텍스처를 적용시킨다..
	CN3UIButton*	m_pBtn_ZoomIn;
	CN3UIButton*	m_pBtn_ZoomOut;
	CN3UIButton*	m_pBtn_Quest;
	CN3UIButton*	m_pBtn_Power;

	// NOTE(srmeier): new components
	CN3UIString*	m_pText_FPS;

	float			m_fZoom; // 지도의 배율..
	float			m_fMapSizeX;
	float			m_fMapSizeZ;
	float			m_fYawPlayer;
	__Vector3		m_vPosPlayer;
	__Vector3		m_vViewPos;

	__VertexTransformedColor	m_vArrows[6]; // 플레이어 위치 화살표..
	std::list<__PositionInfo>	m_Positions;
	std::list<__PositionInfo>	m_PositionsTop; // 맨 위에 그릴 위치덜..

	//컬려있는 마법스킬 표시하기..
	std::list<__DurationMagicImg*> m_pMagic;
	
// Attributes
public:
	bool OnKeyPress(int iKey);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void	AddMagic(__TABLE_UPC_SKILL* pSkill, float fDuration);
	void	DelMagic(__TABLE_UPC_SKILL* pSkill);
	void	ClearMagic();
	void	TickMagicIcon();
	void	TickMiniMap();

	bool	ToggleMiniMap();

	void	UpdateExp(int64_t iExp, int64_t iExpNext, bool bUpdateImmediately);
	void	UpdateMSP(int iMSP, int iMSPMax, bool bUpdateImmediately);
	void	UpdateHP(int iHP, int iHPMax, bool bUpdateImmediately);

	void UpdatePosition(const __Vector3& vPos, float fYaw);

	void ZoomSet(float fZoom);
	void PositionInfoAdd(int iID, const __Vector3& vPos, D3DCOLOR crID, bool bDrawTop);
	void PositionInfoClear();
	bool LoadMap(const std::string& szMiniMapFN, float fMapSizeX, float fMapSizeZ); // 미니맵 비트맵 파일 이름, 매의 너비 길이..(Meter);
	
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg); // 메시지를 받는다.. 보낸놈, msg

	virtual void	Tick();
	virtual void	Render(); // 미니맵 렌더링..
	virtual bool	Load(HANDLE hFile);
	
	virtual void	Release();
	CUIStateBar();
	virtual ~CUIStateBar();

};

#endif // !defined(AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)



