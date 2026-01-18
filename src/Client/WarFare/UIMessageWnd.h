// UIMessageWnd.h: interface for the CUIMessageWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMESSAGEWND_H__F825624B_A51E_4889_9ADC_BEBF022D010B__INCLUDED_)
#define AFX_UIMESSAGEWND_H__F825624B_A51E_4889_9ADC_BEBF022D010B__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>
#include "UIChat.h"

class CUIMessageWnd : public CN3UIBase
{
protected:
	CN3UIString* m_pChatOut;       // 채팅이 출력되는 UIString 참조포인터(실제 m_Child로 관리), 글씨체와 초기 영역만 참조한다.
	CN3UIScrollBar* m_pScrollbar;  // scrollbar 참조포인터(실제 m_Child로 관리)
	CN3UIBase* m_pBtn_Fold;

	ChatList m_ChatBuffer;         // 채팅 packet기준으로 된 buffer
	ChatList m_LineBuffer;         // Line 기준으로 된 buffer

	int m_iChatLineCount;          // 채팅창에 출력되는 line의 수(채팅창 사이즈가 변했을때 다시 계산해주자.)
	RECT m_rcChatOutRegion;        // 채팅이 출력되는 영역
	CN3UIString** m_ppUILines;     // 채팅이 출력되는 UIString 배열포인터(채팅창 사이즈가 변하므로 배열도 변한다.

protected:
	void SetTopLine(int iTopLine); // 맨 윗줄을 지정해준다.
	void AddLineBuffer(const std::string& szString, D3DCOLOR color); // line 버퍼를 만들어준다.(너무 길면 알아서 2줄로 만들어준다.)
	void RecalcLineBuffer(); // 채팅창 사이즈가 변했을때 호출해주면 line buffer를 다시 계산해서 넣어준다.
	void CreateLines();

							 // Operations
public:
	bool OnKeyPress(int iKey) override;
	BOOL MoveOffset(int iOffsetX, int iOffsetY) override; // offsetY만큼 이동해준다.(region, children, move rect 이동)
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Release() override;
	bool Load(File& file) override;
	void SetRegion(
		const RECT& Rect) override; // 영역 지정(사이즈가 변할때 호출된다. 단순 이동은 호출되지 않는다.(단순이동은 MoveOffset이 호출))

	void AddMsg(const std::string& szString, D3DCOLOR color = 0xffffffff); // 채팅 메세지를 저장하고 알맞은 형태로 화면에 출력해준다.

	CUIMessageWnd();
	~CUIMessageWnd() override;
};

// TODO: Use the official name for this
class CUIMessageWnd2 : public CN3UIBase
{
protected:
	CN3UIBase* m_pBtn_Fold;

public:
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Release() override;
	bool Load(File& file) override;

	CUIMessageWnd2();
};
#endif // !defined(AFX_UIMESSAGEWND_H__F825624B_A51E_4889_9ADC_BEBF022D010B__INCLUDED_)
