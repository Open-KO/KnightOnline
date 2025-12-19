// UIChat.h: interface for the CUIChat class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICHAT_H__2CFECA0D_EA38_4900_86BB_BAFD4D5EE6F7__INCLUDED_)
#define AFX_UICHAT_H__2CFECA0D_EA38_4900_86BB_BAFD4D5EE6F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>
#include <deque>

//enum e_ChatBuffer { CHAT_BUFFER_NORMAL = 0, CHAT_BUFFER_PRIVATE, CHAT_BUFFER_PARTY, CHAT_BUFFER_KNIGHTS, CHAT_BUFFER_COUNT };

struct __ChatInfo
{
	std::string szChat; // 문자열
	D3DCOLOR	color; // 문자열 색깔

	__ChatInfo() {};
	__ChatInfo(const std::string& szChat_Arg, D3DCOLOR color_Arg) { szChat = szChat_Arg; color = color_Arg; }
};

typedef std::deque<__ChatInfo*>		ChatList;
typedef ChatList::iterator			ChatListItor;
typedef ChatList::reverse_iterator	ChatListReverseItor;
const int MAX_CHAT_LINES = 100;

class CUIChat : public CN3UIBase  
{
protected:
//	ChatList		m_ChatBuffers[CHAT_BUFFER_COUNT];		// 채팅 packet기준으로 된 buffer
//	ChatList		m_LineBuffers[CHAT_BUFFER_COUNT];		// Line 기준으로 된 buffer
	ChatList		m_ChatBuffer;		// 채팅 packet기준으로 된 buffer
	ChatList		m_LineBuffer;		// Line 기준으로 된 buffer
	ChatList		m_ContinueMsg;		// 지속적으로 공지해주는 메시지 buffer
	int				m_iCurContinueMsg;

	CN3UIString*	m_pNoticeTitle;		// 채팅창 맨윗줄에 표시될 공지...
	CN3UIString*	m_pChatOut;			// 채팅이 출력되는 UIString 참조포인터(실제 m_Child로 관리), 글씨체와 초기 영역만 참조한다.
	CN3UIScrollBar* m_pScrollbar;		// scrollbar 참조포인터(실제 m_Child로 관리)
	int				m_iChatLineCount;	// 채팅창에 출력되는 line의 수(채팅창 사이즈가 변했을때 다시 계산해주자.)
	RECT			m_rcChatOutRegion;	// 채팅이 출력되는 영역
	CN3UIString**	m_ppUILines;		// 채팅이 출력되는 UIString 배열포인터(채팅창 사이즈가 변하므로 배열도 변한다.

	CN3UIEdit*		m_pEdit;			//son, chat_in
	std::string		m_szString;			//son, chat_in

	CN3UIBase*		m_pBtn_Normal;
	CN3UIBase*		m_pBtn_Private;
	CN3UIBase*		m_pBtn_PartyOrForce;
	CN3UIBase*		m_pBtn_KnightsOrGuild;
	CN3UIBase*		m_pBtn_Shout;
	CN3UIBase*		m_pBtn_Check;
	CN3UIBase*		m_pBtn_Fold;

	enum e_ChatMode	m_eChatMode;

	bool			m_bChatNormal;
	bool			m_bChatPrivate;
	bool			m_bChatClan;
	bool			m_bChatShout;
	bool			m_bChatParty;

	bool			m_bKillFocus;
//	e_ChatBuffer	m_eChatBuffer; // 채팅 표시 모드 .. 버퍼가 나누어져있다..
	
/*
	ChatList		m_MsgBuffer;		// 채팅 packet기준으로 된 buffer
	ChatList		m_MsgLineBuffer;	// Line 기준으로 된 buffer

	CN3UIString*	m_pMsgOut;		// 채팅이 출력되는 UIString 참조포인터(실제 m_Child로 관리)
	CN3UIScrollBar* m_pMsgScrollbar;	// scrollbar 참조포인터(실제 m_Child로 관리)
	int				m_iMsgLineCount;	// 채팅창에 출력되는 line의 수(채팅창 사이즈가 변했을때 다시 계산해주자.)
*/

protected:
	void			SetTopLine(int iTopLine);		// 맨 윗줄을 지정해준다.
//	void			AddLineBuffer(e_ChatBuffer eCB, const std::string& szString, D3DCOLOR color);	// line 버퍼를 만들어준다.(너무 길면 알아서 2줄로 만들어준다.)
	void			AddLineBuffer(const std::string& szString, D3DCOLOR color);	// line 버퍼를 만들어준다.(너무 길면 알아서 2줄로 만들어준다.)
	void			RecalcLineBuffers();		// 채팅창 사이즈가 변했을때 호출해주면 line buffer를 다시 계산해서 넣어준다.
	void			CreateLines();

// Operations
public:
	void			SetNoticeTitle(const std::string& szString, D3DCOLOR color);
	void			ShowContinueMsg();
	void			DeleteContinueMsg();
	bool			OnKeyPress(int iKey);
	bool			GetEnableKillFocus() { return m_bKillFocus; }
	void			SetEnableKillFocus(bool bKillFocus) { m_bKillFocus = bKillFocus; }
	void			ChatListenEnable();
	void			ChangeChattingMode(e_ChatMode eCM);
	BOOL			MoveOffset(int iOffsetX, int iOffsetY) override;	// Offset만큼 이동해준다.(region, children, move rect 이동)
	bool			ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void			Release() override;
	bool			Load(File& file) override;
	void			SetRegion(const RECT& Rect) override;	// 영역 지정(사이즈가 변할때 호출된다. 단순 이동은 호출되지 않는다.(단순이동은 MoveOffset이 호출))

	void				SetString(const std::string& szChat);
	void				SetCaretPos(int iPos);
	const std::string&	GetString() const {return m_szString;}				//son, chat_in
	void				AddChatMsg(e_ChatMode eCM, const std::string& szString, D3DCOLOR color = 0xffffffff);		// 채팅 메세지를 저장하고 알맞은 형태로 화면에 출력해준다.
	void				AddContinueMsg(e_ChatMode eCM, const std::string& szString, D3DCOLOR color = 0xffffffff);
	void				AdjustScroll(); // 스크롤 위치등 조정..

	BOOL			IsChatMode();
	void			SetFocus();
	void			KillFocus();

	CUIChat();
	~CUIChat() override;
};

// TODO: Use official name for this
class CUIChat2 : public CN3UIBase
{
protected:
	CN3UIBase*		m_pBtn_Fold;

public:
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Release() override;
	bool Load(File& file) override;

	CUIChat2();
};

#endif // !defined(AFX_UICHAT_H__2CFECA0D_EA38_4900_86BB_BAFD4D5EE6F7__INCLUDED_)


