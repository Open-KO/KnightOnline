// GameProcCharacterCreate.cpp: implementation of the CGameProcCharacterCreate class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameProcCharacterCreate.h"
#include "GameEng.h"
#include "UICharacterCreate.h"
#include "UIMessageBox.h"
#include "UIMessageBoxManager.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameCursor.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>

CGameProcCharacterCreate::CGameProcCharacterCreate()
{
	m_pUICharacterCreate         = nullptr;

	s_pPlayer->m_InfoBase.eRace  = RACE_UNKNOWN;
	s_pPlayer->m_InfoBase.eClass = CLASS_UNKNOWN;

	m_rcChr                      = {};
}

CGameProcCharacterCreate::~CGameProcCharacterCreate()
{
	delete m_pUICharacterCreate;
}

void CGameProcCharacterCreate::Release()
{
	CGameProcedure::Release();

	delete m_pUICharacterCreate;
	m_pUICharacterCreate = nullptr;

	SetRect(&m_rcChr, 0, 0, 0, 0);
	m_Tbl_InitValue.Release();

	s_pPlayer->m_ChrInv.Release();
	s_pPlayer->m_ChrInv.PartAlloc(PART_POS_COUNT);
	s_pPlayer->m_ChrInv.PlugAlloc(PLUG_POS_COUNT);
}

void CGameProcCharacterCreate::Init()
{
	CGameProcedure::Init();

	SetRect(&m_rcChr, 0, 0, 0, 0);

	m_Tbl_InitValue.LoadFromFile("Data\\NewChrValue.tbl");

	s_pPlayer->m_InfoBase.eRace  = RACE_UNKNOWN;
	s_pPlayer->m_InfoBase.eClass = CLASS_UNKNOWN;
	s_pPlayer->m_ChrInv.Release();
	s_pPlayer->m_ChrInv.PartAlloc(PART_POS_COUNT);
	s_pPlayer->m_ChrInv.PlugAlloc(PLUG_POS_COUNT);

	__TABLE_UI_RESRC* pTblUI = s_pTbl_UI.Find(s_pPlayer->m_InfoBase.eNation);
	m_pUICharacterCreate     = new CUICharacterCreate();
	m_pUICharacterCreate->Init(s_pUIMgr);
	if (pTblUI)
		m_pUICharacterCreate->LoadFromFile(pTblUI->szCharacterCreate);
}

void CGameProcCharacterCreate::Render()
{
	s_pEng->Clear(0);                 // 클리어..
	s_pEng->s_lpD3DDev->BeginScene(); // 씬 렌더 ㅅ작...

	s_pUIMgr->Render();

	s_pPlayer->InventoryChrRender(m_rcChr); // 캐릭터 그리기..

	s_pMsgBoxMgr->Render();                 //MessageBox를 그려준다.
	if (s_pGameCursor)
		s_pGameCursor->Render();

	s_pEng->s_lpD3DDev->EndScene(); // 씬 렌더 시작...
	s_pEng->Present(CN3Base::s_hWndBase);
}

void CGameProcCharacterCreate::SetChr()
{
	__InfoPlayerBase* pInfoBase  = &(s_pPlayer->m_InfoBase);

	__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(s_pPlayer->m_InfoBase.eRace); // User Player Character Skin 구조체 포인터..;
	if (nullptr == pLooks)
		return;

	s_pPlayer->InitChr(pLooks);
	s_pPlayer->m_ChrInv.ScaleSet(1, 1, 1); // 스케일을 원래대로 돌린다.

	if (pLooks)                            // 파트 세팅..
	{
		for (int i = 0; i < PART_POS_COUNT; i++)
		{
			if (i == PART_POS_FACE)
			{
				s_pPlayer->InitFace();
				continue;
			}
			if (i == PART_POS_HAIR_HELMET)
			{
				s_pPlayer->InitHair();
				continue;
			}
			s_pPlayer->PartSet((e_PartPosition) i, pLooks->szPartFNs[i], nullptr, nullptr);
		}
	}

	m_pUICharacterCreate->Reset();
	m_pUICharacterCreate->UpdateRaceAndClassButtons(pInfoBase->eRace);
	m_pUICharacterCreate->UpdateClassButtons(pInfoBase->eClass);

	s_pPlayer->InventoryChrAnimationInitialize();
	s_pPlayer->Action(PSA_BASIC, true, nullptr, true);
}

void CGameProcCharacterCreate::SetStats()
{
	__InfoPlayerBase* pInfoBase  = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);
	__TABLE_NEW_CHR* pTbl        = nullptr;
	// dwID of InitValue is a concatenation of the race and the class
	uint32_t dwID                = pInfoBase->eRace * 10000 + pInfoBase->eClass;

	pTbl                         = m_Tbl_InitValue.Find(dwID);
	if (pTbl)
	{
		pInfoExt->iStrength                    = pTbl->iStr;
		pInfoExt->iStamina                     = pTbl->iSta;
		pInfoExt->iDexterity                   = pTbl->iDex;
		pInfoExt->iIntelligence                = pTbl->iInt;
		pInfoExt->iMagicAttak                  = pTbl->iMAP;
		m_pUICharacterCreate->m_iBonusPoint    = pTbl->iBonus;
		m_pUICharacterCreate->m_iMaxBonusPoint = pTbl->iBonus;
	}

	m_pUICharacterCreate->UpdateStats();
}

void CGameProcCharacterCreate::Tick()
{
	//	s_pLocalInput->Tick(); // 키보드와 마우스로부터 입력을 받는다.
	//	if(dwMouseFlags & MOUSE_LBDOWN) SetCursor(s_hCursorClick);
	//	else SetCursor(s_hCursorNormal);

	CGameProcedure::Tick();

	uint32_t dwMouseFlags = s_pLocalInput->MouseGetFlag();
	m_pUICharacterCreate->Tick();
	m_pUICharacterCreate->MouseProc(dwMouseFlags, s_pLocalInput->MouseGetPos(), s_pLocalInput->MouseGetPosOld());

	s_pEng->s_SndMgr.Tick(); // Sound Engine...

	while (!s_pSocket->m_qRecvPkt.empty())
	{
		auto pkt = s_pSocket->m_qRecvPkt.front();
		if (!ProcessPacket(*pkt))
			break;

		delete pkt;
		s_pSocket->m_qRecvPkt.pop();
	}

	s_pPlayer->InventoryChrTick();
}

bool CGameProcCharacterCreate::MsgSendCharacterCreate()
{
	e_ErrorCharacterCreate eErrCode = ERROR_CHARACTER_CREATE_SUCCESS;
	const std::string& szID         = s_pPlayer->IDString();

	if (szID.empty())
	{
		eErrCode = ERROR_CHARACTER_CREATE_INVALID_NAME;
	}
	else if (RACE_UNKNOWN == s_pPlayer->m_InfoBase.eRace)
	{
		eErrCode = ERROR_CHARACTER_CREATE_INVALID_RACE;
	}
	//	else if(RACE_KA_WRINKLETUAREK == s_pPlayer->m_InfoBase.eRace) // 마법사는 선택 불가능..
	//	{
	//		eErrCode = ERROR_CHARACTER_CREATE_NOT_SUPPORTED_RACE;
	//	}
	else if (CLASS_UNKNOWN == s_pPlayer->m_InfoBase.eClass)
	{
		eErrCode = ERROR_CHARACTER_CREATE_INVALID_CLASS;
	}
	else if (m_pUICharacterCreate->m_iBonusPoint > 0)
	{
		eErrCode = ERROR_CHARACTER_CREATE_REMAIN_BONUS_POINT;
	}
	else
	{
		// 이름에 빈칸이나 특수문자가 들어 있는지 확인
		bool bHasSpecialLetter = false;
		for (char c : szID)
		{
			// CompadmreString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, id, strlen(id), pUser->m_UserId, strlen(pUser->m_UserId) ) == CSTR_EQUAL )
			switch (c)
			{
				case '~':
				case '`':
				case '!':
				case '@':
				case '#':
				case '$':
				case '%':
				case '^':
				case '&':
				case '*':
				case '(':
				case ')':
				// case '_':
				case '-':
				case '+':
				case '=':
				case '|':
				case '\\':
				case '<':
				case '>':
				case ',':
				case '.':
				case '?':
				case '/':
				case '{':
				case '[':
				case '}':
				case ']':
				case '\"':
				case '\'':
				case ' ':
					bHasSpecialLetter = true;
					eErrCode          = ERROR_CHARACTER_CREATE_INVALID_NAME_HAS_SPECIAL_LETTER;
					break;

				default:
					break;
			}
		}

		if (!bHasSpecialLetter)
		{
			__InfoPlayerBase* pInfoBase  = &s_pPlayer->m_InfoBase;
			__InfoPlayerMySelf* pInfoExt = &s_pPlayer->m_InfoExt;

			uint8_t byBuff[64];
			int iOffset = 0;
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_NEW_CHAR);                         // 커멘드.
			CAPISocket::MP_AddByte(byBuff, iOffset, CGameProcedure::s_iChrSelectIndex);    // 캐릭터 인덱스 b
			CAPISocket::MP_AddShort(byBuff, iOffset, static_cast<int16_t>(szID.length())); // Id 길이 s
			CAPISocket::MP_AddString(byBuff, iOffset, s_pPlayer->IDString());              // ID 문자열 str
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoBase->eRace);                     // 종족 b
			CAPISocket::MP_AddShort(byBuff, iOffset, pInfoBase->eClass);                   // 직업 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iFace);                      // 얼굴모양 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iHair);                      // 머리모양 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iStrength);                  // 힘 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iStamina);                   // 지구력 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iDexterity);                 // 민첩 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iIntelligence);              // 지능 b
			CAPISocket::MP_AddByte(byBuff, iOffset, pInfoExt->iMagicAttak);                // 마력 b

			s_pSocket->Send(byBuff, iOffset);                                              // 보낸다

			s_pUIMgr->EnableOperationSet(false);                                           // 패킷이 들어올때까지 UI 를 Disable 시킨다...

			return true;
		}
	}

	ReportErrorCharacterCreate(eErrCode); // 에러 보고...

	return false;
}

void CGameProcCharacterCreate::ReportErrorCharacterCreate(e_ErrorCharacterCreate eErrCode)
{
	std::string szErr;

	if (ERROR_CHARACTER_CREATE_NO_MORE_CHARACTER == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_NO_MORE_CHARACTER);
	else if (ERROR_CHARACTER_CREATE_INVALID_NATION_AND_INVALID_RACE == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_INVALID_NATION_RACE);
	else if (ERROR_CHARACTER_CREATE_OVERLAPPED_ID == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_OVERLAPPED_ID);
	else if (ERROR_CHARACTER_CREATE_DB_CREATE == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_DB_CREATE);
	else if (ERROR_CHARACTER_CREATE_INVALID_NAME == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_INVALID_NAME);
	else if (ERROR_CHARACTER_CREATE_INVALID_NAME_HAS_SPECIAL_LETTER == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_INVALID_NAME_HAS_SPECIAL_LETTER);
	else if (ERROR_CHARACTER_CREATE_INVALID_RACE == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_INVALID_RACE);
	else if (ERROR_CHARACTER_CREATE_NOT_SUPPORTED_RACE == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_NOT_SUPPORTED_RACE);
	else if (ERROR_CHARACTER_CREATE_INVALID_CLASS == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_INVALID_CLASS);
	else if (ERROR_CHARACTER_CREATE_REMAIN_BONUS_POINT == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_REMAIN_BONUS_POINT);
	else // if (ERROR_CHARACTER_CREATE_INVALID_STAT_POINT == eErrCode)
		szErr = fmt::format_text_resource(IDS_ERR_UNKNOWN);

	std::string szTitle = fmt::format_text_resource(IDS_ERR_CHARACTER_CREATE);
	MessageBoxPost(szErr, szTitle, MB_OK);
}

bool CGameProcCharacterCreate::ProcessPacket(Packet& pkt)
{
	size_t rpos = pkt.rpos();
	if (CGameProcedure::ProcessPacket(pkt))
		return true;

	pkt.rpos(rpos);

	int iCmd = pkt.read<uint8_t>();                                             // 커멘드 파싱..
	switch (iCmd)                                                               // 커멘드에 다라서 분기..
	{
		case WIZ_NEW_CHAR:                                                      // 캐릭터 선택 메시지..
		{
			uint8_t bySuccess = pkt.read<uint8_t>();                            // 커멘드 파싱..
			if (0 == bySuccess)
			{
				ProcActiveSet((CGameProcedure*) s_pProcCharacterSelect);        // 캐릭터 선택창으로 가기..
			}
			else                                                                // 실패하면.. 이유가 0 이 아닌 값으로 온다..
			{
				ReportErrorCharacterCreate((e_ErrorCharacterCreate) bySuccess); // 에러 메시지 띄움..
				s_pUIMgr->EnableOperationSet(false);                            // UI 조작 가능하게 한다... 다시 캐릭터 만들어야 한다..
			}
			s_pUIMgr->EnableOperationSet(false);                                // 패킷이 들어올때까지 UI 를 Disable 시킨다...
		}
			return true;

		default:
			break;
	}

	return false;
}
