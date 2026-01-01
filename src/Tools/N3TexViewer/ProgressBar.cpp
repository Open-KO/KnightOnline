// ProgressBar.cpp : implementation file
//

#include "stdafx.h"
#include "N3TexViewer.h"

#include "ProgressBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressBar

CProgressBar::CProgressBar()
{
	m_nSize = 0;
}

CProgressBar::~CProgressBar()
{
	Clear();
}


BEGIN_MESSAGE_MAP(CProgressBar, CProgressCtrl)
	//{{AFX_MSG_MAP(CProgressBar)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressBar message handlers

BOOL CProgressBar::Create(LPCTSTR strMessage, int nSize, int MaxValue)
{
	// 상태바를 얻음
	CStatusBar * pStatusBar = GetStatusBar();
	if (pStatusBar == nullptr) return FALSE;

	// 상태바 위에 프로그레스 컨트롤생성
	if(!CProgressCtrl::Create(WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), pStatusBar, 1)) return FALSE;

	// 프로그레스 컨트롤의 범위와 스텝 설정
	SetRange(0, MaxValue);
	SetStep(1);

	m_strMessage = strMessage;
	m_nSize = nSize;

	// 위치와 크기 조정
	Resize();
	return TRUE;
}

CStatusBar* CProgressBar::GetStatusBar()
{
	CFrameWnd* pFrame = (CFrameWnd*)AfxGetMainWnd();
	if (pFrame == nullptr || pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) == FALSE) return nullptr;

	CStatusBar* pBar = (CStatusBar*)pFrame->GetMessageBar();
	if(pBar == nullptr || pBar->IsKindOf(RUNTIME_CLASS(CStatusBar)) == FALSE) return nullptr;

	return pBar;
}

void CProgressBar::Resize()
{
	CStatusBar* pStatusBar = GetStatusBar();
	if (pStatusBar == nullptr) return;

	// 텍스트 출력
	if (::IsWindow(m_hWnd) && IsWindowVisible())
	{
		pStatusBar->SetWindowText(m_strMessage);
		pStatusBar->UpdateWindow();
	}

	// 텍스트가 차지하는 영역 계산
	CClientDC dc(pStatusBar);
	CFont* pOldFont = dc.SelectObject(pStatusBar->GetFont());
	CSize size = dc.GetTextExtent(m_strMessage);
	int margin = dc.GetTextExtent(_T(" ")).cx*2;
	dc.SelectObject(pOldFont);

	// 프로그레스 컨트롤이 그려질 영역 계산
	CRect rc;
	pStatusBar->GetItemRect(0, rc);
	rc.left = size.cx + 2*margin;
	rc.right = rc.left + (rc.right-rc.left)*m_nSize/100;
	if (rc.right < rc.left) rc.right = rc.left;

	// 상태바 상하로 10%의 여백을 둠
	int Height = rc.bottom - rc.top;
	rc.bottom -= Height/10;
	rc.top += Height/10;

	// 프로그레스 컨트롤의 위치와 크기를 재조정
	if (::IsWindow(m_hWnd) && (rc != m_Rect)) MoveWindow(&rc);
	m_Rect = rc;
}

void CProgressBar::Clear()
{
	ModifyStyle(WS_VISIBLE, 0);
	CString str;
	str.LoadString(AFX_IDS_IDLEMESSAGE);
	CStatusBar* pStatusBar = GetStatusBar();
	if(pStatusBar) pStatusBar->SetWindowText(str);
}


BOOL CProgressBar::OnEraseBkgnd(CDC* pDC) 
{
	Resize();
	
	return CProgressCtrl::OnEraseBkgnd(pDC);
}
