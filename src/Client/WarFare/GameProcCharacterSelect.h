// GameProcCharacterSelect.h: interface for the CGameProcCharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_)
#define AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_

#pragma once

#include "GameProcedure.h"

enum e_ChrPos : uint8_t
{
	POS_CENTER = 1,
	POS_LEFT,
	POS_RIGHT
};

enum e_DoProcProcess : uint8_t
{
	PROCESS_ROTATEING = 1,
	PROCESS_PRESELECT,
	PROCESS_SELECTED,
	PROCESS_COMPLETE
};

enum e_ChrClass : uint8_t
{
	CLASS_WAR = 1,
	CLASS_ROG,
	CLASS_PRIST,
	CLASS_WIZARD
};

inline constexpr int SELECT_ANIM_PRE_SELECT   = 0;
inline constexpr int SELECT_ANIM_SELECTED     = 1;
inline constexpr int SELECT_ANIM_DUMMY        = 2;

inline constexpr float KARUS_THETA_MAX        = 0.5f;
inline constexpr float ELMORAD_THERA_MAX      = 0.38f;
inline constexpr float KARUS_INCRESE_OFFSET   = 0.02f;
inline constexpr float ELMORAD_INCRESE_OFFSET = 0.015f;

class CUIManager;
class CUICharacterSelect;

struct __CharacterSelectInfo
{
	std::string szID;      // 캐릭터 아이디 문자열 str
	e_Race eRace;          // 종족 b
	e_Class eClass;        // 직업 b
	int iLevel;            // 레벨 b
	int iFace;             // 얼굴모양 b
	int iHair;             // 머리모양 b
	int iZone;             //zone number
	uint32_t dwItemUpper;  // 상체 dw
	int iItemUpperDurability;
	uint32_t dwItemLower;  // 하체 dw
	int iItemLowerDurability;
	uint32_t dwItemHelmet; // 투구 dw
	int iItemHelmetDurability;
	uint32_t dwItemCloak;  // 어깨(망토) dw
	int iItemCloakDurability;
	uint32_t dwItemGloves; // 장갑 dw
	int iItemGlovesDurability;
	uint32_t dwItemShoes;  // 신발 dw
	int iItemShoesDurability;
	uint32_t dwRightHand;
	int iItemRightHandDurability;
	uint32_t dwLeftHand;
	int iItemLeftHandDurability;

	void clear()
	{
		szID.clear();
		eRace                    = RACE_UNKNOWN;  // 종족 b
		eClass                   = CLASS_UNKNOWN; // 직업 b
		iLevel                   = 0;             // 레벨 b
		iFace                    = 0;             // 얼굴모양 b
		iHair                    = 0;             // 머리모양 b
		iZone                    = 0;             //zone number
		dwItemUpper              = 0;             // 상체 dw
		iItemUpperDurability     = 0;
		dwItemLower              = 0;             // 하체 dw
		iItemLowerDurability     = 0;
		dwItemHelmet             = 0;             // 투구 dw
		iItemHelmetDurability    = 0;
		dwItemCloak              = 0;             // 어깨(망토) dw
		iItemCloakDurability     = 0;
		dwItemGloves             = 0;             // 장갑 dw
		iItemGlovesDurability    = 0;
		dwItemShoes              = 0;             // 신발 dw
		iItemShoesDurability     = 0;
		dwRightHand              = 0;
		iItemRightHandDurability = 0;
		dwLeftHand               = 0;
		iItemLeftHandDurability  = 0;
	}

	__CharacterSelectInfo()
	{
		clear();
	}

	~__CharacterSelectInfo()
	{
	}
};

class CN3Camera;
class CN3Chr;
class CN3Light;
class CN3Shape;
class CN3SndObj;
class CGameProcCharacterSelect : public CGameProcedure
{
	CN3SndObj* m_pSnd_Rotate = nullptr;

public:
	CN3Shape* m_pActiveBg                                     = nullptr;

	CN3Chr* m_pChrs[MAX_AVAILABLE_CHARACTER]                  = {};
	__CharacterSelectInfo m_InfoChrs[MAX_AVAILABLE_CHARACTER] = {}; // 이미 만들어진 캐릭터 정보..

	CN3Camera* m_pCamera                                      = nullptr;
	CN3Light* m_pLights[8]                                    = {};
	__Vector3 m_vEye                                          = {};
	__Vector3 m_vEyeBackup                                    = {};
	__Vector3 m_vAt                                           = {};
	__Vector3 m_vUp                                           = {};
	__D3DLight9 m_lgt[3]                                      = {};

	CUICharacterSelect* m_pUICharacterSelect                  = nullptr;

	e_ChrPos m_eCurPos;
	e_ChrPos m_eDestPos;

	e_DoProcProcess m_eCurProcess;
	float m_fCurTheta;
	float m_fFadeOut;
	bool m_bFadeOutRender;

	bool m_bReceivedCharacterSelect;

public:
	void CharacterSelectOrCreate();
	void MsgSend_RequestAllCharacterInfo();
	void MsgSend_DeleteChr(const std::string& szKey);
	void MsgSend_CharacterSelect() override;            // virtual

	int MsgRecv_VersionCheck(Packet& pkt) override;     // virtual
	int MsgRecv_GameServerLogIn(Packet& pDataPack) override;
	bool MsgRecv_CharacterSelect(Packet& pkt) override; // virtual
	void MsgRecv_AllCharacterInfo(Packet& pkt);
	void MsgRecv_DeleteChr(Packet& pkt);

	void Release() override;
	void Init() override;
	void Tick() override;
	void Render() override;

	CGameProcCharacterSelect();
	~CGameProcCharacterSelect() override;

	void RotateLeft();
	void RotateRight();

	void AddChr(e_ChrPos eCP, __CharacterSelectInfo* pInfo);
	void AddChrPart(int iPosIndex, const __TABLE_PLAYER_LOOKS* pItemBasic, e_PartPosition ePartPos, uint32_t dwItemID, int iItemDurability);

	void DoJobLeft();
	void DojobRight();
	void CheckJobState();
	bool CheckRotateLeft();
	bool CheckRotateCenterToRight();
	bool CheckRotateCenterToLeft();
	bool CheckRotateRight();

	void CharacterSelect();
	void CharacterSelectFailed();

	void DoSelectedChrProc();
	void DoProcPreselect();
	void IncreseLightFactor();
	void DecreseLightFactor();
	void ProcessOnReturn();
	void FadeOutProcess();
	void FadeOutRender();

protected:
	bool ProcessPacket(Packet& pkt) override;
};

#endif // !defined(AFX_GAMEPROCCHARACTERSELECT_H__FA8E7800_AE8D_4469_8D6A_7F409EED8C86__INCLUDED_)
