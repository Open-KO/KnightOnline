// User.h: interface for the CUser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USER_H__44B01102_A28D_4527_BCBC_1815DD57BCB0__INCLUDED_)
#define AFX_USER_H__44B01102_A28D_4527_BCBC_1815DD57BCB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPSocket2.h"

class CVersionManagerDlg;
class CUser : public CIOCPSocket2
{
public:
	CUser();
	virtual ~CUser();

	void Initialize();
	void Parsing(int len, char* pData);
	void CloseProcess();

	void SendDownloadInfo(int version);
	void MgameLogin(char* pBuf);
	void LogInReq(char* pBuf);

	CVersionManagerDlg* m_pMain;
};

#endif // !defined(AFX_USER_H__44B01102_A28D_4527_BCBC_1815DD57BCB0__INCLUDED_)
