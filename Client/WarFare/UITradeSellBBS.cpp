// UITradeSellBBS.cpp: implementation of the CUITradeSellBBS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UITradeSellBBS.h"
#include "GameProcMain.h"
#include "UITradeBBSEditDlg.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "text_resources.h"

#include <N3Base/N3UIList.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define CHILD_UI_SELL_MSG			1
#define CHILD_UI_TRADE_MSG			2
#define CHILD_UI_EXPLANATION_EDIT	3
#define CHILD_UI_EXPLANATION		4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define TRADE_BBS_MAXSTRING	69
#define TRADE_BBS_MAX_LINE	23

CUITradeSellBBS::CUITradeSellBBS()
{
//	m_pList_Infos			= NULL;
	m_pBtn_PageUp			= NULL;
	m_pBtn_PageDown			= NULL;
	m_pBtn_Refresh			= NULL;
	m_pBtn_Close			= NULL;
	m_pBtn_Register			= NULL;
	m_pBtn_RegisterCancel	= NULL;
	m_pBtn_Whisper			= NULL;
	m_pBtn_Trade			= NULL;

	m_pImage_Sell			= NULL;
	m_pImage_Buy			= NULL;
	m_pImage_Sell_Title		= NULL;
	m_pImage_Buy_Title		= NULL;

	m_pString_Page			= NULL;

	m_byBBSKind				= 0;
	m_iCurPage				= 0;
	m_iMaxPage				= 0;
	m_bProcessing			= false;
	m_fTime					= 0.0f;
	m_iCurIndex				= -1;
}

CUITradeSellBBS::~CUITradeSellBBS()
{
	m_Datas.clear();
}

bool CUITradeSellBBS::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

//	m_pList_Infos			= (CN3UIList*)(this->GetChildByID("List_Friends"));		__ASSERT(m_pList_Infos, "NULL UI Component!!!");
	m_pBtn_PageUp			= (CN3UIButton*)(this->GetChildByID("btn_page_up"));	__ASSERT(m_pBtn_PageUp, "NULL UI Component!!!");
	m_pBtn_PageDown			= (CN3UIButton*)(this->GetChildByID("btn_page_down"));	__ASSERT(m_pBtn_PageDown, "NULL UI Component!!!");
	m_pBtn_Refresh			= (CN3UIButton*)(this->GetChildByID("btn_refresh"));	__ASSERT(m_pBtn_Refresh, "NULL UI Component!!!");
	m_pBtn_Close			= (CN3UIButton*)(this->GetChildByID("btn_exit"));		__ASSERT(m_pBtn_Close, "NULL UI Component!!!");
	m_pBtn_Register			= (CN3UIButton*)(this->GetChildByID("btn_add"));		__ASSERT(m_pBtn_Register, "NULL UI Component!!!");
	m_pBtn_Whisper			= (CN3UIButton*)(this->GetChildByID("btn_whisper"));	__ASSERT(m_pBtn_Whisper, "NULL UI Component!!!");
	m_pBtn_Trade			= (CN3UIButton*)(this->GetChildByID("btn_sale"));		__ASSERT(m_pBtn_Trade, "NULL UI Component!!!");
	m_pBtn_RegisterCancel	= (CN3UIButton*)(this->GetChildByID("btn_delete"));		__ASSERT(m_pBtn_RegisterCancel, "NULL UI Component!!!");

	m_pImage_Sell			= (CN3UIImage*)(this->GetChildByID("img_sell gold"));	__ASSERT(m_pImage_Sell, "NULL UI Component!!!");
	m_pImage_Buy			= (CN3UIImage*)(this->GetChildByID("img_buy gold"));	__ASSERT(m_pImage_Buy, "NULL UI Component!!!");
	m_pImage_Sell_Title		= (CN3UIImage*)(this->GetChildByID("img_sell"));		__ASSERT(m_pImage_Sell_Title, "NULL UI Component!!!");
	m_pImage_Buy_Title		= (CN3UIImage*)(this->GetChildByID("img_buy"));			__ASSERT(m_pImage_Buy_Title, "NULL UI Component!!!");

	m_pString_Page			= (CN3UIString*)(this->GetChildByID("string_page"));	__ASSERT(m_pString_Page, "NULL UI Component!!!");

	std::string szID;
	for(int i = 0; i < TRADE_BBS_MAXSTRING; i++)
	{
		szID = fmt::format("text_{:02}", i);
		m_pText[i] = (CN3UIString*) GetChildByID(szID);
	}

	m_iCurPage = 0; // 현재 페이지..

	__TABLE_UI_RESRC*	pTblUI	= NULL;
	pTblUI = CGameBase::s_pTbl_UI.Find(NATION_ELMORAD);

	m_MsgBox.LoadFromFile(pTblUI->szMessageBox);

	RECT rt = m_MsgBox.GetRegion();
	POINT pt;
	pt.x = (CN3Base::s_CameraData.vp.Width - (rt.right - rt.left)) / 2;
	pt.y = (CN3Base::s_CameraData.vp.Height - (rt.bottom - rt.top)) / 2;
	m_MsgBox.SetPos(pt.x, pt.y);

	m_UIExplanation.LoadFromFile(pTblUI->szTradeMemolist);

	rt = m_UIExplanation.GetRegion();
	pt.x = (CN3Base::s_CameraData.vp.Width - (rt.right - rt.left)) / 2;
	pt.y = (CN3Base::s_CameraData.vp.Height - (rt.bottom - rt.top)) / 2;
	m_UIExplanation.SetPos(pt.x, pt.y);

	return true;
}

bool CUITradeSellBBS::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	int iID = -1;
	if( dwMsg == UIMSG_BUTTON_CLICK )
	{
		if(pSender == m_pBtn_Refresh)
		{
			float fTime = CN3Base::TimeGet();
			if( fTime - m_fTime < 3.0f )
				return true;//너무 자주 새데이터 요청을 못하게 함 3초에 한번정도로 제약을 둠.
			m_fTime = fTime;

			this->MsgSend_RefreshData(m_iCurPage);
		}
		else if(pSender == m_pBtn_PageUp)
		{
			int iCurPage = m_iCurPage;
			iCurPage--;
			if(iCurPage >= 0)
			{
				this->MsgSend_RefreshData(iCurPage);
			}
		}
		else if(pSender == m_pBtn_PageDown)
		{
			int iCurPage = m_iCurPage;
			iCurPage++;
			if(iCurPage < m_iMaxPage)
			{
				this->MsgSend_RefreshData(iCurPage);
			}
		}
		else if(pSender == m_pBtn_Close)
		{
			m_iCurPage = 0;
			m_fTime = 0.0f;
			this->SetVisible(false);
		}
		else if(pSender == m_pBtn_Register)
		{
			m_pBtn_Register->SetState(UI_STATE_BUTTON_NORMAL);
			OnButtonRegister();
		}
		else if(pSender == m_pBtn_RegisterCancel)
		{
			OnButtonRegisterCancel();
		}
		else if(pSender == m_pBtn_Whisper)
		{
			OnButtonWhisper();
		}
		else if(pSender == m_pBtn_Trade)
		{
			m_pBtn_Trade->SetState(UI_STATE_BUTTON_NORMAL);
			if(m_bProcessing == false)
				OnButtonTrade();
		}
	}
	else if( dwMsg == UIMSG_STRING_LCLICK )
	{
		if(SelectedString(pSender, iID))
		{
			m_iCurIndex = iID;
		}
	}
	else if( dwMsg == UIMSG_STRING_LDCLICK )
	{
		OnListExplanation();
	}

	return true;
}

void CUITradeSellBBS::MsgRecv_TradeBBS(Packet& pkt)
{
	m_bProcessing	= false;

	uint8_t bySubType	= pkt.read<uint8_t>();
	uint8_t byBBSKind	= pkt.read<uint8_t>();
	uint8_t byResult	= pkt.read<uint8_t>();

	if (byResult != 0x01)
	{
		uint8_t bySubResult = pkt.read<uint8_t>();
		if (bySubType == N3_SP_TYPE_BBS_OPEN)
		{
			std::string szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL6);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff0000);
		}
		else if (bySubType == N3_SP_TYPE_REGISTER)
		{
			std::string szMsg;

			switch (bySubResult)
			{
				case 1://1: 일반적인 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL1);
					break;

				case 2://2: 돈이 없어서 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL2);
					break;

				case 3://3: 항목이 없어서 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL4);
					break;
			}

			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff0000);
		}
		else if (bySubType == N3_SP_TYPE_REGISTER_CANCEL)
		{
			std::string szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL3);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff0000);
		}
		else if (bySubType == N3_SP_TYPE_BBS_DATA)
		{
		}
		else if (bySubType == N3_SP_TYPE_BBS_TRADE)
		{
			std::string szMsg;

			switch (bySubResult)
			{
				case 1://1: 일반적인 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL5);
					break;
				case 2://2: 돈이 없어서 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL2);
					break;
				case 3://3: 항목이 없어서 실패
					szMsg = fmt::format_text_resource(IDS_TRADE_BBS_FAIL4);
					break;
			}

			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff0000);
		}
		return; //실패했다면
	}

	if (bySubType == N3_SP_TYPE_BBS_OPEN)
	{
		if (!IsVisible()) SetVisible(true);

		if (byBBSKind == N3_SP_TRADE_BBS_BUY)
		{
			m_byBBSKind = N3_SP_TRADE_BBS_BUY;

			if (m_pImage_Sell != nullptr)
				m_pImage_Sell->SetVisible(false);

			if (m_pImage_Buy != nullptr)
				m_pImage_Buy->SetVisible(true);

			if (m_pImage_Sell_Title != nullptr)
				m_pImage_Sell_Title->SetVisible(false);

			if (m_pImage_Buy_Title != nullptr)
				m_pImage_Buy_Title->SetVisible(true);
		}
		else
		{
			m_byBBSKind = N3_SP_TRADE_BBS_SELL;

			if (m_pImage_Sell != nullptr)
				m_pImage_Sell->SetVisible(true);

			if (m_pImage_Buy != nullptr)
				m_pImage_Buy->SetVisible(false);

			if (m_pImage_Sell_Title != nullptr)
				m_pImage_Sell_Title->SetVisible(true);

			if (m_pImage_Buy_Title != nullptr)
				m_pImage_Buy_Title->SetVisible(false);
		}
	}
	else if (bySubType == N3_SP_TYPE_BBS_TRADE)
	{
		CGameProcedure::s_pProcMain->MsgSend_PerTradeBBSReq(m_ITSB.szID, m_ITSB.sID);
		SetVisible(false);
		return;
	}

	MsgRecv_RefreshData(pkt);
}

void CUITradeSellBBS::MsgRecv_RefreshData(Packet& pkt)
{
	int iLen;
	m_Datas.clear();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++ )
	{
		__InfoTradeSellBBS Info;
		Info.sID = pkt.read<int16_t>();
		iLen = pkt.read<int16_t>();
		if(iLen>0) pkt.readString(Info.szID, iLen);
		iLen = pkt.read<int16_t>();
		if(iLen>0) pkt.readString(Info.szTitle, iLen);
		iLen = pkt.read<int16_t>();
		if(iLen>0) pkt.readString(Info.szExplanation, iLen);
		Info.iPrice = pkt.read<uint32_t>();		//아이템에 제시한 가격
		Info.sIndex = pkt.read<int16_t>();		//등록된 인덱스

		if( Info.sID != -1 )
			m_Datas.push_back(Info);
	}

	int16_t sPage = pkt.read<int16_t>();
	int16_t sTotal = pkt.read<int16_t>();

	//TRACE("TRADE_BBS_PAGE:%d\n",sPage);
	m_iCurPage = sPage;
	m_iMaxPage = sTotal / TRADE_BBS_MAX_LINE;
	if( (sTotal % TRADE_BBS_MAX_LINE) > 0 )
		m_iMaxPage++;

	RefreshPage();
}

void CUITradeSellBBS::RefreshPage()
{
	if(m_pString_Page) m_pString_Page->SetStringAsInt(m_iCurPage+1); // 페이지 표시..

	ResetContent();

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++ )
	{
		if(it==m_Datas.end()) break;

		__InfoTradeSellBBS ITSB = (*it);
		SetContentString(i, ITSB.szID.c_str(), ITSB.iPrice, ITSB.szTitle.c_str());
		it++;
	}
}

void CUITradeSellBBS::MsgSend_RefreshData(int iCurPage)
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면

	uint8_t byBuff[10];
	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);	
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_DATA);
	CAPISocket::MP_AddByte(byBuff, iOffset, m_byBBSKind);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)iCurPage);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	m_bProcessing = true;

}

void CUITradeSellBBS::MsgSend_Register()
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면
	if(!CGameProcedure::s_pProcMain->m_pUITradeBBSEdit) return;
	int16_t sLen = 0;
	std::string szTitle;
	std::string szExplanation;
	int	iPrice = 0;

	szTitle			= CGameProcedure::s_pProcMain->m_pUITradeBBSEdit->GetTradeTitle();
	szExplanation	= CGameProcedure::s_pProcMain->m_pUITradeBBSEdit->GetTradeExplanation();
	iPrice			= CGameProcedure::s_pProcMain->m_pUITradeBBSEdit->GetPrice();

	sLen = 15;
	sLen += (int16_t)szTitle.size();
	sLen += (int16_t)szExplanation.size();

	uint8_t* byBuff = new uint8_t[sLen];
	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);	
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_REGISTER);
	CAPISocket::MP_AddByte(byBuff, iOffset, m_byBBSKind);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)szTitle.size());
	CAPISocket::MP_AddString(byBuff, iOffset, szTitle);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)szExplanation.size());
	CAPISocket::MP_AddString(byBuff, iOffset, szExplanation);
	CAPISocket::MP_AddDword(byBuff, iOffset, iPrice);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	m_bProcessing = true;
	delete [] byBuff;
}

void CUITradeSellBBS::MsgSend_RegisterCancel(int16_t sIndex)
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면

	uint8_t byBuff[10];
	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_REGISTER_CANCEL);
	CAPISocket::MP_AddByte(byBuff, iOffset, m_byBBSKind);
	CAPISocket::MP_AddShort(byBuff, iOffset, sIndex);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	m_bProcessing = true;
}

void CUITradeSellBBS::CallBackProc(int iID, uint32_t dwFlag)
{
	//TRACE("OnButton ID:%d Btn %d\n",iID, dwFlag);

	if(iID == CHILD_UI_SELL_MSG)
	{
		if(dwFlag == 1)//OK
		{
			if(CGameProcedure::s_pProcMain->m_pUITradeBBSEdit)
				CGameProcedure::s_pProcMain->m_pUITradeBBSEdit->ShowWindow(CHILD_UI_EXPLANATION_EDIT,this);
		}
	}
	else if(iID == CHILD_UI_TRADE_MSG)
	{
		if(dwFlag == 1)//OK
		{
			MsgSend_PerTrade();
		}
	}
	else if(iID == CHILD_UI_EXPLANATION_EDIT)
	{
		if(dwFlag == 1)//OK
		{
			MsgSend_Register();
		}
		else //CANCEL
		{
		}
	}
	else if(iID == CHILD_UI_EXPLANATION)
	{
		if(dwFlag == 1)//pageup
		{
			RefreshExplanation(true);
		}
		else if(dwFlag == 2)//pagedown
		{
			RefreshExplanation(false);
		}
	}

}

void CUITradeSellBBS::OnButtonRegister()
{
	// 전에 보낸 패킷 응답이 없으면
	if (m_bProcessing)
		return;

	if (m_byBBSKind == N3_SP_TRADE_BBS_BUY)
	{
		std::string szMsg = fmt::format_text_resource(IDS_TRADE_BBS_BUY_REGISTER, 500);

		m_MsgBox.SetBoxStyle(MB_YESNO);
		m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
		m_MsgBox.SetTitle("");
		m_MsgBox.SetText(szMsg);
		m_MsgBox.ShowWindow(CHILD_UI_SELL_MSG, this);
	}
	else
	{
		std::string szMsg = fmt::format_text_resource(IDS_TRADE_BBS_SELL_REGISTER, 1000);

		m_MsgBox.SetBoxStyle(MB_YESNO);
		m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
		m_MsgBox.SetTitle("");
		m_MsgBox.SetText(szMsg);
		m_MsgBox.ShowWindow(CHILD_UI_SELL_MSG, this);
	}
}

void CUITradeSellBBS::OnButtonRegisterCancel()
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면
	if(m_iCurIndex <= -1) return;

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++, it++ )
	{
		if( it == m_Datas.end() ) break;
		if( i == m_iCurIndex )
		{
			__InfoTradeSellBBS ITSB = (*it);

			if(0 == lstrcmpi(ITSB.szID.c_str(), CGameProcedure::s_pPlayer->m_InfoBase.szID.c_str()))
			{//자기것만 등록해제하게..
				MsgSend_RegisterCancel(ITSB.sIndex);
				break;
			}
			else if(AUTHORITY_MANAGER == CGameProcedure::s_pProcMain->s_pPlayer->m_InfoBase.iAuthority)
			{//운영자에게는 해제 권한을 준다...(도배나 욕설등의 게시물 삭제를 위해서...)
				MsgSend_RegisterCancel(ITSB.sIndex);
				break;
			}
		}
	}//for(
}

void CUITradeSellBBS::OnButtonWhisper()
{
	if(m_iCurIndex <= -1) return;

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++, it++ )
	{
		if( it == m_Datas.end() ) break;
		if( i == m_iCurIndex )
		{
			__InfoTradeSellBBS ITSB = (*it);
			if(0 != lstrcmpi(ITSB.szID.c_str(), CGameProcedure::s_pPlayer->m_InfoBase.szID.c_str()))
			{//나 자신에게는 귓속말을 못하게 한다...
				CGameProcedure::s_pProcMain->MsgSend_ChatSelectTarget(ITSB.szID);
			}
			break;
		}
	}
}

void CUITradeSellBBS::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUITradeSellBBS::OnButtonTrade()
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면

	if(m_iCurIndex <= -1) return;

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++, it++ )
	{
		if( it == m_Datas.end() ) break;
		if( i == m_iCurIndex )
		{
			__InfoTradeSellBBS ITSB = (*it);

			if (0 != lstrcmpi(ITSB.szID.c_str(), CGameProcedure::s_pPlayer->m_InfoBase.szID.c_str()))
			{
				std::string szMsg = fmt::format_text_resource(IDS_TRADE_BBS_PER_TRADE, 5000);

				m_ITSB = ITSB;
				m_MsgBox.SetBoxStyle(MB_YESNO);
				m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
				m_MsgBox.SetTitle("");
				m_MsgBox.SetText(szMsg);
				m_MsgBox.ShowWindow(CHILD_UI_TRADE_MSG, this);
				break;
			}
		}
	}//for(
}

void CUITradeSellBBS::RefreshExplanation(bool bPageUp)
{
	if(m_iCurIndex <= -1) return;

	if(bPageUp)
	{
		if(m_iCurIndex == 0) return;
		m_iCurIndex--;
	}
	else
	{
		int iCnt = m_Datas.size();
		if((m_iCurIndex+1) >= iCnt) return;
		m_iCurIndex++;
	}

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++, it++ )
	{
		if( it == m_Datas.end() ) break;
		if( i == m_iCurIndex )
		{
			__InfoTradeSellBBS ITSB = (*it);

			m_UIExplanation.SetExplanation(m_iCurIndex,ITSB.szExplanation);
			break;
		}
	}//for(
}

void CUITradeSellBBS::OnListExplanation()
{
	if(m_iCurIndex <= -1) return;

	it_TradeSellBBS it = m_Datas.begin();

	for( int i = 0 ; i < TRADE_BBS_MAX_LINE ; i++, it++ )
	{
		if( it == m_Datas.end() ) break;
		if( i == m_iCurIndex )
		{
			__InfoTradeSellBBS ITSB = (*it);

			m_UIExplanation.ShowWindow(CHILD_UI_EXPLANATION, this);
			m_UIExplanation.SetExplanation(m_iCurIndex,ITSB.szExplanation);
			break;
		}
	}//for(
}

void CUITradeSellBBS::MsgSend_PerTrade()
{
	if(m_bProcessing) return; //전에 보낸 패킷 응답이 없으면
	if(0 == lstrcmpi(m_ITSB.szID.c_str(), CGameProcedure::s_pPlayer->m_InfoBase.szID.c_str()))
		return; //자기 자신에게는 거래를 하지 못하게

	uint8_t byBuff[10];

	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);	
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_TRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, m_byBBSKind);
	CAPISocket::MP_AddShort(byBuff, iOffset, m_ITSB.sIndex);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	m_bProcessing = true;
}

bool CUITradeSellBBS::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUITradeSellBBS::RenderSelectContent()
{
	if(!IsVisible())	return;
	if(m_iCurIndex < 0)	return;
	if(m_iCurIndex >= TRADE_BBS_MAX_LINE) return;

	RECT rc, rc1;
	if(m_pText[m_iCurIndex])
	{
		rc = m_pText[m_iCurIndex]->GetRegion();
		if(m_pText[m_iCurIndex + TRADE_BBS_MAX_LINE*2])
		{
			rc1 = m_pText[m_iCurIndex + TRADE_BBS_MAX_LINE*2]->GetRegion();
			rc.right = rc1.right;
		}
	}
	else
		return;

	__VertexTransformedColor vLines[5];
	vLines[0].Set((float)rc.left, (float)rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[1].Set((float)rc.right, (float)rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[2].Set((float)rc.right, (float)rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[3].Set((float)rc.left, (float)rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[4] = vLines[0];

	DWORD dwZ, dwFog, dwAlpha, dwCOP, dwCA1, dwSrcBlend, dwDestBlend, dwVertexShader, dwAOP, dwAA1;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
	CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
	CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, vLines, sizeof(__VertexTransformedColor));
	
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
	CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
}

bool CUITradeSellBBS::SelectedString(CN3UIBase* pSender, int& iID)
{
	int iIndex = -1;
	for (int i = 0; i < TRADE_BBS_MAXSTRING; i++)
	{
		if (pSender == m_pText[i])
		{
			iIndex = i % TRADE_BBS_MAX_LINE;
			if (iIndex >= static_cast<int>(m_Datas.size()))
				return false;

			iID = iIndex;
			return true;
		}
	}

	return false;
}

void CUITradeSellBBS::Render()
{
	if(!IsVisible()) return;

	CN3UIBase::Render();
	RenderSelectContent();
}

void CUITradeSellBBS::ResetContent()
{
	if(m_Datas.size()>0)
		m_iCurIndex = 0;
	else
		m_iCurIndex = -1;

	for(int i = 0 ; i < TRADE_BBS_MAXSTRING ; i++)
	{
		if(m_pText[i])
		{
			m_pText[i]->SetString("");
			m_pText[i]->SetColor(0xffffffff);
		}
	}
}

void CUITradeSellBBS::SetContentString(int iIndex, std::string szID, int iPrice, std::string szTitle)
{
	std::string szGold = fmt::format_text_resource(IDS_TOOLTIP_GOLD);

	if(m_pText[iIndex])
		m_pText[iIndex]->SetString(szID);

	if(m_pText[iIndex + TRADE_BBS_MAX_LINE])
		m_pText[iIndex + TRADE_BBS_MAX_LINE]->SetString(szTitle);

	if (m_pText[iIndex + TRADE_BBS_MAX_LINE * 2] != nullptr)
	{
		std::string buff = fmt::format("{} {}", iPrice, szGold);
		m_pText[iIndex + TRADE_BBS_MAX_LINE * 2]->SetString(buff);
	}
}
