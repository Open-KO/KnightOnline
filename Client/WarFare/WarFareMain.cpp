/*
*/

#include "stdafx.h"
#include "UIChat.h"
#include "GameEng.h"
#include "resource.h"
#include "N3SndMgr.h"
#include "N3UIEdit.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "GameProcMain.h"
#include "N3WorldManager.h"

#include <shared/Ini.h>

#include <WinSock2.h>
#include <time.h>

#include "DFont.h"
#include "UIManager.h"
#include "UIMessageBoxManager.h"

#include <windowsx.h>

HWND CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	// NOTE: get the current directory and make it known to CN3Base
	char szPath[_MAX_PATH] = "";
	GetCurrentDirectory(_MAX_PATH, szPath);
	CN3Base::PathSet(szPath);

	// NOTE: we are anticipating an Options file to exist within this directory
	char szIniPath[_MAX_PATH] = "";
	lstrcpy(szIniPath, CN3Base::PathGet().c_str());
	lstrcat(szIniPath, "Option.ini");

	CIni ini(szIniPath);

	// NOTE: what is the texture quality?
	CN3Base::s_Options.iTexLOD_Chr = ini.GetInt("Texture", "LOD_Chr", 0);
	if(CN3Base::s_Options.iTexLOD_Chr < 0) CN3Base::s_Options.iTexLOD_Chr = 0;
	if(CN3Base::s_Options.iTexLOD_Chr >= 2) CN3Base::s_Options.iTexLOD_Chr = 1;
	
	// NOTE: what is the texture quality?
	CN3Base::s_Options.iTexLOD_Shape = ini.GetInt("Texture", "LOD_Shape", 0);
	if(CN3Base::s_Options.iTexLOD_Shape < 0) CN3Base::s_Options.iTexLOD_Shape = 0;
	if(CN3Base::s_Options.iTexLOD_Shape >= 2) CN3Base::s_Options.iTexLOD_Shape = 1;

	// NOTE: what is the texture quality?
	CN3Base::s_Options.iTexLOD_Terrain = ini.GetInt("Texture", "LOD_Terrain", 0);
	if(CN3Base::s_Options.iTexLOD_Terrain < 0) CN3Base::s_Options.iTexLOD_Terrain = 0;
	if(CN3Base::s_Options.iTexLOD_Terrain >= 2) CN3Base::s_Options.iTexLOD_Terrain = 1;

	// NOTE: should we use shadows?
	CN3Base::s_Options.iUseShadow = ini.GetInt("Shadow", "Use", 1);

	// NOTE: what is the screen resolution?
	CN3Base::s_Options.iViewWidth = ini.GetInt("ViewPort", "Width", 1024);
	CN3Base::s_Options.iViewHeight = ini.GetInt("ViewPort", "Height", 768);
	
	if(CN3Base::s_Options.iViewWidth == 1024) CN3Base::s_Options.iViewHeight = 768;
	else if(1280 == CN3Base::s_Options.iViewWidth) CN3Base::s_Options.iViewHeight = 1024;
	else if(1600 == CN3Base::s_Options.iViewWidth) CN3Base::s_Options.iViewHeight = 1200;
	else if (1366 == CN3Base::s_Options.iViewWidth) CN3Base::s_Options.iViewHeight = 768;
	else if(1920 == CN3Base::s_Options.iViewWidth) CN3Base::s_Options.iViewHeight = 1080;
	/*
	else {
		CN3Base::s_Options.iViewWidth = 1024;
		CN3Base::s_Options.iViewHeight = 768;
	*/

	// NOTE: what is the viewport's color depth?
	// Officially this defaults to 16-bit, but this isn't as supported these days so we should
	// just default to 32-bit to ensure compatibility with ChangeDisplaySettings().
	CN3Base::s_Options.iViewColorDepth = ini.GetInt("ViewPort", "ColorDepth", 32);
	if (CN3Base::s_Options.iViewColorDepth != 16
		&& CN3Base::s_Options.iViewColorDepth != 32)
		CN3Base::s_Options.iViewColorDepth = 32;

	// NOTE: what is the viewport's draw distance?
	CN3Base::s_Options.iViewDist = ini.GetInt("ViewPort", "Distance", 512);
	if(CN3Base::s_Options.iViewDist < 256) CN3Base::s_Options.iViewDist = 256;
	if(CN3Base::s_Options.iViewDist > 512) CN3Base::s_Options.iViewDist = 512;

	// NOTE: what is the distance for sound events?
	CN3Base::s_Options.iEffectSndDist = ini.GetInt("Sound", "Distance", 48);
	if(CN3Base::s_Options.iEffectSndDist < 20) CN3Base::s_Options.iEffectSndDist = 20;
	if(CN3Base::s_Options.iEffectSndDist > 48) CN3Base::s_Options.iEffectSndDist = 48;

	// NOTE: is sound enabled?
	CN3Base::s_Options.bSndEnable = ini.GetBool("Sound", "Enable", true);

	// NOTE: is sound duplicated?
	CN3Base::s_Options.bSndDuplicated = ini.GetBool("Sound", "Duplicate", false);

	// NOTE: should we use the Windows cursor? If false, will use the software cursor (CGameCursor) instead.
	CN3Base::s_Options.bWindowCursor = ini.GetBool("Cursor", "WindowCursor", true);
																	   
	// NOTE: should we show window full screen?
	CN3Base::s_Options.bWindowMode = ini.GetBool(
		"Screen",
		"WindowMode",
		// In debug builds, if not otherwise configured, we should just prefer to use windowed mode.
#if defined(_DEBUG)
		true
#else
		false
#endif
	);

	CN3Base::s_Options.bVSyncEnabled = ini.GetBool("Screen", "VSyncEnabled", true);

	srand((uint32_t) time(nullptr));

	// 메인 윈도우를 만들고..
	HWND hWndMain = CreateMainWindow(hInstance);
	if (hWndMain == nullptr)
	{
		CLogWriter::Write("Cannot create window.");
		exit(-1);
	}

	::ShowWindow(hWndMain, nCmdShow); // 보여준다..
	::SetActiveWindow(hWndMain);

	CGameProcedure::s_bWindowed = true;

	// allocate the static members
	CGameProcedure::StaticMemberInit(
		hInstance,
		hWndMain);

	// set the game's current procedure to s_pProcLogIn
	CGameProcedure::ProcActiveSet((CGameProcedure*)CGameProcedure::s_pProcLogIn);

#if _DEBUG
	HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCELATOR) );
	HDC hDC = GetDC(hWndMain);
#endif // #if _DEBUG

	MSG msg = {};
	BOOL bGotMsg = FALSE;

	while (WM_QUIT != msg.message)
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		bGotMsg = PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE);
		if (bGotMsg)
		{
#if _DEBUG
			if (0 == TranslateAccelerator(hWndMain, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
#else
			TranslateMessage(&msg);
			DispatchMessage(&msg);
#endif // #if _DEBUG
		}
		else
		{
			// Render a frame during idle time (no messages are waiting)
			CGameProcedure::TickActive();
			CGameProcedure::RenderActive();
		}
	}

#if _DEBUG
	ReleaseDC(hWndMain, hDC);
	DestroyAcceleratorTable(hAccel);
#endif // #if _DEBUG

	CGameProcedure::StaticMemberRelease();

	return msg.wParam;
}

HWND CreateMainWindow(HINSTANCE hInstance)
{
	WNDCLASSEXA wc;

	//  only register the window class once - use hInstance as a flag. 
	wc.cbSize        = sizeof(WNDCLASSEXA);
	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) WndProcMain;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.hCursor       = nullptr;
	wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = "Knight OnLine Client";
	wc.hIconSm       = nullptr;

	if (0 == ::RegisterClassExA(&wc))
	{
		CLogWriter::Write("Cannot register window class.");
		exit(-1);
	}

	DWORD style;
	int iViewWidth, iViewHeight;
	if (CN3Base::s_Options.bWindowMode)
	{
		style = WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_GROUP;

		RECT rc;
		rc.left = 0;
		rc.right = CN3Base::s_Options.iViewWidth;
		rc.top = 0;
		rc.bottom = CN3Base::s_Options.iViewHeight;

		AdjustWindowRect(&rc, style, FALSE);

		iViewWidth = rc.right - rc.left;
		iViewHeight = rc.bottom - rc.top;
	}
	else
	{
		style = WS_POPUP | WS_CLIPCHILDREN;
		iViewWidth = CN3Base::s_Options.iViewWidth;
		iViewHeight = CN3Base::s_Options.iViewHeight;
	}

	return ::CreateWindowExA(
		0,
		wc.lpszClassName,
		"Knight OnLine Client",
		style,
		0,
		0,
		iViewWidth,
		iViewHeight,
		0,
		0,
		hInstance,
		nullptr);
}

/*
	WndProcMain processes the messages for the main window
*/
LRESULT CALLBACK WndProcMain(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
		{
			uint16_t wNotifyCode = HIWORD(wParam); // notification code
			CN3UIEdit* pEdit = CN3UIEdit::GetFocusedEdit();

			if (wNotifyCode == EN_CHANGE && pEdit)
			{
				uint16_t wID = LOWORD(wParam); // item, control, or accelerator identifier
				HWND hwndCtl = (HWND) lParam;

				if (CN3UIEdit::s_hWndEdit == hwndCtl)
				{
					pEdit->UpdateTextFromEditCtrl();
					pEdit->UpdateCaretPosFromEditCtrl();
					CGameProcedure::SetGameCursor(CGameProcedure::s_hCursorNormal);
				}
			}
		} break;

		case WM_SOCKETMSG:
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
				case FD_CONNECT:
				{
				  //TRACE("Socket connected..\n");
				} break;
				case FD_CLOSE:
				{
					if (CGameProcedure::s_bNeedReportConnectionClosed)
						CGameProcedure::ReportServerConnectionClosed(true);
					//TRACE("Socket closed..\n");
				}  break;
				case FD_READ:
				{
					CGameProcedure::s_pSocket->Receive();
				} break;
				default:
				{
					__ASSERT(0, "WM_SOCKETMSG: unknown socket flag.");
				} break;
			}
		} break;

		case WM_ACTIVATE:
		{
			int iActive = LOWORD(wParam);           // activation flag 
			int iMinimized = (BOOL) HIWORD(wParam); // minimized flag 
			HWND hwndPrevious = (HWND) lParam;      // window handle 

			switch (iActive)
			{
				case WA_CLICKACTIVE:
				case WA_ACTIVE:
				{
					SetFocus(hWnd);

					CN3UIEdit* pUIFocused = CN3UIBase::GetFocusedEdit();
					if (pUIFocused != nullptr)
					{
						pUIFocused->KillFocus();
						pUIFocused->SetFocus();
					}

					CGameProcedure::s_bIsWindowInFocus = true;
				}
				return 1;

				case WA_INACTIVE:
					CGameProcedure::s_bIsWindowInFocus = false;

					if (!CGameProcedure::s_bWindowed)
					{
						CLogWriter::Write("WA_INACTIVE.");
						PostQuitMessage(0);
					}
					break;
			}
		} break;

		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
		{
			if (CGameProcedure::s_pProcActive != nullptr
				&& CGameProcedure::s_pProcActive == CGameProcedure::s_pProcMain)
			{
				if (!_IsKeyDown(VK_MENU))
				{
					CGameProcedure::s_pProcMain->RequestExit();
					return 1;
				}

				if (CGameProcedure::s_pProcMain->m_fExitTimer != -1.0f)
				{
					if (CGameProcedure::s_pProcMain->m_pUIChatDlg != nullptr)
					{
						std::string szMsg;
						CGameBase::GetText(IDS_CANNOT_EXIT_DURING_A_BATTLE, &szMsg);
						CGameProcedure::s_pProcMain->m_pUIChatDlg->AddChatMsg(N3_CHAT_NORMAL, szMsg, 0xFFFF0000);
						CGameProcedure::s_pProcMain->m_eExitType = EXIT_TYPE_QUIT;
					}

					return 1;
				}
			}

			CGameProcedure::s_pSocket->Disconnect();
			CGameProcedure::s_pSocketSub->Disconnect();

			PostQuitMessage(0);
		} break;

		case WM_MOUSEWHEEL:
		{
			CN3UIBase* pUI = nullptr;
			if (CGameProcedure::s_pMsgBoxMgr != nullptr)
				pUI = CGameProcedure::s_pMsgBoxMgr->GetFocusMsgBox();

			short delta = GET_WHEEL_DELTA_WPARAM(wParam);

			if (pUI != nullptr
				&& pUI->IsVisible()
				&& pUI->OnMouseWheelEvent(delta))
				break;

			if (CGameProcedure::s_pUIMgr != nullptr)
				pUI = CGameProcedure::s_pUIMgr->GetFocusedUI();

			if (pUI != nullptr
				&& pUI->IsVisible()
				&& pUI->OnMouseWheelEvent(delta))
				break;

			if (CGameProcedure::s_pProcActive == CGameProcedure::s_pProcMain)
				CGameProcedure::s_pEng->CameraZoom(delta * 0.05f);
		} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
