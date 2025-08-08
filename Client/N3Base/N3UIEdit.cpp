// N3UIEdit.cpp: implementation of the CN3UIEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIEdit.h"
#include "N3UIString.h"
#include "N3UIImage.h"
#include "DFont.h"
#include "N3UIStatic.h"

#include "N3SndMgr.h"
#include "N3SndObj.h"
#include <imm.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

const float CARET_FLICKERING_TIME = 0.4f;

//HWND CN3UIEdit::s_hWndParent = NULL;
HWND CN3UIEdit::s_hWndEdit = NULL;
HWND CN3UIEdit::s_hWndParent = NULL;
WNDPROC	CN3UIEdit::s_lpfnEditProc = NULL;
char CN3UIEdit::s_szBuffTmp[512] = "";

//////////////////////////////////////////////////////////////////////
// CN3UIEdit::CN3Caret
//////////////////////////////////////////////////////////////////////
CN3UIEdit::CN3Caret::CN3Caret()
{
	m_pVB[0].Set(0,0,UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xffff0000);
	m_pVB[1].Set(0,10,UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xffff0000);
	m_bVisible = FALSE;
	m_fFlickerTimePrev = CN3Base::TimeGet();	// 깜박이기 위한 시간..
	m_bFlickerStatus = true;

}
CN3UIEdit::CN3Caret::~CN3Caret()
{
}
void CN3UIEdit::CN3Caret::SetPos(int x, int y)
{
	m_pVB[0].x = (float)x; m_pVB[0].y = (float)y;
	m_pVB[1].x = (float)x; m_pVB[1].y = (float)y+m_iSize;
	this->InitFlckering();
}
void CN3UIEdit::CN3Caret::MoveOffset(int iOffsetX, int iOffsetY)
{
	m_pVB[0].x += iOffsetX; m_pVB[0].y += iOffsetY; 
	m_pVB[1].x = m_pVB[0].x; m_pVB[1].y = m_pVB[0].y+m_iSize;
	this->InitFlckering();
}
void CN3UIEdit::CN3Caret::SetSize(int iSize)
{
	m_iSize = iSize;
	m_pVB[1].y = m_pVB[0].y+m_iSize;
}
void CN3UIEdit::CN3Caret::SetColor(D3DCOLOR color)
{
	m_pVB[0].color = color;
	m_pVB[1].color = color;
}
void CN3UIEdit::CN3Caret::Render(LPDIRECT3DDEVICE9	lpD3DDev)
{
	if (FALSE == m_bVisible) return;

	// 깜박임 처리..
	float fTime = CN3Base::TimeGet();
	if(fTime - m_fFlickerTimePrev > CARET_FLICKERING_TIME)
	{
		m_bFlickerStatus = !m_bFlickerStatus;
		m_fFlickerTimePrev = fTime;
	}
	if(!m_bFlickerStatus) return;

	__ASSERT(lpD3DDev, "DIRECT3DDEVICE8 is null");
	lpD3DDev->SetTexture(0, NULL);
//	lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1 );
//	lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,  D3DTA_DIFFUSE );
	lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
	lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_pVB, sizeof(m_pVB[0]));
}
void CN3UIEdit::CN3Caret::InitFlckering()
{
	m_fFlickerTimePrev = CN3Base::TimeGet();	// 깜박이기 위한 시간..
	m_bFlickerStatus = true;
}

//////////////////////////////////////////////////////////////////////
// CN3UIEdit
//////////////////////////////////////////////////////////////////////

BOOL CN3UIEdit::CreateEditWindow(HWND hParent, RECT rect)
{
	if(NULL == hParent) return FALSE;
	if(s_hWndEdit) return FALSE;

	s_hWndParent = hParent;
	s_hWndEdit = CreateWindow("EDIT", "EditWindow", WS_CHILD|WS_TABSTOP|ES_LEFT|ES_WANTRETURN, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, hParent, NULL, NULL, NULL);
	s_lpfnEditProc = (WNDPROC)SetWindowLong(s_hWndEdit, GWL_WNDPROC, (uint32_t)(CN3UIEdit::EditWndProc));

	// Set the edit control's text size to the maximum.
	::SendMessage(s_hWndEdit, EM_LIMITTEXT, 0, 0);

	// Set the edit control's font
	HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	::SendMessage(s_hWndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	::SendMessage(s_hWndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

	// 배경 지우기...??
	HDC hDC = GetDC(s_hWndEdit);
	SetBkMode(hDC, TRANSPARENT);
	SetROP2(hDC, R2_XORPEN);
	ReleaseDC(s_hWndEdit, hDC);

	return TRUE;
}

LRESULT APIENTRY CN3UIEdit::EditWndProc(HWND hWnd, uint16_t Message, WPARAM wParam, LPARAM lParam)
{
   //
   // When the focus is in an edit control inside a dialog box, the
   //  default ENTER key action will not occur.
   //
    switch (Message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			if(s_pFocusedEdit && s_pFocusedEdit->GetParent())
			{
				s_pFocusedEdit->GetParent()->ReceiveMessage(s_pFocusedEdit, UIMSG_EDIT_RETURN);
			}
			return 0;
		}

		(CallWindowProc(s_lpfnEditProc, hWnd, Message, wParam, lParam));
		if(s_pFocusedEdit) CN3UIEdit::UpdateCaretPosFromEditCtrl();
		return 0;
		//break;

    case WM_CHAR:
		if(s_pFocusedEdit) CN3UIEdit::UpdateCaretPosFromEditCtrl();
		if(wParam==VK_RETURN) return 0;
		if(wParam==VK_TAB) return 0;
		break;
	case WM_INPUTLANGCHANGE:
		{
			POINT ptPos;
			ptPos.x = 0;
			ptPos.y = 0;
			SetImeStatus(ptPos,true);
		}
		break;
	case WM_IME_ENDCOMPOSITION:
		{
			POINT ptPos;
			ptPos.x = -1000;
			ptPos.y = -1000;
			SetImeStatus(ptPos,false);
		}
		break;
	case WM_IME_STARTCOMPOSITION:
		{
			POINT ptPos;
			ptPos.x = 0;
			ptPos.y = 0;
			SetImeStatus(ptPos,true);
		}
		break;
    } // switch









/*
	switch(Message)
	{
	case WM_IME_CHAR:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_COMPOSITION:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_COMPOSITIONFULL:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_CONTROL:
		{
			int iiii = 0;
			uint32_t dwCmd = wParam;
			switch(dwCmd)
			{
			case IMC_GETCANDIDATEPOS: 
				iiii = 0;
				break;
			case IMC_OPENSTATUSWINDOW:
				iiii = 0;
				break;
			case IMC_GETCOMPOSITIONFONT:
				iiii = 0;
				break;
			case IMC_SETCANDIDATEPOS :
				iiii = 0;
				break;
			case IMC_GETCOMPOSITIONWINDOW:
				iiii = 0;
				break;
			case IMC_SETCOMPOSITIONFONT :
				iiii = 0;
				break;
//			case IMC_GETCONVERSIONMODE:
//				iiii = 0;
//				break;
			case IMC_SETCOMPOSITIONWINDOW :
				iiii = 0;
				break;
//			case IMC_GETOPENSTATUS:
//				iiii = 0;
//				break;
//			case IMC_SETCONVERSIONMODE :
//				iiii = 0;
//				break;
//			case IMC_GETSENTENCEMODE:
//				iiii = 0;
//				break;
//			case IMC_SETOPENSTATUS :
//				iiii = 0;
//				break;
			case IMC_GETSTATUSWINDOWPOS:
				iiii = 0;
				break;
//			case IMC_SETSENTENCEMODE :
//				iiii = 0;
//				break;
			case IMC_CLOSESTATUSWINDOW:
				iiii = 0;
				break;
			case IMC_SETSTATUSWINDOWPOS :
				iiii = 0;
				break;
			}
		}
		break;
	case WM_IME_ENDCOMPOSITION:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_KEYDOWN:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_KEYUP:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_NOTIFY:
		{
			int iiii = 0;

			switch(wParam)
			{
			case IMN_CHANGECANDIDATE:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETCANDIDATEPOS:
				{
					int iiii = 0;
					break;
				}
			case IMN_CLOSECANDIDATE:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETCOMPOSITIONFONT:
				{
					int iiii = 0;
					break;
				}
			case IMN_CLOSESTATUSWINDOW:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETCOMPOSITIONWINDOW:
				{
					int iiii = 0;
					
					COMPOSITIONFORM CompForm;
					CompForm.dwStyle = CFS_RECT;
					int msg2 = (UINT) WM_IME_CONTROL;
					WPARAM wParam2 = (WPARAM) IMC_GETCOMPOSITIONWINDOW;
					WPARAM lParam2 = (LPARAM) &CompForm;
					SendMessage(hWnd, msg2, wParam2, lParam2);

					break;
				}
			case IMN_GUIDELINE:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETCONVERSIONMODE:
				{
					int iiii = 0;
					break;
				}
			case IMN_OPENCANDIDATE:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETOPENSTATUS:
				{
					int iiii = 0;
					break;
				}
			case IMN_OPENSTATUSWINDOW:
				{
					int iiii = 0;
					break;
				}
			case IMN_SETSENTENCEMODE:
				{
					int iiii = 0;
					break;
				}
			case IMN_PRIVATE:
				{
					int iiii = 0;

					CANDIDATEFORM cf;
					cf.dwStyle = CFS_RECT;
					iiii = ::SendMessage(hWnd, WM_IME_CONTROL, IMC_GETCANDIDATEPOS, (LPARAM)(&cf));

					LOGFONT lf;
					iiii = ::SendMessage(hWnd, WM_IME_CONTROL, IMC_GETCOMPOSITIONFONT, (LPARAM)(&lf));

					iiii = 0;



					break;
				}
			case IMN_SETSTATUSWINDOWPOS:
				{
					int iiii = 0;
					break;
				}
			}

//			CANDIDATEFORM form;
//			form.dwIndex = 0;
//			form.dwStyle = CFS_EXCLUDE;
//			this->SendMessage(0x0288, 0x0002, (LPARAM)(&form));
//			iiii = -1;
//			this->SendMessage(WM_IME_REQUEST, IMR_CANDIDATEWINDOW, &form);
		}
		break;
	case WM_IME_SELECT:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_SETCONTEXT:
		{
			int iiii = 0;
		}
		break;
	case WM_IME_STARTCOMPOSITION:
		{
			int iiii = 0;
		}
		break;
	}
*/
















	return (CallWindowProc(s_lpfnEditProc, hWnd, Message, wParam, lParam));
}

CN3UIEdit::CN3Caret CN3UIEdit::s_Caret;

CN3UIEdit::CN3UIEdit()
{
	m_eType = UI_TYPE_EDIT;
	m_nCaretPos = 0;
	m_iCompLength = 0;
	m_iMaxStrLen = 0x7fffffff;
	KillFocus();
	m_pSnd_Typing = NULL;
}

CN3UIEdit::~CN3UIEdit()
{
	KillFocus();
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Typing);
}

void CN3UIEdit::Release()
{
	CN3UIBase::Release();
	m_nCaretPos = 0;
	m_iCompLength = 0;
	m_iMaxStrLen = 0x7fffffff;
	KillFocus();
	m_szPassword = "";
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Typing);
}

void CN3UIEdit::Render()
{
	if (!m_bVisible) return;

	CN3UIStatic::Render();
	if (HaveFocus())
	{
		s_Caret.Render(s_lpD3DDev);	// 포커스가 있으면 캐럿 그리기
	}
}

void CN3UIEdit::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (false == bVisible && true == m_bVisible)	// 보이지 않게 할때
	{
		KillFocus();
	}
}

void CN3UIEdit::KillFocus()
{
	if (HaveFocus())
	{
		s_pFocusedEdit = NULL;
		s_Caret.m_bVisible = FALSE;

		if(s_hWndEdit)
		{
			::SetWindowText(s_hWndEdit, "");
			::SetFocus(s_hWndParent);
		}
	}
}

bool CN3UIEdit::SetFocus()
{
//	if (HaveFocus()) return true;		// 이미 내가 포커스를 가지고 있으면 return true;
	if (NULL != s_pFocusedEdit) s_pFocusedEdit->KillFocus();	// 다른 edit 가 가지고 있으면 killfocus호출
	s_pFocusedEdit = this;				// 포커스를 가지고 있는 edit를 나로 설정

	SIZE size;
	if (m_pBuffOutRef && m_pBuffOutRef->GetTextExtent("가",2,&size))
	{
		s_Caret.SetSize(size.cy);
		s_Caret.SetColor(m_pBuffOutRef->GetFontColor());
	}

	s_Caret.m_bVisible = TRUE;
	s_Caret.InitFlckering();
	CN3UIEdit::UpdateCaretPosFromEditCtrl(); // 캐럿 포지션 설정

	if(s_hWndEdit)
	{
		::SetFocus(s_hWndEdit);

		RECT rcEdit = GetRegion();
		int iX		= rcEdit.left;
		int iY		= rcEdit.top;
		int iH		= rcEdit.bottom - rcEdit.top;
		int iW		= rcEdit.right - rcEdit.left;
		::MoveWindow(s_hWndEdit, iX, iY, iW, iH, false);
		
		if(UISTYLE_EDIT_PASSWORD & m_dwStyle)
		{
			::SetWindowText(s_hWndEdit, m_szPassword.c_str());
		}
		else
		{
			if(m_pBuffOutRef) ::SetWindowText(s_hWndEdit, m_pBuffOutRef->GetString().c_str());
			else ::SetWindowText(s_hWndEdit, "");
		}
	}

	return true;
}

uint32_t CN3UIEdit::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if(dwFlags & UI_MOUSE_LBCLICK &&IsIn(ptCur.x, ptCur.y))	// 영역 안에서 왼쪽 버튼이 눌렸으면
	{
		SetFocus();	// 나에게 포커스를 준다.
		dwRet |= (UI_MOUSEPROC_DONESOMETHING|UI_MOUSEPROC_INREGION);
		return dwRet;
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CN3UIEdit::SetCaretPos(UINT nPos)
{
	if (nPos > m_iMaxStrLen) nPos = m_iMaxStrLen;	// 최대 길이보다 길경우 작게 세팅
	m_nCaretPos = nPos;

	const std::string& szBuff = m_pBuffOutRef->GetString();
	__ASSERT(szBuff.empty() || -1 == szBuff.find('\n'), "multiline edit");	// 지금은 multiline은 지원하지 않는다.
	SIZE size = {0,0};
	if (!szBuff.empty() && m_pBuffOutRef ) m_pBuffOutRef->GetTextExtent(szBuff, m_nCaretPos, &size) ;

	int iRegionWidth = m_rcRegion.right - m_rcRegion.left;
	if (size.cx > iRegionWidth) size.cx = iRegionWidth;
	s_Caret.SetPos(m_pBuffOutRef->m_ptDrawPos.x + size.cx, m_pBuffOutRef->m_ptDrawPos.y);
}

void CN3UIEdit::SetMaxString(size_t iMax)		// 최대 글씨 수를 정해준다
{
	if (iMax <= 0) {__ASSERT(0, "최대 글씨 수를 0보다 크게 정해주세요"); return;}
	m_iMaxStrLen = iMax;

	if (NULL == m_pBuffOutRef) return;

	const std::string szBuff = GetString();
	if ( m_iMaxStrLen >= szBuff.size()) return;

	// 여기까지 오는 경우는 현재 글씨길이가 iMax보다 큰 경우이므로 제한글자에 맞춰 잘라주게 다시 설정한다.
	SetString(szBuff);
}

/////////////////////////////////////////////////////////////////////
//
// 특정 위치가 한글의 2byte중에 두번째 바이트인지 검사하는 부분이다.
// IsDBCSLeadByte라는 함수가 있지만 조합형일 경우는
// 시작Byte와 끝byte의 범위가 같으로 이 함수로 검사 할 수 없다.
// 따라서 처음부터 검사를 하는 방법 외에는 다른 방법이 없다.
//
// NT의 Unicode에서는 어떻게 작용하는지 검사해 보지 않았지만
// 별 다른 문제 없이 사용할 수 있다고 생각한다.
//
/////////////////////////////////////////////////////////////////////
BOOL CN3UIEdit::IsHangulMiddleByte( const char* lpszStr, int iPos )
{
    if( !lpszStr ) return FALSE;
    if (iPos <= 0) return FALSE;
    int nLength = lstrlen(lpszStr);
    if (iPos >= nLength) return FALSE;
    if (!(lpszStr[iPos] & 0x80))
        return FALSE;

	BOOL bMiddle = FALSE;
	for (int i = 0; i < iPos && i < nLength; i++)
	{
		if (lpszStr[i] & 0x80)
			bMiddle = !bMiddle;
	}
	return bMiddle;
}

const std::string& CN3UIEdit::GetString()
{
	if (UISTYLE_EDIT_PASSWORD & m_dwStyle) return m_szPassword;
	return CN3UIStatic::GetString();
}

void CN3UIEdit::SetString(const std::string& szString)
{
	if (NULL == m_pBuffOutRef) return;
	if (szString.size() > m_iMaxStrLen)
	{
		std::string szNewBuff(m_iMaxStrLen, '?');

		if (IsHangulMiddleByte(szString.c_str(), m_iMaxStrLen))
		{
			szNewBuff = szString.substr(0, m_iMaxStrLen-1);	// -1은 한글이므로 하나 덜 카피하기 위해 +1은 맨 마지막에 NULL 넣기 위해
			if (UISTYLE_EDIT_PASSWORD & m_dwStyle)
			{
				int iNewBuffLen = szNewBuff.size();
				m_szPassword = szNewBuff;

				szNewBuff.assign(m_iMaxStrLen-1, '*');
				__ASSERT(NULL == szNewBuff[m_iMaxStrLen-1],"글자수가 다르다.");
			}
			m_pBuffOutRef->SetString(szNewBuff);
		}
		else
		{
			szNewBuff = szString.substr(0, m_iMaxStrLen);	// +1은 맨 마지막에 NULL 넣기 위해
			if (UISTYLE_EDIT_PASSWORD & m_dwStyle)
			{
				int iNewBuffLen = szNewBuff.size();
				m_szPassword = szNewBuff;

				szNewBuff.assign(m_iMaxStrLen, '*');
				__ASSERT(NULL == szNewBuff[m_iMaxStrLen],"글자수가 다르다.");
			}
			m_pBuffOutRef->SetString(szNewBuff);
		}
	}
	else
	{
		if (UISTYLE_EDIT_PASSWORD & m_dwStyle)
		{
			m_szPassword = szString;
			if (!szString.empty())
			{
				std::string szNewBuff(szString.size(), '*');
				m_pBuffOutRef->SetString(szNewBuff);
			}
			else m_pBuffOutRef->SetString("");
		}
		else
		{
			m_pBuffOutRef->SetString(szString);
		}
	}

	const std::string& szTempStr = m_pBuffOutRef->GetString();
	size_t iStrLen = szTempStr.size();
	if (m_nCaretPos > iStrLen) SetCaretPos(iStrLen);
}

BOOL CN3UIEdit::MoveOffset(int iOffsetX, int iOffsetY)		// 위치 지정(chilren의 위치도 같이 바꾸어준다. caret위치도 같이 바꾸어줌.)
{
	if (FALSE == CN3UIBase::MoveOffset(iOffsetX, iOffsetY)) return FALSE;
/*
	RECT rcEdit = GetRegion();
	int iX		= rcEdit.left;
	int iY		= rcEdit.top;
	int iH		= rcEdit.bottom - rcEdit.top;
	int iW		= rcEdit.right - rcEdit.left;

	::MoveWindow(s_hWndEdit, iX, iY, iW, iH, false);
*/
	if (HaveFocus()) s_Caret.MoveOffset(iOffsetX, iOffsetY);
	return TRUE;
}

bool CN3UIEdit::Load(HANDLE hFile)
{
	if (false == CN3UIStatic::Load(hFile)) return false;

	// 이전 uif파일을 컨버팅 하려면 사운드 로드 하는 부분 막기
	int iSndFNLen = 0;
	DWORD dwNum;

	ReadFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0)
	{
		std::vector<char> buffer(iSndFNLen+1, NULL);
		ReadFile(hFile, &buffer[0], iSndFNLen, &dwNum, NULL);

		__ASSERT(NULL == m_pSnd_Typing, "memory leak");
		m_pSnd_Typing = s_SndMgr.CreateObj(&buffer[0], SNDTYPE_2D);
	}

	return true;
}

#ifdef _N3TOOL
void CN3UIEdit::operator = (const CN3UIEdit& other)
{
	CN3UIStatic::operator = (other);
	SetSndTyping(other.GetSndFName_Typing());
}

bool CN3UIEdit::Save(HANDLE hFile)
{
	if (false == CN3UIStatic::Save(hFile)) return false;

	DWORD dwNum;

	int iSndFNLen = 0;
	if (m_pSnd_Typing) iSndFNLen = m_pSnd_Typing->m_szFileName.size();
	WriteFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0) WriteFile(hFile, m_pSnd_Typing->m_szFileName.c_str(), iSndFNLen, &dwNum, NULL);

	return true;
}

void CN3UIEdit::SetSndTyping(const std::string& strFileName)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Typing);
	if (0 == strFileName.size()) return;

	CN3BaseFileAccess tmpBase;
	tmpBase.FileNameSet(strFileName);	// Base경로에 대해서 상대적 경로를 넘겨준다.

	SetCurrentDirectory(tmpBase.PathGet().c_str());
	m_pSnd_Typing = s_SndMgr.CreateObj(tmpBase.FileName(), SNDTYPE_2D);
}

std::string CN3UIEdit::GetSndFName_Typing() const
{
	if (m_pSnd_Typing) return m_pSnd_Typing->m_szFileName;
	else return std::string("");
}
#endif

void CN3UIEdit::UpdateTextFromEditCtrl()
{
	if(NULL == s_pFocusedEdit || NULL == s_hWndEdit) return;

	::GetWindowText(s_hWndEdit, s_szBuffTmp, 512);
	s_pFocusedEdit->SetString(s_szBuffTmp);
}

void CN3UIEdit::UpdateCaretPosFromEditCtrl()
{
	if(NULL == s_pFocusedEdit || NULL == s_hWndEdit) return;

/*	int iCaret = 0;
	int iLen = GetWindowTextLength(s_hWndEdit);
	POINT ptCaret;
	GetCaretPos(&ptCaret);
	if(ptCaret.x > 0)
	{
		HDC hDC = GetDC(s_hWndEdit);
		SIZE size;
		GetTextExtentPoint32(hDC, "1", 1, &size);
		iCaret = ptCaret.x / size.cx;
		ReleaseDC(s_hWndEdit, hDC);
	}
*/
	int iTmp = ::SendMessage(s_hWndEdit, EM_GETSEL, 0, 0);
	int iCaret = LOWORD(iTmp);
	int iCTmp2 = HIWORD(iTmp);
	s_pFocusedEdit->SetCaretPos(iCaret);
}

void CN3UIEdit::SetImeStatus(POINT ptPos, bool bOpen)
{
#ifndef _N3TOOL
	HKL hHKL = GetKeyboardLayout(0);
	if(ImmIsIME(hHKL))
	{
		HIMC hImc = ImmGetContext(s_hWndEdit);
		if(bOpen)
		{
			SendMessage(s_hWndEdit, WM_IME_NOTIFY, IMN_OPENSTATUSWINDOW, 0);
			ImmSetStatusWindowPos(hImc,&ptPos);
		}
		else
		{
			SendMessage(s_hWndEdit, WM_IME_NOTIFY, IMN_CLOSESTATUSWINDOW, 0);
		}
		ImmReleaseContext( s_hWndEdit, hImc );
	}
#endif
}

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////
//	IME 관련해서
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include<map>
typedef std::map<HWND, CN3UIEdit*>::iterator		it_Edit;
typedef std::map<HWND, CN3UIEdit*>::value_type		val_Edit;
static std::map<HWND, CN3UIEdit*> s_Edits;

bool CN3UIEdit::AddEdit(CN3UIEdit* pEdit)
{
	if(pEdit == NULL)
	{
		__ASSERT(0, "NULL POINTER");
		return false;
	}

	it_Edit it = s_Edits.find(pEdit->m_hWndEdit);
	if(it == s_Edits.end()) // 중복된게 없으면..
	{
		s_Edits.insert(val_Edit(pEdit->m_hWndEdit, pEdit));
		return true;
	}
	else // 중복되었으면..
	{
		__ASSERT(0, "Edit Handle Duplicate");
		return false;
	}
}

bool CN3UIEdit::DeleteEdit(CN3UIEdit* pEdit)
{
	if(pEdit == NULL)
	{
		__ASSERT(0, "NULL POINTER");
		return false;
	}

	it_Edit it = s_Edits.find(pEdit->m_hWndEdit);
	if(it == s_Edits.end()) return false;

	s_Edits.erase(it);
	return true;
}

CN3UIEdit* CN3UIEdit::GetEdit(HWND hWnd)
{
	it_Edit it = s_Edits.find(hWnd);
	if(it == s_Edits.end()) return NULL;

	return it->second;
}

LRESULT APIENTRY CN3UIEdit::EditWndProc(HWND hWnd, uint16_t Message, WPARAM wParam, LPARAM lParam)
{
   //
   // When the focus is in an edit control inside a dialog box, the
   //  default ENTER key action will not occur.
   //
	CN3UIEdit* pEdit = CN3UIEdit::GetEdit(hWnd);
	if(pEdit) pEdit->EditWndProcFuncPointer(hWnd, Message, wParam, lParam);

	return 0;
}

LRESULT APIENTRY CN3UIEdit::EditWndProcFuncPointer(HWND hWnd, uint16_t Message, WPARAM wParam, LPARAM lParam)
{
   //
   // When the focus is in an edit control inside a dialog box, the
   //  default ENTER key action will not occur.
   //
    switch (Message)
	{
	case WM_CREATE:
		::SetWindowText(m_hWndEdit,"");
		break;
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			if(GetParent())
			{
				GetParent()->ReceiveMessage(this, UIMSG_EDIT_RETURN);
			}
			return 0;
		}
		(CallWindowProc(m_lpfnEditProc, hWnd, Message, wParam, lParam));
		UpdateCaretPosFromEditCtrl();
		return 0;
		//break;

    case WM_CHAR:
		CN3UIEdit::UpdateCaretPosFromEditCtrl();
		if(wParam==VK_RETURN) return 0;
		if(wParam==VK_TAB) return 0;
		break;
    } // switch
	return (CallWindowProc(m_lpfnEditProc, hWnd, Message, wParam, lParam));
}
*/

