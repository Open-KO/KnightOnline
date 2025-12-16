// GameProcCharacterSelect.h: interface for the CGameProcCharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_)
#define AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameProcedure.h"

enum e_ChrPos {	POS_CENTER = 1, POS_LEFT, POS_RIGHT };
enum e_DoProcProcess { PROCESS_ROTATEING = 1, PROCESS_PRESELECT, PROCESS_SELECTED, PROCESS_COMPLETE };
enum e_ChrClass { CLASS_WAR = 1, CLASS_ROG, CLASS_PRIST, CLASS_WIZARD };
const int CS_El_rf_wa = 65500;

const int	SELECT_ANIM_PRE_SELECT = 0;
const int	SELECT_ANIM_SELECTED = 1;
const int	SELECT_ANIM_DUMMY = 2;

const float	KARUS_THETA_MAX = 0.5f;
const float	ELMORAD_THERA_MAX = 0.38f;
const float KARUS_INCRESE_OFFSET = 0.02f;
const float ELMORAD_INCRESE_OFFSET = 0.015f;

class CUIManager;
class CUICharacterSelect;

struct __CharacterSelectInfo
{
	std::string szID; // 캐릭터 아이디 문자열 str
	e_Race		eRace; // 종족 b
	e_Class		eClass; // 직업 b
	int			iLevel ; // 레벨 b
	int			iFace; // 얼굴모양 b
	int			iHair; // 머리모양 b
	int			iZone; //zone number
	uint32_t		dwItemUpper; // 상체 dw
	int			iItemUpperDurability;
	uint32_t		dwItemLower; // 하체 dw
	int			iItemLowerDurability;
	uint32_t		dwItemHelmet; // 투구 dw
	int			iItemHelmetDurability;
	uint32_t		dwItemCloak; // 어깨(망토) dw
	int			iItemCloakDurability;
	uint32_t		dwItemGloves; // 장갑 dw
	int			iItemGlovesDurability;
	uint32_t		dwItemShoes; // 신발 dw
	int			iItemShoesDurability;

	void clear()
	{
		szID			= "";
		eRace			= RACE_UNKNOWN; // 종족 b
		eClass			= CLASS_UNKNOWN; // 직업 b
		iLevel			= 0; // 레벨 b
		iFace			= 0; // 얼굴모양 b
		iHair			= 0; // 머리모양 b
		iZone			= 0; //zone number
		dwItemUpper		= 0; // 상체 dw
		dwItemLower		= 0; // 하체 dw
		dwItemHelmet	= 0; // 투구 dw
		dwItemCloak		= 0; // 어깨(망토) dw
		dwItemGloves		= 0; // 장갑 dw
		dwItemShoes		= 0; // 신발 dw
	}

	
	__CharacterSelectInfo() { clear(); }
	~__CharacterSelectInfo() { }
};

class CGameProcCharacterSelect : public CGameProcedure  
{
	CN3SndObj*				m_pSnd_Rotate;

public:
	class CN3Shape*			m_pActiveBg;

	class CN3Chr*			m_pChrs[MAX_AVAILABLE_CHARACTER];
	__CharacterSelectInfo	m_InfoChrs[MAX_AVAILABLE_CHARACTER]; // 이미 만들어진 캐릭터 정보..

	class CN3Camera*		m_pCamera;
	class CN3Light*			m_pLights[8];
	__Vector3				m_vEye;
	__Vector3				m_vEyeBackup;
	__Vector3				m_vAt;
	__Vector3				m_vUp;
	__D3DLight9				m_lgt[3];

	CUICharacterSelect*		m_pUICharacterSelect;

	e_ChrPos				m_eCurPos;	
	e_ChrPos				m_eDestPos;	

	e_DoProcProcess			m_eCurProcess;
	float					m_fCurTheta;
	float					m_fFadeOut;
	bool					m_bFadeOutRender;

	bool					m_bReceivedCharacterSelect;
public:
	void	CharacterSelectOrCreate();
	void	MsgSend_RequestAllCharacterInfo();
	void	MsgSend_DeleteChr(const std::string& szKey);
	void	MsgSend_CharacterSelect(); // virtual

	int		MsgRecv_VersionCheck(Packet& pkt); // virtual
	int		MsgRecv_GameServerLogIn(Packet& pDataPack) override;
	bool	MsgRecv_CharacterSelect(Packet& pkt); // virtual
	void	MsgRecv_AllCharacterInfo(Packet& pkt);
	void	MsgRecv_DeleteChr(Packet& pkt);

	void	Release();
	void	Init();
	void	Tick();
	void	Render();

	CGameProcCharacterSelect();
	virtual ~CGameProcCharacterSelect();

	void	RotateLeft();
	void	RotateRight();

	void	AddChr(e_ChrPos eCP, __CharacterSelectInfo* pInfo);
	void	AddChrPart(int iPosIndex, const __TABLE_PLAYER_LOOKS* pItemBasic, e_PartPosition ePartPos, uint32_t dwItemID, int iItemDurability);

	void	DoJobLeft();
	void	DojobRight();
	void	CheckJobState();
	bool	CheckRotateLeft();
	bool	CheckRotateCenterToRight();
	bool	CheckRotateCenterToLeft();
	bool	CheckRotateRight();
	
	void	CharacterSelect();
	void	CharacterSelectFailed();

	void	DoSelectedChrProc();
	void	DoProcPreselect();
	void	IncreseLightFactor();
	void	DecreseLightFactor();
	void	ProcessOnReturn();
	void	FadeOutProcess();
	void	FadeOutRender();
protected:
	virtual bool ProcessPacket(Packet& pkt);
};

#endif // !defined(AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_)
