// SubProcPerTrade.cpp: implementation of the CSubProcPerTrade class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SubProcPerTrade.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "PlayerOtherMgr.h"
#include "UIManager.h"
#include "UIMessageBox.h"
#include "UIInventory.h"
#include "UIDroppedItemDlg.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "UIPerTradeDlg.h"
#include "UITradeEditDlg.h"
#include "UIHotKeyDlg.h"
#include "UISkillTreeDlg.h"
#include "N3UIIcon.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIButton.h>

CSubProcPerTrade::CSubProcPerTrade()
{
	m_pUIPerTradeDlg          = nullptr;
	m_pUITradeEditDlg         = nullptr;
	m_ePerTradeState          = PER_TRADE_STATE_NONE;
	m_iOtherID                = -1;
	m_iGoldOffsetBackup       = 0;
	m_ePerTradeItemKindBackup = PER_TRADE_ITEM_MONEY;
}

CSubProcPerTrade::~CSubProcPerTrade()
{
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::Release()
{
	if (m_pUIPerTradeDlg)
	{
		m_pUIPerTradeDlg->Release();
		delete m_pUIPerTradeDlg;
		m_pUIPerTradeDlg = nullptr;
	}

	if (m_pUITradeEditDlg)
	{
		m_pUITradeEditDlg->Release();
		delete m_pUITradeEditDlg;
		m_pUITradeEditDlg = nullptr;
	}

	m_szMsg = ""; //MessageBox key
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::InitPerTradeDlg(CUIManager* pUIManager)
{
	int iW = CN3Base::s_CameraData.vp.Width;
	RECT rc;

	e_Nation eNation       = s_pPlayer->m_InfoBase.eNation; // 국가....
	__TABLE_UI_RESRC* pTbl = s_pTbl_UI.Find(eNation);

	// UIPerTradeDlg.. ^^
	m_pUIPerTradeDlg       = new CUIPerTradeDlg();
	m_pUIPerTradeDlg->Init(pUIManager);
	m_pUIPerTradeDlg->m_pSubProcPerTrade = this;
	m_pUIPerTradeDlg->LoadFromFile(pTbl->szPersonalTrade);
	rc = m_pUIPerTradeDlg->GetRegion();
	m_pUIPerTradeDlg->SetPos(iW - (rc.right - rc.left), 10);
	m_pUIPerTradeDlg->SetVisible(false);
	m_pUIPerTradeDlg->InitIconWnd(UIWND_PER_TRADE);
	m_pUIPerTradeDlg->SetUIType(UI_TYPE_ICON_MANAGER);
	m_pUIPerTradeDlg->SetState(UI_STATE_COMMON_NONE);

	// UITradeEditDlg.. ^^
	m_pUITradeEditDlg = new CUITradeEditDlg();
	m_pUITradeEditDlg->Init(pUIManager);
	m_pUITradeEditDlg->m_pSubProcPerTrade = this;
	m_pUITradeEditDlg->LoadFromFile(pTbl->szPersonalTradeEdit);
	m_pUITradeEditDlg->SetStyle(UISTYLE_ALWAYSTOP);
	// 위치 계산 ..
	int iXPos = 0, iYPos = 0;
	int iH = CN3Base::s_CameraData.vp.Height;
	iXPos  = (iW / 2) - (m_pUITradeEditDlg->GetRegion().right - m_pUITradeEditDlg->GetRegion().left) / 2;
	iYPos  = (iH / 2) - (m_pUITradeEditDlg->GetRegion().bottom - m_pUITradeEditDlg->GetRegion().top) / 2;
	m_pUITradeEditDlg->SetPos(iXPos, iYPos);
	m_pUITradeEditDlg->Close();
	m_pUITradeEditDlg->SetUIType(UI_TYPE_BASE);
	m_pUITradeEditDlg->SetState(UI_STATE_COMMON_NONE);

	// 일단은 돈 아이콘으로 픽스.. ^^
	N3_VERIFY_UI_COMPONENT(m_pUITradeEditDlg->m_pArea, m_pUITradeEditDlg->GetChildByID<CN3UIArea>("area_trade_icon"));

	m_pUITradeEditDlg->m_pImageOfIcon = new CN3UIImage;
	m_pUITradeEditDlg->m_pImageOfIcon->Init(m_pUITradeEditDlg);
	// 돈 아이콘 문자열 찾기.. 아이디로 찾는 기능밖에 없다.. ㅠ.ㅠ
	__TABLE_ITEM_BASIC* pItem = s_pTbl_Items_Basic.Find(dwGold); // 열 데이터 얻기..

	std::string szIconFN;
	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	CGameBase::MakeResrcFileNameForUPC(pItem, nullptr, nullptr, &szIconFN, ePart, ePlug); // 아이템에 따른 파일 이름을 만들어서
	m_pUITradeEditDlg->m_pImageOfIcon->SetTex(szIconFN);
	float fUVAspect = (float) 45.0f / (float) 64.0f;
	m_pUITradeEditDlg->m_pImageOfIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
	m_pUITradeEditDlg->m_pImageOfIcon->SetRegion(m_pUITradeEditDlg->m_pArea->GetRegion());
	m_pUITradeEditDlg->m_pImageOfIcon->SetMoveRect(m_pUITradeEditDlg->m_pArea->GetRegion());
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::EnterWaitMsgFromServerStatePerTradeReq()
{
	CPlayerOther* pTarget = s_pOPMgr->UPCGetByID(s_pPlayer->m_iIDTarget, false);
	if (pTarget == nullptr)
		return;

	m_ePerTradeState  = PER_TRADE_STATE_WAIT_FOR_REQ;

	// 메시지 박스 텍스트 표시..
	std::string szMsg = fmt::format_text_resource(IDS_PERSONAL_TRADE_FMT_WAIT, s_pPlayer->IDString(), pTarget->IDString());
	m_szMsg           = CGameProcedure::MessageBoxPost(szMsg, "", MB_CANCEL, BEHAVIOR_PERSONAL_TRADE_FMT_WAIT);

	SecureCodeBegin();
}

void CSubProcPerTrade::EnterWaitMsgFromServerStatePerTradeReq(const std::string& szName)
{
	m_ePerTradeState  = PER_TRADE_STATE_WAIT_FOR_REQ;

	// 메시지 박스 텍스트 표시..
	std::string szMsg = fmt::format_text_resource(IDS_PERSONAL_TRADE_FMT_WAIT, s_pPlayer->IDString(), szName);
	m_szMsg           = CGameProcedure::MessageBoxPost(szMsg, "", MB_CANCEL, BEHAVIOR_PERSONAL_TRADE_FMT_WAIT);

	SecureCodeBegin();
}

void CSubProcPerTrade::EnterWaitMyDecisionToPerTrade(int iOtherID) // 내가 타인에게서 아이템 거래를 신청 받은 상태..
{
	CPlayerOther* pTarget = s_pOPMgr->UPCGetByID(iOtherID, false);
	if (pTarget == nullptr)
		return;

	m_iOtherID        = iOtherID;
	m_ePerTradeState  = PER_TRADE_STATE_WAIT_FOR_MY_DECISION_AGREE_OR_DISAGREE;

	// 메시지 박스 텍스트 표시..
	std::string szMsg = fmt::format_text_resource(IDS_PERSONAL_TRADE_PERMIT, s_pPlayer->IDString(), pTarget->IDString());
	m_szMsg           = CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_PERSONAL_TRADE_PERMIT);

	SecureCodeBegin();
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::SecureCodeBegin()
{
	// 1.상거래 중이면 개인 거래 신청이 않된다.. -> 해당 부분..	ok
	// 2.상거래 중에 개인 거래 신청을 받으면 거절.. -> 해당 부분.. ok

	// 3.유저가 움직이고 있는 중이면 멈춘다..
	if (s_pPlayer->IsMovingNow())
		s_pPlayer->ToggleMoveMode(); // 자동 전진 토글..

	// 4.아이콘 매니저 윈도우는 모두 닫는다..
	//   인벤토리 윈도우이면..
	if (CGameProcedure::s_pProcMain->m_pUIInventory->IsVisible())
		CGameProcedure::s_pProcMain->m_pUIInventory->Close();

	//   Drop Item 윈도우이면..
	if (CGameProcedure::s_pProcMain->m_pUIDroppedItemDlg->IsVisible())
		CGameProcedure::s_pProcMain->m_pUIDroppedItemDlg->LeaveDroppedState();

	// 5.인풋을 막는다..	-> 해당 부분..	ok	(키입력과 메시지..)

	// 6.거래창의 편집 Control의 값을 Clear..
	CN3UIString *pStrMy = nullptr, *pStrOther = nullptr;
	N3_VERIFY_UI_COMPONENT(pStrMy, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_my"));
	pStrMy->SetString("0");

	N3_VERIFY_UI_COMPONENT(pStrOther, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_other"));
	pStrOther->SetString("0");

	// 7.개인 거래 창의 처크 버튼들 원래대로..
	CN3UIButton *pButtonMy = nullptr, *pButtonOther = nullptr;
	N3_VERIFY_UI_COMPONENT(pButtonMy, m_pUIPerTradeDlg->GetChildByID<CN3UIButton>("btn_trade_my"));
	pButtonMy->SetState(UI_STATE_BUTTON_NORMAL);

	N3_VERIFY_UI_COMPONENT(pButtonOther, m_pUIPerTradeDlg->GetChildByID<CN3UIButton>("btn_trade_other"));
	pButtonOther->SetState(UI_STATE_BUTTON_NORMAL);

	// 8.상대방 거래 버튼은 Click할 수 없다. uif 자체 기능..
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::FinalizePerTrade()
{
	m_ePerTradeState = PER_TRADE_STATE_NONE;

	if (m_pUIPerTradeDlg->IsVisible())
		m_pUIPerTradeDlg->SetVisible(false);

	CGameProcedure::MessageBoxClose(m_szMsg);
	m_szMsg = "";

	if (m_pUITradeEditDlg->IsVisible())
	{
		m_pUITradeEditDlg->SetQuantity(0);
		m_pUITradeEditDlg->Close();
	}
}

void CSubProcPerTrade::PerTradeCompleteSuccess() // 개인 거래 최종 성공..
{
	// 개인 거래 창에 있는 아이템 삭제..
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pUIPerTradeDlg->m_pPerTradeMy[i] != nullptr)
		{
			__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeMy[i];

			// 매니저에서 제거..
			m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem                             = nullptr;
			m_pUIPerTradeDlg->m_pPerTradeMy[i] = nullptr;
		}

		if (m_pUIPerTradeDlg->m_pPerTradeOther[i] != nullptr)
		{
			__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeOther[i];

			// 매니저에서 제거..
			m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem                                = nullptr;
			m_pUIPerTradeDlg->m_pPerTradeOther[i] = nullptr;
		}
	}

	m_pUIPerTradeDlg->LeavePerTradeState();
}

void CSubProcPerTrade::PerTradeCompleteCancel() // 개인 거래 취소..
{
	std::string str;
	int iGold    = 0,                           // 거래창의 값..
		iMyMoney = 0;                           // 인벤토리의 값..

	if ((int) m_ePerTradeState >= (int) PER_TRADE_STATE_NORMAL)
	{
		// 먼저 돈을 검사 한다..
		// 거래 창의 내 현재 돈을 얻어 온다..
		CN3UIString* pStrMy = nullptr;
		N3_VERIFY_UI_COMPONENT(pStrMy, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_my"));
		str                         = pStrMy->GetString();
		iGold                       = atoi(str.c_str());

		// 현재 내가 가진 돈을 얻어 온다..
		iMyMoney                    = s_pPlayer->m_InfoExt.iGold;
		// 돈을 복구 한다..
		iMyMoney                   += iGold;
		s_pPlayer->m_InfoExt.iGold  = iMyMoney;

		// 그다음 아이템을 검사한다..
		// 아이템들을 원래 대로..
		for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		{
			if (m_pUIPerTradeDlg->m_pPerTradeMy[i] != nullptr)
			{
				if ((m_pUIPerTradeDlg->m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
					|| (m_pUIPerTradeDlg->m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
				{
					// 활이나 물약등 아이템인 경우..
					__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeMy[i];

					// 기존에 아이콘이 있다면..
					if (m_pUIPerTradeDlg->m_pPerTradeInv[m_pUIPerTradeDlg->m_iBackupiOrder[i]])
					{
						// 숫자 업데이트 하고..
						m_pUIPerTradeDlg->m_pPerTradeInv[m_pUIPerTradeDlg->m_iBackupiOrder[i]]->iCount += m_pUIPerTradeDlg->m_pPerTradeMy[i]
																											  ->iCount;

						// 매니저에서 제거..
						m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

						// 리소스 제거..
						spItem->pUIIcon->Release();
						delete spItem->pUIIcon;
						spItem->pUIIcon = nullptr;
						delete spItem;
						spItem                             = nullptr;
						m_pUIPerTradeDlg->m_pPerTradeMy[i] = nullptr;
					}
					// 없다면 그대로 이동한다..
					else
					{
						__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeMy[i];
						spItem->pUIIcon->SetParent(m_pUIPerTradeDlg);

						// 인벤토리 윈도우에 세팅하고..
						m_pUIPerTradeDlg->m_pPerTradeInv[m_pUIPerTradeDlg->m_iBackupiOrder[i]] = spItem;

						// 내 거래 윈도우에서 클리어..
						m_pUIPerTradeDlg->m_pPerTradeMy[i]                                     = nullptr;

						CN3UIArea* pArea                                                       = m_pUIPerTradeDlg->GetChildAreaByiOrder(
                            UI_AREA_TYPE_PER_TRADE_INV, m_pUIPerTradeDlg->m_iBackupiOrder[i]);
						if (pArea != nullptr)
						{
							spItem->pUIIcon->SetRegion(pArea->GetRegion());
							spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
						}
					}
				}
				else
				{
					__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeMy[i];
					spItem->pUIIcon->SetParent(m_pUIPerTradeDlg);

					// 인벤토리 윈도우에 세팅하고..
					m_pUIPerTradeDlg->m_pPerTradeInv[m_pUIPerTradeDlg->m_iBackupiOrder[i]] = spItem;

					// 내 거래 윈도우에서 클리어..
					m_pUIPerTradeDlg->m_pPerTradeMy[i]                                     = nullptr;

					CN3UIArea* pArea                                                       = m_pUIPerTradeDlg->GetChildAreaByiOrder(
                        UI_AREA_TYPE_PER_TRADE_INV, m_pUIPerTradeDlg->m_iBackupiOrder[i]);
					if (pArea != nullptr)
					{
						spItem->pUIIcon->SetRegion(pArea->GetRegion());
						spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
				}
			}

			if (m_pUIPerTradeDlg->m_pPerTradeOther[i] != nullptr)
			{
				__IconItemSkill* spItem = m_pUIPerTradeDlg->m_pPerTradeOther[i];

				// 매니저에서 제거..
				m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

				// 리소스 제거..
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = nullptr;
				delete spItem;
				spItem                                = nullptr;
				m_pUIPerTradeDlg->m_pPerTradeOther[i] = nullptr;
			}
		}

		m_pUIPerTradeDlg->LeavePerTradeState();
	}
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::LeavePerTradeState(e_PerTradeResultCode ePTRC) // 아이템 거래가 취소되는 상태를 정의..
{
	uint8_t byBuff[4];                                                // 패킷 버퍼..
	int iOffset = 0;                                                  // 패킷 오프셋..

	std::string szMsg;

	switch (ePTRC)
	{
		case PER_TRADE_RESULT_MY_CANCEL: // 거래를 신청한 내가 거래 신청을 취소..
			// 서버에게 패킷 만들어서 날림..
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_CANCEL);
			CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..

			// 뒷 마무리..
			PerTradeCompleteCancel();
			FinalizePerTrade();
			break;

		case PER_TRADE_RESULT_MY_DISAGREE: // 거래를 신청받은 내가 거래 신청을 취소..
			// 서버에게 패킷 만들어서 날림..
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_AGREE);
			CAPISocket::MP_AddByte(byBuff, iOffset, 0x00);

			CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..

			// 뒷 마무리..
			FinalizePerTrade();
			break;

		case PER_TRADE_RESULT_OTHER_DISAGREE: // 거래를 신청받은 상대방이 거래 신청을 취소..
			// 메시지 박스 텍스트 표시..
			szMsg = fmt::format_text_resource(IDS_OTHER_PER_TRADE_ID_NO);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
			// 뒷 마무리..
			FinalizePerTrade();
			break;
	}
}

void CSubProcPerTrade::ProcessProceed(e_PerTradeProceedCode ePTPC) // 아이템 거래가 계속되는 상태를 정의..
{
	uint8_t byBuff[4];                                             // 패킷 버퍼..
	int iOffset = 0;                                               // 패킷 오프셋..

	switch (ePTPC)
	{
		case PER_TRADE_RESULT_MY_AGREE: // 거래를 신청받은 내가 거래 신청을 허락..
			// 서버에게 패킷 만들어서 날림..
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_AGREE);
			CAPISocket::MP_AddByte(byBuff, iOffset, 0x01);

			CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..

			CGameProcedure::MessageBoxClose(m_szMsg);
			m_szMsg = "";

			// Trade Core Start..
			PerTradeCoreStart();
			break;

		case PER_TRADE_RESULT_OTHER_AGREE: // 거래를 신청받은 상대방이 거래 신청을 허락..
			CGameProcedure::MessageBoxClose(m_szMsg);
			m_szMsg = "";

			// Trade Core Start..
			PerTradeCoreStart();
			break;
	}
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::PerTradeCoreStart()
{
	m_ePerTradeState = PER_TRADE_STATE_NORMAL;

	m_pUIPerTradeDlg->SetVisible(true);
	m_pUIPerTradeDlg->EnterPerTradeState();

	if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->IsVisible())
		CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->Close();
}

void CSubProcPerTrade::PerTradeCoreInvDisable()
{
	RECT rect = { 0, 0, 0, 0 };

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pUIPerTradeDlg->m_pPerTradeInv[i] != nullptr)
			m_pUIPerTradeDlg->m_pPerTradeInv[i]->pUIIcon->SetMoveRect(rect);
	}
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::RequestItemCountEdit()
{
	if (m_ePerTradeState != PER_TRADE_STATE_NORMAL)
		return;
	m_ePerTradeState = PER_TRARE_STATE_EDITTING;

	m_pUITradeEditDlg->Open(true);
	m_pUIPerTradeDlg->PlayGoldSound();
}

void CSubProcPerTrade::ItemCountEditOK()
{
	std::string str;
	int iGold           = 0, // 거래창의 값..
		iGoldOffset     = 0, // 편집창의 값..
		iMyMoney        = 0; // 인벤토리의 값..

	// 거래 창의 내 현재 돈을 얻어 온다..
	CN3UIString* pStrMy = nullptr;
	N3_VERIFY_UI_COMPONENT(pStrMy, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_my"));
	str                 = pStrMy->GetString();
	iGold               = atoi(str.c_str());

	//  입력 창의 값을 얻어서
	iGoldOffset         = m_pUITradeEditDlg->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGoldOffset;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney            = s_pPlayer->m_InfoExt.iGold;

	if (iGoldOffset <= 0)
		return;
	if (iGoldOffset > iMyMoney)
		return;

	// 돈을 감소 시킨다..
	iMyMoney                   -= iGoldOffset;
	s_pPlayer->m_InfoExt.iGold  = iMyMoney;

	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
	if (m_pUIPerTradeDlg->m_pStrMyGold)
		m_pUIPerTradeDlg->m_pStrMyGold->SetStringAsInt(iMyMoney);

	// 돈 표시.. 개인 거래 창..
	iGold += iGoldOffset;
	pStrMy->SetStringAsInt(iGold);

	// 서버에게 전송한다..
	uint8_t byBuff[16]; // 패킷 버퍼..
	int iOffset = 0;    // 패킷 오프셋..

	// 서버에게 패킷 만들어서 날림..
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_ADD);
	CAPISocket::MP_AddByte(byBuff, iOffset, 0xff);
	CAPISocket::MP_AddDword(byBuff, iOffset, dwGold);
	CAPISocket::MP_AddDword(byBuff, iOffset, iGoldOffset);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..

	m_pUITradeEditDlg->SetQuantity(0);

	// 상태를 변화시키고.. 창을 닫고..
	CN3UIBase::s_bWaitFromServer = true;
	m_ePerTradeState             = PER_TRADE_STATE_NORMAL;
	m_ePerTradeItemKindBackup    = PER_TRADE_ITEM_MONEY;
	m_pUITradeEditDlg->Close();

	m_pUIPerTradeDlg->PlayGoldSound();
}

void CSubProcPerTrade::ItemCountEditCancel()
{
	m_pUITradeEditDlg->SetQuantity(0);

	// 상태를 변화시키고.. 창을 닫고..
	m_ePerTradeState = PER_TRADE_STATE_NORMAL;
	m_pUITradeEditDlg->Close();

	m_pUIPerTradeDlg->PlayGoldSound();
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::PerTradeMyDecision() // 내가 거래를 결정 했다..
{
	std::string szFN     = "btn_trade_my";
	CN3UIButton* pButton = (CN3UIButton*) m_pUIPerTradeDlg->GetChildButtonByName(szFN);

	// 서버에게 전송한다..
	uint8_t byBuff[4]; // 패킷 버퍼..
	int iOffset = 0;   // 패킷 오프셋..

	// 서버에게 패킷 만들어서 날림..
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_DECIDE);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..

	// 내 결정 버튼 Disable..
	if (pButton != nullptr)
		pButton->SetState(UI_STATE_BUTTON_DISABLE);
	SecureJobStuffByMyDecision();
}

void CSubProcPerTrade::SecureJobStuffByMyDecision()
{
	m_ePerTradeState = PER_TRADE_STATE_MY_TRADE_DECISION_DONE;

	// 1. 인벤토리 영역 Icon 움직임 방지..
	PerTradeCoreInvDisable();

	// 2. 돈 입력창 클릭 방지..		해당 부분..

	// 3. 올려놓는 버튼 금지..		해당 부분..
}

void CSubProcPerTrade::PerTradeOtherDecision() // 다른 사람이 거래를 결정 했다..
{
	CN3UIButton* pButtonOther = nullptr;
	N3_VERIFY_UI_COMPONENT(pButtonOther, m_pUIPerTradeDlg->GetChildByID<CN3UIButton>("btn_trade_other"));
	pButtonOther->SetState(UI_STATE_BUTTON_DISABLE);
}

///////////////////////////////////////////////////////////////////////

void CSubProcPerTrade::ReceiveMsgPerTradeReq(int iOtherID)
{
	EnterWaitMyDecisionToPerTrade(iOtherID); // 내가 타인에게서 아이템 거래를 신청 받은 상태..
}

void CSubProcPerTrade::ReceiveMsgPerTradeAgree(uint8_t bResult)
{
	switch (bResult)
	{
		case 0x01: // 성공..
			ProcessProceed(PER_TRADE_RESULT_OTHER_AGREE);
			break;

		case 0x00: // 실패..
			LeavePerTradeState(PER_TRADE_RESULT_OTHER_DISAGREE);
			break;

		default:
			break;
	}
}

void CSubProcPerTrade::ReceiveMsgPerTradeAdd(uint8_t bResult)
{
	// 상태를 변화시키고.. 창을 닫고..
	CN3UIBase::s_bWaitFromServer = false;

	std::string str;
	int iGold    = 0, // 거래창의 값..
		iMyMoney = 0; // 인벤토리의 값..

	switch (bResult)
	{
		case 0x00: // 실패 했을 경우.. 복구한다..
			// 전에 작업한 아이템 종류가 돈인 경우..
			switch (m_ePerTradeItemKindBackup)
			{
				case PER_TRADE_ITEM_MONEY:
				{
					// 거래 창의 내 현재 돈을 얻어 온다..
					CN3UIString* pStrMy = nullptr;
					N3_VERIFY_UI_COMPONENT(pStrMy, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_my"));
					str                         = pStrMy->GetString();
					iGold                       = atoi(str.c_str());

					// 현재 내가 가진 돈을 얻어 온다..
					iMyMoney                    = s_pPlayer->m_InfoExt.iGold;
					// 돈을 증가 시킨다..
					iMyMoney                   += m_iGoldOffsetBackup;
					s_pPlayer->m_InfoExt.iGold  = iMyMoney;

					// 돈 표시.. 인벤토리..
					CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
					if (m_pUIPerTradeDlg->m_pStrMyGold)
						m_pUIPerTradeDlg->m_pStrMyGold->SetStringAsInt(iMyMoney);

					// 돈 표시.. 개인 거래 창..
					iGold -= m_iGoldOffsetBackup;
					pStrMy->SetStringAsInt(iGold);
				}
				break;

				case PER_TRADE_ITEM_OTHER:
				{
					// 전에 작업한 아이템 종류가 아이템인 경우..
					if ((m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->pItemBasic->byContable
							== UIITEM_TYPE_COUNTABLE)
						|| (m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->pItemBasic->byContable
							== UIITEM_TYPE_COUNTABLE_SMALL))
					{
						// 활이나 물약등 아이템인 경우..
						bool bFound = false;

						// 인벤토리에 아이템이 없어진 경우.. 만든다..
						if (!m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder])
						{
							bFound                  = true;

							// 인벤토리에 만들고 아이템의 갯수를 정해준다..
							__IconItemSkill *spItem = nullptr, *spItemNew = nullptr;
							spItem                = m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

							// 아이콘이 없으면 아이콘을 만드록 갯수는 0으로..
							spItemNew             = new __IconItemSkill;
							spItemNew->pItemBasic = spItem->pItemBasic;
							spItemNew->pItemExt   = spItem->pItemExt;
							spItemNew->szIconFN   = spItem->szIconFN; // 아이콘 파일 이름 복사..
							spItemNew->iCount     = m_pUIPerTradeDlg->m_iBackupiCount;
							spItemNew->iDurability = spItem->iDurability;

							// 아이콘 리소스 만들기..
							spItemNew->pUIIcon     = new CN3UIIcon;
							float fUVAspect        = (float) 45.0f / (float) 64.0f;
							spItemNew->pUIIcon->Init(m_pUIPerTradeDlg);
							spItemNew->pUIIcon->SetTex(spItemNew->szIconFN);
							spItemNew->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
							spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
							spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
							spItemNew->pUIIcon->SetVisible(true);

							CN3UIArea* pArea = m_pUIPerTradeDlg->GetChildAreaByiOrder(
								UI_AREA_TYPE_PER_TRADE_INV, CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
							if (pArea != nullptr)
							{
								spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
								spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
							}

							m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemNew;

							// 내 거래창의 아이템이 있는 경우 .. 갯수 갱신..
							if (m_pUIPerTradeDlg->m_iBackupiCount
								!= m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount)
							{
								m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]
									->iCount -= m_pUIPerTradeDlg->m_iBackupiCount;
							}
						}

						// 아이콘이 내 거래창의 슬롯에 생긴경우.. 삭제한다..
						if (m_pUIPerTradeDlg->m_iBackupiCount
							== m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount)
						{
							bFound                  = true;

							__IconItemSkill* spItem = m_pUIPerTradeDlg
														  ->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

							// 매니저에서 제거..
							m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

							// 리소스 제거..
							spItem->pUIIcon->Release();
							delete spItem->pUIIcon;
							spItem->pUIIcon = nullptr;
							delete spItem;
							spItem                                                                                  = nullptr;
							m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;

							// 인벤토리에 아이템이 있는 경우.. 갯수 갱신..
							if (m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder])
							{
								m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder]
									->iCount += m_pUIPerTradeDlg->m_iBackupiCount;
							}
						}

						// 위에서 갯수 보정한게 아니면.. 갯수만 보정한다..
						if (!bFound)
						{
							// 내 거래창과 인벤토리의 아이템의 갯수를 정해준다..
							m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder]
								->iCount += m_pUIPerTradeDlg->m_iBackupiCount;

							m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]
								->iCount -= m_pUIPerTradeDlg->m_iBackupiCount;
						}
					}
					else
					{
						__IconItemSkill* spItem = CN3UIWndBase::s_sRecoveryJobInfo.pItemSource;
						spItem->pUIIcon->SetParent(m_pUIPerTradeDlg);

						// 인벤토리 윈도우에 세팅하고..
						m_pUIPerTradeDlg->m_pPerTradeInv[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItem;

						// 내 거래 윈도우에서 클리어..
						m_pUIPerTradeDlg->m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]    = nullptr;

						CN3UIArea* pArea = m_pUIPerTradeDlg->GetChildAreaByiOrder(
							UI_AREA_TYPE_PER_TRADE_INV, CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
						if (pArea != nullptr)
						{
							spItem->pUIIcon->SetRegion(pArea->GetRegion());
							spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
						}
					}
				}
				break;
			}
			break;

		default:
			break;
	}
}

void CSubProcPerTrade::ReceiveMsgPerTradeOtherAdd(int iItemID, int iCount, int iDurability)
{
	std::string str;
	int iGold = 0, iDestiOrder = -1; // 거래창의 값..

	if (iItemID == dwGold)
	{
		// 거래 창의 다른 사람의 현재 돈을 얻어 온다..
		CN3UIString* pStrOther = nullptr;
		N3_VERIFY_UI_COMPONENT(pStrOther, m_pUIPerTradeDlg->GetChildByID<CN3UIString>("string_money_other"));
		str    = pStrOther->GetString();
		iGold  = atoi(str.c_str());

		// 돈을 더한 다음..
		iGold += iCount;

		// 돈 표시.. 개인 거래 창..
		pStrOther->SetStringAsInt(iGold);
	}
	else
	{
		// 아이템이 들어갈 수 있는지 확인, 아이템이 들어 가는 자리 계산..
		bool bFound                = false;

		// 아이템 만들어서 넣기..
		__TABLE_ITEM_BASIC* pItem  = nullptr;
		__TABLE_ITEM_EXT* pItemExt = nullptr;
		pItem                      = s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000); // 열 데이터 얻기..
		if (pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);
		if (nullptr == pItem || nullptr == pItemExt)
		{
			__ASSERT(0, "아이템 포인터 테이블에 없음!!");
			return;
		}

		if ((pItem->byContable == UIITEM_TYPE_COUNTABLE) || (pItem->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
		{
			for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
			{
				if ((m_pUIPerTradeDlg->m_pPerTradeOther[i]) && (m_pUIPerTradeDlg->m_pPerTradeOther[i]->pItemBasic->dwID == pItem->dwID))
				{
					bFound      = true;
					iDestiOrder = i;
					break;
				}
			}

			// 못찾았으면..
			if (!bFound)
			{
				// 인벤토리 빈슬롯을 찾아 들어간다..
				for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
				{
					if (!m_pUIPerTradeDlg->m_pPerTradeOther[i])
					{
						bFound      = true;
						iDestiOrder = i;
						break;
					}
				}

				if (!bFound) // 빈 슬롯을 찾지 못했으면..
					return;
			}

			if (m_pUIPerTradeDlg->m_pPerTradeOther[iDestiOrder]) // 해당 위치에 아이콘이 있으면..
			{
				m_pUIPerTradeDlg->m_pPerTradeOther[iDestiOrder]->iCount += iCount;
			}
			else
			{
				std::string szIconFN;
				e_PartPosition ePart = PART_POS_UNKNOWN;
				e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
				CGameBase::MakeResrcFileNameForUPC(
					pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug, RACE_UNKNOWN); // 아이템에 따른 파일 이름을 만들어서

				__IconItemSkill* spItem = nullptr;

				spItem                  = new __IconItemSkill;
				spItem->pItemBasic      = pItem;
				spItem->pItemExt        = pItemExt;
				spItem->szIconFN        = szIconFN; // 아이콘 파일 이름 복사..
				spItem->iCount          = iCount;
				spItem->iDurability     = iDurability;
				float fUVAspect         = (float) 45.0f / (float) 64.0f;
				spItem->pUIIcon         = new CN3UIIcon;
				spItem->pUIIcon->Init(m_pUIPerTradeDlg);
				spItem->pUIIcon->SetTex(szIconFN);
				spItem->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
				spItem->pUIIcon->SetUIType(UI_TYPE_ICON);
				spItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
				spItem->pUIIcon->SetVisible(true);

				CN3UIArea* pArea = nullptr;
				//pArea = m_pUIPerTradeDlg->GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, i);
				if (pArea)
				{
					spItem->pUIIcon->SetRegion(pArea->GetRegion());
					// 움직일 수 없다..
					RECT rect = { 0, 0, 0, 0 };
					spItem->pUIIcon->SetMoveRect(rect);
				}

				//m_pUIPerTradeDlg->m_pPerTradeOther[i] = spItem;
			}
		}
		else
		{
			for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
			{
				if (m_pUIPerTradeDlg->m_pPerTradeOther[i] == nullptr)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
				return; // 못 찾았으므로.. 실패..

			std::string szIconFN;
			e_PartPosition ePart = PART_POS_UNKNOWN;
			e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
			CGameBase::MakeResrcFileNameForUPC(
				pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug, RACE_UNKNOWN); // 아이템에 따른 파일 이름을 만들어서

			__IconItemSkill* spItem = nullptr;

			spItem                  = new __IconItemSkill;
			spItem->pItemBasic      = pItem;
			spItem->pItemExt        = pItemExt;
			spItem->szIconFN        = szIconFN; // 아이콘 파일 이름 복사..
			spItem->iCount          = 1;
			spItem->iDurability     = iDurability;
			float fUVAspect         = (float) 45.0f / (float) 64.0f;
			spItem->pUIIcon         = new CN3UIIcon;
			spItem->pUIIcon->Init(m_pUIPerTradeDlg);
			spItem->pUIIcon->SetTex(szIconFN);
			spItem->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
			spItem->pUIIcon->SetUIType(UI_TYPE_ICON);
			spItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
			spItem->pUIIcon->SetVisible(true);

			CN3UIArea* pArea = nullptr;
			//pArea = m_pUIPerTradeDlg->GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, i);
			if (pArea)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				// 움직일 수 없다..
				RECT rect = { 0, 0, 0, 0 };
				spItem->pUIIcon->SetMoveRect(rect);
			}

			if (spItem->iDurability == 0)
				spItem->pUIIcon->SetStyle(spItem->pUIIcon->GetStyle() | UISTYLE_DURABILITY_EXHAUST);

			//m_pUIPerTradeDlg->m_pPerTradeOther[i] = spItem;
		}
	}
}

void CSubProcPerTrade::ReceiveMsgPerTradeOtherDecide()
{
	PerTradeOtherDecision();
}

void CSubProcPerTrade::ReceiveMsgPerTradeDoneSuccessBegin(int iTotalGold)
{
	s_pPlayer->m_InfoExt.iGold = iTotalGold;
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
}

void CSubProcPerTrade::ReceiveMsgPerTradeDoneItemMove(uint8_t bItemPos, int iItemID, int iCount, int iDurability)
{
	__IconItemSkill* spItem = nullptr;

	// 아이템 인벤토리 INV 영역에 추가..
	// 아이템이 들어갈 수 있는지 확인..
	if (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos] != nullptr)
	{
		if (iItemID
			!= (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->pItemBasic->dwID / 1000 * 1000)
				   + (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->pItemExt->dwID % 1000))
		{
			// 기존 아이콘을 클리어..
			spItem = m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos];

			// 매니저에서 제거..
			m_pUIPerTradeDlg->RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem                                     = nullptr;
			m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos] = nullptr;

			// 서버가 준 아이디로 아이콘을 만든다..
		}
		else
		{
			switch (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->pItemBasic->byContable)
			{
				case UIITEM_TYPE_COUNTABLE: // 화살 같은 종류..
					m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount += iCount;
					if (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount > UIITEM_COUNT_MANY)
						m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount = UIITEM_COUNT_MANY;
					return;

				case UIITEM_TYPE_COUNTABLE_SMALL: // 물약같은 종류..
					m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount += iCount;
					if (m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount > UIITEM_COUNT_FEW)
						m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos]->iCount = UIITEM_COUNT_FEW;
					return;

				default:
					break;
			}
		}
	}

	// 아이템 만들어서 넣기..
	__TABLE_ITEM_BASIC* pItem  = nullptr;
	__TABLE_ITEM_EXT* pItemExt = nullptr;
	pItem                      = s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000); // 열 데이터 얻기..
	if (pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);
	if (nullptr == pItem || nullptr == pItemExt)
	{
		__ASSERT(0, "아이템 포인터 테이블에 없음!!");
		return;
	}

	std::string szIconFN;
	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	CGameBase::MakeResrcFileNameForUPC(
		pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug, RACE_UNKNOWN); // 아이템에 따른 파일 이름을 만들어서

	spItem              = new __IconItemSkill;
	spItem->pItemBasic  = pItem;
	spItem->pItemExt    = pItemExt;
	spItem->szIconFN    = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount      = iCount;
	spItem->iDurability = iDurability;

	float fUVAspect     = (float) 45.0f / (float) 64.0f;
	spItem->pUIIcon     = new CN3UIIcon;
	spItem->pUIIcon->Init(m_pUIPerTradeDlg);
	spItem->pUIIcon->SetTex(szIconFN);
	spItem->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
	spItem->pUIIcon->SetUIType(UI_TYPE_ICON);
	spItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
	spItem->pUIIcon->SetVisible(true);

	CN3UIArea* pArea = nullptr;
	pArea            = m_pUIPerTradeDlg->GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, bItemPos);
	if (pArea)
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	if (spItem->iDurability == 0)
		spItem->pUIIcon->SetStyle(spItem->pUIIcon->GetStyle() | UISTYLE_DURABILITY_EXHAUST);

	m_pUIPerTradeDlg->m_pPerTradeInv[bItemPos] = spItem;
}

void CSubProcPerTrade::ReceiveMsgPerTradeDoneSuccessEnd()
{
	// 뒷 마무리..
	PerTradeCompleteSuccess();
	FinalizePerTrade();

	if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg)
		CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
	if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg)
		CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
}

void CSubProcPerTrade::ReceiveMsgPerTradeDoneFail()
{
	if (s_pOPMgr->UPCGetByID(m_iOtherID, false) != nullptr)
	{
		std::string szMsg = fmt::format_text_resource(IDS_PER_TRADE_FAIL);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffffffff);

		szMsg = fmt::format_text_resource(IDS_ITEM_TOOMANY_OR_HEAVY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	PerTradeCompleteCancel();
	FinalizePerTrade();
}

void CSubProcPerTrade::ReceiveMsgPerTradeCancel()
{
	// 메시지 박스 텍스트 표시..
	CPlayerOther* pUPC = s_pOPMgr->UPCGetByID(m_iOtherID, false);
	if (pUPC != nullptr)
	{
		std::string szMsg = fmt::format_text_resource(IDS_OTHER_PER_TRADE_CANCEL, pUPC->IDString());
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	// 뒷 마무리..
	PerTradeCompleteCancel();
	FinalizePerTrade();
}

void CSubProcPerTrade::ItemCountOK()
{
	m_pUIPerTradeDlg->ItemCountOK();
}

void CSubProcPerTrade::ItemCountCancel()
{
	m_pUIPerTradeDlg->ItemCountCancel();
}
