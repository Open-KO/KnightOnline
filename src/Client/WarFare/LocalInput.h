#ifndef _LocalInput_H_
#define _LocalInput_H_

#pragma once

#include <dinput.h>

#include <N3Base/My_3DStruct.h>

inline constexpr int DK_NONE         = 0;
inline constexpr int DK_RELEASE      = 1;
inline constexpr int DK_PRESS        = 2;
inline constexpr int DK_REPEAT       = 4;
inline constexpr int NUMDIKEYS       = 256;

// 마우스 플래그 - 한개 이상의 플래그가 OR 연산으로 조합되어 있다..
inline constexpr int MOUSE_LBCLICK   = 0x1;
inline constexpr int MOUSE_LBCLICKED = 0x2;
inline constexpr int MOUSE_LBDOWN    = 0x4;
inline constexpr int MOUSE_MBCLICK   = 0x8;
inline constexpr int MOUSE_MBCLICKED = 0x10;
inline constexpr int MOUSE_MBDOWN    = 0x20;
inline constexpr int MOUSE_RBCLICK   = 0x40;
inline constexpr int MOUSE_RBCLICKED = 0x80;
inline constexpr int MOUSE_RBDOWN    = 0x100;
inline constexpr int MOUSE_LBDBLCLK  = 0x200;
inline constexpr int MOUSE_MBDBLCLK  = 0x400;
inline constexpr int MOUSE_RBDBLCLK  = 0x800;

//////////////////////////////////////////////////////////////////////////////////
// CLocalInput is a class wrapper for DirectInput and contains functions to receive
// data from the mouse, keyboard
//////////////////////////////////////////////////////////////////////////////////
class CLocalInput
{
private:
	void AcquireKeyboard();
	void UnacquireKeyboard();

protected:
	LPDIRECTINPUT8 m_lpDI;
	LPDIRECTINPUTDEVICE8 m_lpDIDKeyboard;

	HWND m_hWnd;

	int m_nMouseFlag, m_nMouseFlagOld; // 마우스 버튼 눌림 플래그
	uint32_t m_dwTickLBDown;           // 마우스 왼쪽 버튼 더블 클릭 감지용
	uint32_t m_dwTickRBDown;           // 마우스 오른쪽 버튼 더블 클릭 감지용

	POINT m_ptCurMouse;                // 현재 마우스 포인터
	POINT m_ptOldMouse;                // 직전 마우스 포인터

	RECT m_rcLBDrag;                   // 드래그 영역
	RECT m_rcMBDrag;                   // 드래그 영역
	RECT m_rcRBDrag;                   // 드래그 영역

	uint8_t m_byCurKeys[NUMDIKEYS];    // 현재 키 상태
	uint8_t m_byOldKeys[NUMDIKEYS];    // 직전 키 상태
	BOOL m_bKeyPresses[NUMDIKEYS];     // 키를 누른 순간인지
	BOOL m_bKeyPresseds[NUMDIKEYS];    // 키를 눌렀다 떼는 순간인지
	BOOL m_bNoKeyDown;                 // 아무 키입력도 없는지

	uint32_t m_dwTickKeyPress[NUMDIKEYS];

public:
	// 키보드 입력을 무효화 시킨다.. 기본값은 몽땅 무효화이다..
	void KeyboardClearInput(int iIndex = -1)
	{
		if (-1 == iIndex)
		{
			memset(m_byOldKeys, 0, sizeof(m_byOldKeys));
			memset(m_byCurKeys, 0, sizeof(m_byCurKeys));
			memset(m_bKeyPresses, 0, sizeof(m_bKeyPresses));
			memset(m_bKeyPresseds, 0, sizeof(m_bKeyPresseds));
		}
		else if (iIndex >= 0 && iIndex < NUMDIKEYS) // 특정한 키만 무효화..
		{
			m_byCurKeys[iIndex] = m_byOldKeys[iIndex] = m_bKeyPresses[iIndex] = m_bKeyPresseds[iIndex] = 0;
		}
	}

	BOOL IsNoKeyDown() const
	{
		return m_bNoKeyDown;
	}

	// 키보드가 눌려있는지... "DInput.h" 에 정의 되어 있는 DIK_???? 스캔코드를 참조..
	BOOL IsKeyDown(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= NUMDIKEYS)
			return FALSE;

		return m_byCurKeys[iIndex];
	}

	// 키보드를 누르는 순간... "DInput.h" 에 정의 되어 있는 DIK_???? 스캔코드를 참조..
	BOOL IsKeyPress(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= NUMDIKEYS)
			return FALSE;

		return m_bKeyPresses[iIndex];
	}

	// 키보드를 누르고나서 떼는 순간... "DInput.h" 에 정의 되어 있는 DIK_???? 스캔코드를 참조..
	BOOL IsKeyPressed(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= NUMDIKEYS)
			return FALSE;

		return m_bKeyPresseds[iIndex];
	}

	BOOL Init(HINSTANCE hInst, HWND hWnd);

	void Tick();
	void KeyboardFlushData();
	void SetActiveDevices(BOOL bKeyboard);
	void MouseSetPos(int x, int y);

	BOOL KeyboardGetKeyState(int nDIKey); // 최근 눌려진 키 검사..

	POINT MouseGetPos() const
	{
		return m_ptCurMouse;
	}

	POINT MouseGetPosOld() const
	{
		return m_ptOldMouse;
	}

	RECT MouseGetLBDragRect() const
	{
		return m_rcLBDrag;
	}

	RECT MouseGetMBDragRect() const
	{
		return m_rcMBDrag;
	}

	RECT MouseGetRBDragRect() const
	{
		return m_rcRBDrag;
	}

	// Mouse Flag 의 or 연산으로 조합되어 있다.
	int MouseGetFlag() const
	{
		return m_nMouseFlag;
	}

	int MouseGetFlagOld() const
	{
		return m_nMouseFlagOld;
	}

	// 특정한 Mouse Flag 제거
	void MouseRemoveFlag(int nFlag = -1)
	{
		if (-1 == nFlag)
			m_nMouseFlag = m_nMouseFlagOld = 0;
		else
			m_nMouseFlag &= (~nFlag);
	}

	CLocalInput();
	~CLocalInput();
};

#endif // end of _LocalInput_H_
