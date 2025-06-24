// DBAgent.cpp: implementation of the CDBAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemManager.h"
#include "DBAgent.h"
#include "ItemManagerDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBAgent::CDBAgent()
{
}

CDBAgent::~CDBAgent()
{
}

BOOL CDBAgent::DatabaseInit()
{
//	Main DB Connecting..
/////////////////////////////////////////////////////////////////////////////////////
	m_pMain = (CItemManagerDlg*) AfxGetMainWnd();

	CString strConnect;
	strConnect.Format(_T("ODBC;DSN=%s;UID=%s;PWD=%s"), m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);

	m_GameDB.SetLoginTimeout(10);
	if (!m_GameDB.Open(nullptr, FALSE, FALSE, strConnect))
	{
		AfxMessageBox("GameDB SQL Connection Fail...");
		return FALSE;
	}

	return TRUE;
}

void CDBAgent::ReConnectODBC(CDatabase* m_db, TCHAR* strdb, TCHAR* strname, TCHAR* strpwd)
{
	TCHAR strlog[256] = {};
	CTime t = CTime::GetCurrentTime();
	wsprintf(strlog, _T("Try ReConnectODBC... %d월 %d일 %d시 %d분\r\n"), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute());
	m_pMain->m_ItemLogFile.Write(strlog, strlen(strlog));

	// DATABASE 연결...
	CString strConnect;
	strConnect.Format(_T("DSN=%s;UID=%s;PWD=%s"), strdb, strname, strpwd);
	int iCount = 0;

	do
	{
		iCount++;
		if (iCount >= 4)
			break;

		m_db->SetLoginTimeout(10);

		try
		{
			m_db->OpenEx(strConnect, CDatabase::noOdbcDialog);
		}
		catch (CDBException* e)
		{
			e->Delete();
		}
	}
	while (!m_db->IsOpen());
}


