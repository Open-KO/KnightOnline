// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7F6B8F8D_6B2B_47EE_A744_F49F1D4A2556__INCLUDED_)
#define AFX_STDAFX_H__7F6B8F8D_6B2B_47EE_A744_F49F1D4A2556__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT _WIN32_WINNT_MAXVER
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winsock2.h>		// Winsock 2
#include <afxtempl.h>
#include <afxdb.h>

//#include "Mmsystem.h"
//#include "Imm.h"
//#include "./N3Base/My_3DStruct.h"

#include "packet.h"			// packet 정의
#include "global.h"			// 전역 funtion
#include "define.h"			// define
//#include "extern.h"			// 전역 객체

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7F6B8F8D_6B2B_47EE_A744_F49F1D4A2556__INCLUDED_)
