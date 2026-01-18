// UIMessageWnd.cpp: implementation of the CUIMessageWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIMessageWnd.h"
#include "GameProcMain.h"
#include "PacketDef.h"
#include "UIManager.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIScrollBar.h>

CUIMessageWnd::CUIMessageWnd()
{
	m_pChatOut        = nullptr;
	m_pScrollbar      = nullptr;
	m_pBtn_Fold       = nullptr;
	m_iChatLineCount  = 0;
	m_ppUILines       = nullptr;
	m_rcChatOutRegion = {};
}

CUIMessageWnd::~CUIMessageWnd()
{
	if (m_ppUILines)
	{
		delete[] m_ppUILines;
		m_ppUILines = nullptr;
	} // m_ppUILines[n]의 포인터는 메모리 할당되어 있어도 부모가 해제될때 자동으로 해제하므로 안지워야 한다.

	ChatListItor itor;
	for (itor = m_ChatBuffer.begin(); m_ChatBuffer.end() != itor; ++itor)
	{
		__ChatInfo* pChatInfo = (*itor);
		if (pChatInfo)
			delete pChatInfo;
	}
	m_ChatBuffer.clear();

	for (itor = m_LineBuffer.begin(); m_LineBuffer.end() != itor; ++itor)
	{
		__ChatInfo* pChatInfo = (*itor);
		if (pChatInfo)
			delete pChatInfo;
	}
	m_LineBuffer.clear();
}

void CUIMessageWnd::Release()
{
	CN3UIBase::Release();

	m_pChatOut       = nullptr;
	m_pScrollbar     = nullptr;
	m_pBtn_Fold      = nullptr;
	m_iChatLineCount = 0;
	if (m_ppUILines)
	{
		delete[] m_ppUILines;
		m_ppUILines = nullptr;
	} // m_ppUILines[n]의 포인터는 메모리 할당되어 있어도 부모가 해제될때 자동으로 해제하므로 안지워야 한다.
	ZeroMemory(&m_rcChatOutRegion, sizeof(m_rcChatOutRegion));

	ChatListItor itor;
	for (itor = m_ChatBuffer.begin(); m_ChatBuffer.end() != itor; ++itor)
	{
		__ChatInfo* pChatInfo = (*itor);
		if (pChatInfo)
			delete pChatInfo;
	}
	m_ChatBuffer.clear();

	for (itor = m_LineBuffer.begin(); m_LineBuffer.end() != itor; ++itor)
	{
		__ChatInfo* pChatInfo = (*itor);
		if (pChatInfo)
			delete pChatInfo;
	}
	m_LineBuffer.clear();
}

BOOL CUIMessageWnd::MoveOffset(int iOffsetX, int iOffsetY)
{
	if (0 == iOffsetX && 0 == iOffsetY)
		return FALSE;
	// ui 영역
	m_rcRegion.left    += iOffsetX;
	m_rcRegion.top     += iOffsetY;
	m_rcRegion.right   += iOffsetX;
	m_rcRegion.bottom  += iOffsetY;

	// movable 영역
	m_rcMovable.left   += iOffsetX;
	m_rcMovable.top    += iOffsetY;
	m_rcMovable.right  += iOffsetX;
	m_rcMovable.bottom += iOffsetY;

	// children 좌표 갱신
	CN3UIBase* pCUI     = nullptr; // Child UI...
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		pCUI = (*itor);
		__ASSERT(pCUI, "child UI pointer is NULL!");
		pCUI->MoveOffset(iOffsetX, iOffsetY);
	}

	//여기에 채팅창 옮기는 것도 넣어라...
	/*RECT rt = CGameProcedure::s_pProcMain->m_pUIChatDlg->GetRegion();
	POINT pt = this->GetPos();
	if( (pt.x != rt.right) || ( pt.y != rt.top) )
	{
	CGameProcedure::s_pProcMain->m_pUIChatDlg->SetPos(pt.x - (rt.right-rt.left), pt.y);

	}*/

	//NOTE: (madpew) Don't stick them together, so chat and info can move freely, but sync with the folded version instead
	POINT pt = this->GetPos();
	RECT rc  = this->GetRegion();
	RECT rc2 = CGameProcedure::s_pProcMain->m_pUIMsgDlg2->GetRegion();
	CGameProcedure::s_pProcMain->m_pUIMsgDlg2->SetPos(pt.x, rc.bottom + (rc2.top - rc2.bottom));

	return true;
}

bool CUIMessageWnd::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;
	N3_VERIFY_UI_COMPONENT(m_pChatOut, GetChildByID<CN3UIString>("text_message"));
	N3_VERIFY_UI_COMPONENT(m_pScrollbar, GetChildByID<CN3UIScrollBar>("scroll"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Fold, GetChildByID("btn_off"));

	m_rcChatOutRegion = m_pChatOut->GetRegion();
	CreateLines();

	__ASSERT(0 < m_iChatLineCount, "채팅창이 너무 작아요");

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////

bool CUIMessageWnd::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		// 스크롤바에 맞는 채팅 Line 설정
		int iCurLinePos = m_pScrollbar->GetCurrentPos();
		SetTopLine(iCurLinePos);
		return true;
	}
	else if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Fold)
		{
			CGameProcedure::s_pProcMain->CommandToggleUIMsgWnd();
			return true;
		}
	}

	return false;
}

void CUIMessageWnd::CreateLines()
{
	if (m_ppUILines != nullptr)
	{
		for (int i = 0; i < m_iChatLineCount; ++i)
		{
			delete m_ppUILines[i];
			m_ppUILines[i] = nullptr;
		}

		delete[] m_ppUILines;
		m_ppUILines = nullptr;
	}

	SIZE size {};
	if (m_pChatOut != nullptr && m_pChatOut->GetTextExtent("가", lstrlen("가"), &size) && size.cy > 0)
		m_iChatLineCount = (m_rcChatOutRegion.bottom - m_rcChatOutRegion.top) / size.cy;
	else
		return;

	const std::string& szFontName = m_pChatOut->GetFontName();
	uint32_t dwFontHeight         = m_pChatOut->GetFontHeight();
	uint32_t dwFontFlag           = m_pChatOut->GetFontFlags();

	if (m_iChatLineCount <= 0 || szFontName.size() <= 0)
		return;

	m_ppUILines = new CN3UIString*[m_iChatLineCount];
	for (int i = 0; i < m_iChatLineCount; ++i)
	{
		RECT rc;
		SetRect(&rc, m_rcChatOutRegion.left, m_rcChatOutRegion.top + (i * size.cy), m_rcChatOutRegion.right,
			m_rcChatOutRegion.top + ((i + 1) * size.cy));
		m_ppUILines[i] = new CN3UIString();
		m_ppUILines[i]->Init(this);
		m_ppUILines[i]->SetRegion(rc);
		m_ppUILines[i]->SetStyle(UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNLEFT | UISTYLE_STRING_ALIGNTOP);
		m_ppUILines[i]->SetFont(szFontName, dwFontHeight, dwFontFlag & D3DFONT_BOLD, dwFontFlag & D3DFONT_ITALIC);
	}
}

void CUIMessageWnd::AddMsg(const std::string& szString, D3DCOLOR color)
{
	const int iStrLen = static_cast<int>(szString.size());
	__ASSERT(m_pScrollbar, "");
	if (0 >= iStrLen)
		return;

	// ChatBuffer에 넣기
	__ChatInfo* pChatInfo = new __ChatInfo;
	pChatInfo->szChat     = szString;
	pChatInfo->color      = color;
	m_ChatBuffer.push_back(pChatInfo);

	if (m_ChatBuffer.size() > 255) // 255개가 넘으면 앞에서부터 지우기
	{
		__ChatInfo* pTemp = m_ChatBuffer.front();
		if (pTemp)
			delete pTemp;

		m_ChatBuffer.pop_front();
	}

	// line buffer 에 넣기
	AddLineBuffer(szString, color);

	// Line buffer 갯수 조절
	int iCurLinePos  = m_pScrollbar->GetCurrentPos(); // 현재 scroll bar가 가리키고 있는 line
	BOOL bAutoScroll = (m_pScrollbar->GetMaxPos() == iCurLinePos) ? TRUE : FALSE;

	while (
		m_LineBuffer.size() > MAX_CHAT_LINES
		&& 0 < iCurLinePos) // MAX_CHAT_LINES은 최대 line의 수 (단 스크롤바가 0인 곳에 있으면 line을 지우지 않으므로 500개를 넘길 수 있다)
	{
		// 한줄 지우기
		__ChatInfo* pTemp = m_LineBuffer.front();
		if (pTemp)
			delete pTemp;
		m_LineBuffer.pop_front();
		--iCurLinePos;
	}

	int iLineBufferSize = static_cast<int>(m_LineBuffer.size());
	int iMaxScrollPos   = iLineBufferSize - m_iChatLineCount;
	if (iMaxScrollPos < 0)
		iMaxScrollPos = 0;
	m_pScrollbar->SetRange(0, iMaxScrollPos); // scroll bar range 설정

	// 자동으로 스크롤이면
	if (bAutoScroll)
		iCurLinePos = iMaxScrollPos;
	if (iCurLinePos < 0)
		iCurLinePos = 0;

	// 스크롤바 현재 위치 재설정
	m_pScrollbar->SetCurrentPos(iCurLinePos);

	// 스크롤바에 맞는 채팅 Line 설정
	SetTopLine(iCurLinePos);
}

void CUIMessageWnd::AddLineBuffer(const std::string& szString, D3DCOLOR color)
{
	if (szString.empty())
		return;

	__ASSERT(m_pChatOut, "");

	const int iStrLen = static_cast<int>(szString.size());

	// line buffer 넣기
	SIZE size;
	if (!m_pChatOut->GetTextExtent(szString, iStrLen, &size))
	{
		__ASSERT(0, "no device context");
		return;
	}

	const int iRegionWidth = m_rcChatOutRegion.right - m_rcChatOutRegion.left;

	// 글자 자르는 코드, 영역 밖으로 벗어나는 글자는 자르고 밑에 줄에..
	int iCX                = 0;
	int iCount             = 0;
	int iLineStart         = 0;

	while (iCount < iStrLen)
	{
		if ('\n' == szString[iCount]) // \n
		{
			__ChatInfo* pLineInfo = new __ChatInfo;
			m_LineBuffer.push_back(pLineInfo);

			pLineInfo->color = color;
			if ((iCount - iLineStart) > 0)
			{
				int iLineLength   = iCount - iLineStart + 1;
				pLineInfo->szChat = szString.substr(iLineStart, iLineLength);
			} // 연속된 \n일 경우 pszLine = nullptr이 될 수 있다.

			++iCount;
			iLineStart = iCount;
			iCX        = 0;
		}
		else
		{
			int iCC = 0;
			if (0x80 & szString[iCount])
				iCC = 2; // 2BYTE 문자
			else
				iCC = 1; // 1BYTE 문자

			BOOL bFlag = m_pChatOut->GetTextExtent(&(szString[iCount]), iCC, &size);
			__ASSERT(bFlag, "cannot get size of dfont");
			if ((iCX + size.cx) > iRegionWidth) // 가로 길이가 넘었으면
			{
				// 한 라인 더 추가하기

				int iLineLength = iCount - iLineStart;
				if (iLineLength > 0)
				{
					__ChatInfo* pLineInfo = new __ChatInfo;
					m_LineBuffer.push_back(pLineInfo);

					pLineInfo->color  = color;
					pLineInfo->szChat = szString.substr(iLineStart, iLineLength);
				}
				else
				{
					__ASSERT(iRegionWidth > 15, "너무 좁아서 한글자도 찍을 수가 없다");
					break;
				}
				iLineStart = iCount;
				iCX        = 0;
			}
			// 글자 더하기
			iCount += iCC;
			iCX    += size.cx;
		}
	}

	// 맨 마지막 출 처리
	int iLineLength = iStrLen - iLineStart;
	if (iLineLength > 0)
	{
		__ChatInfo* pLineInfo = new __ChatInfo;
		m_LineBuffer.push_back(pLineInfo);

		pLineInfo->color  = color;
		pLineInfo->szChat = szString.substr(iLineStart, iLineLength);
	}
}

void CUIMessageWnd::SetTopLine(int iTopLine)
{
	if (m_iChatLineCount <= 0)
		return;

	const int iLineBufferSize = static_cast<int>(m_LineBuffer.size());
	if (iTopLine < 0)
		iTopLine = 0;
	else if (iTopLine > iLineBufferSize)
		iTopLine = iLineBufferSize;

	// 앞줄서부터 차례로 임시버퍼에 저장하고 string 길이 측정
	__ChatInfo** ppLineInfos = new __ChatInfo* [m_iChatLineCount] {};

	int i = 0, iCurLine = 0;
	for (; i < m_iChatLineCount; ++i)
	{
		iCurLine = iTopLine + i;
		if (iLineBufferSize <= iCurLine)
			break;

		ppLineInfos[i] = m_LineBuffer[iCurLine];
	}

	__ASSERT(m_ppUILines, "null pointer");
	// 앞에서부터 맞게 차례로 각각 버퍼에 넣기
	int iRealLine      = i; // 실제 출력되는 줄 수
	int iRealLineCount = 0;
	for (i = 0; i < iRealLine; ++i)
	{
		++iRealLineCount;
		if (m_ppUILines[i] == nullptr)
			continue;

		m_ppUILines[i]->SetColor(ppLineInfos[i]->color);
		m_ppUILines[i]->SetString(ppLineInfos[i]->szChat);
	}

	for (i = iRealLineCount; i < m_iChatLineCount; ++i)
	{
		if (m_ppUILines[i] != nullptr)
			m_ppUILines[i]->SetString(""); // 나머지는 빈칸 만들기
	}
	delete[] ppLineInfos;
}

void CUIMessageWnd::RecalcLineBuffer() // 채팅창 사이즈가 변했을때 호출해주면 line buffer를 다시 계산해서 넣어준다.
{
	// line buffer 초기화하기
	ChatListItor itor;
	for (itor = m_LineBuffer.begin(); m_LineBuffer.end() != itor; ++itor)
	{
		delete *itor;
	}
	m_LineBuffer.clear();

	// Line buffer 다시 넣기
	for (itor = m_ChatBuffer.begin(); m_ChatBuffer.end() != itor; ++itor)
	{
		__ChatInfo* pChatBuff = (*itor);
		if (pChatBuff)
			AddLineBuffer(pChatBuff->szChat, pChatBuff->color);
	}

	// Line buffer 갯수 조절
	while (m_LineBuffer.size() > MAX_CHAT_LINES) // MAX_CHAT_LINES은 최대 line의 수
	{
		// 한줄 지우기
		delete m_LineBuffer.front();
		m_LineBuffer.pop_front();
	}

	int iLineBufferSize = static_cast<int>(m_LineBuffer.size());
	int iMaxScrollPos   = iLineBufferSize - m_iChatLineCount;
	if (iMaxScrollPos < 0)
		iMaxScrollPos = 0;

	m_pScrollbar->SetRange(0, iMaxScrollPos); // scroll bar range 설정

	// 스크롤바 현재 위치 재설정
	m_pScrollbar->SetCurrentPos(iMaxScrollPos);

	// 스크롤바에 맞는 채팅 Line 설정
	SetTopLine(iMaxScrollPos);
}

void CUIMessageWnd::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	// 자식들을 적당히 배치한다.
	// m_rcChatOutRegion = ;	// 채팅 출력 영역을 다시 지정해준다.
	//CreateLines();	// 채팅 라인을 몇줄 들어갈지 계산하고 다시 만든다.
	//RecalcLineBuffer();	// 라인 버퍼를 다 지우고 다시 만들어주고 글씨를 표시한다.
}

bool CUIMessageWnd::OnKeyPress(int iKey)
{
	// hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
	if (iKey == DIK_ESCAPE)
	{
		// 열려있는 다른 유아이를 닫아준다.
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
		CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
		if (pFocus != nullptr && pFocus != this)
			pFocus->OnKeyPress(iKey);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

// UI MESSAGE WND 2 (FOLDED)

CUIMessageWnd2::CUIMessageWnd2()
{
	m_pBtn_Fold = nullptr;
}

bool CUIMessageWnd2::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;
	N3_VERIFY_UI_COMPONENT(m_pBtn_Fold, GetChildByID("btn_on"));
	return true;
}

bool CUIMessageWnd2::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Fold)
		{
			CGameProcedure::s_pProcMain->CommandToggleUIMsgWnd();
			return true;
		}
	}

	return false;
}

void CUIMessageWnd2::Release()
{
	CN3UIBase::Release();
	m_pBtn_Fold = nullptr;
}
