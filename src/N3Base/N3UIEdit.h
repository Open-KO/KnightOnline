// N3UIEdit.h: interface for the CN3UIEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIEDIT_H__91BCC181_3AA5_4CD4_8D33_06D5D96F4F26__INCLUDED_)
#define AFX_N3UIEDIT_H__91BCC181_3AA5_4CD4_8D33_06D5D96F4F26__INCLUDED_

#pragma once

#include "N3UIStatic.h"

class CN3UIEdit : public CN3UIStatic
{
	static constexpr uint32_t DEFAULT_MAX_LENGTH = 0x7fffffff;

	//	friend class CN3IME;
public:
	CN3UIEdit();
	~CN3UIEdit() override;

	// class
protected:
	class CN3Caret
	{
	public:
		CN3Caret();
		virtual ~CN3Caret();
		void SetPos(int x, int y);
		void MoveOffset(int iOffsetX, int iOffsetY);
		void SetSize(int iSize);
		void SetColor(D3DCOLOR color);
		void Render(LPDIRECT3DDEVICE9 lpD3DDev);
		void InitFlckering();     // 깜박임 초기화..
		BOOL m_bVisible;          // 보이나

	protected:
		int m_iSize;              // caret의 pixel 크기
		float m_fFlickerTimePrev; // 깜박이기 위한 시간..
		bool m_bFlickerStatus;
		//		POINT	m_ptPos;				// caret의 pixel 좌표
		__VertexTransformedColor m_pVB[2]; // vertex 버퍼
	};

										   // Attributes
public:
	static HWND s_hWndEdit, s_hWndParent;
	static WNDPROC s_lpfnEditProc;
	static char s_szBuffTmp[512];

	static void SetImeStatus(POINT ptPos, bool bOpen);
	static BOOL CreateEditWindow(HWND hParent, RECT rect);
	static LRESULT APIENTRY EditWndProc(HWND hWnd, uint16_t Message, WPARAM wParam, LPARAM lParam);
	static void UpdateTextFromEditCtrl();
	static void UpdateCaretPosFromEditCtrl();

protected:
	static CN3Caret s_Caret;
	size_t m_nCaretPos;       // 글자 단위위치(byte단위)
	int m_iCompLength;        // 현재 조합중인 글자의 byte수 0이면 조합중이 아니다.
	size_t m_iMaxStrLen;      // 쓸수 있는 글씨의 최대 숫자
	std::string m_szPassword; // password buffer

	CN3SndObj* m_pSnd_Typing; // 타이핑 할 때 나는 소리
							  // Operations
public:
	const std::string& GetString() override;
	void SetString(const std::string& szString) override;

	bool Load(File& file) override;
	void Render() override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;

	// 위치 지정(chilren의 위치도 같이 바꾸어준다. caret위치도 같이 바꾸어줌.)
	BOOL MoveOffset(int iOffsetX, int iOffsetY) override;

	void KillFocus(); // 포커스를 없앤다.
	bool SetFocus();  // 포커스를 준다.

	bool HaveFocus() const
	{
		return (this == s_pFocusedEdit);
	}

	void SetCaretPos(size_t nPos);                          //몇번째 바이트에 있는지 설정한다.
	void SetMaxString(size_t nMax);                         // 최대 글씨 수를 정해준다.

protected:
	BOOL IsHangulMiddleByte(const char* lpszStr, int iPos); // 한글의 2번째 바이트 글자인가?

#ifdef _N3TOOL
public:
	CN3UIEdit& operator=(const CN3UIEdit& other);
	bool Save(File& file) override;
	void SetSndTyping(const std::string& strFileName);
	std::string GetSndFName_Typing() const;
#endif
};

#endif // !defined(AFX_N3UIEDIT_H__91BCC181_3AA5_4CD4_8D33_06D5D96F4F26__INCLUDED_)
