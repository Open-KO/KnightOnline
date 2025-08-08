// DBAgent.h: interface for the CDBAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)
#define AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "define.h"
#include <vector>

class CItemManagerDlg;
class CDBAgent
{
public:
	void ReConnectODBC(CDatabase* m_db, const TCHAR* strdb, const TCHAR* strname, const TCHAR* strpwd);
	BOOL DatabaseInit();
	CDBAgent();
	virtual ~CDBAgent();

	CDatabase	m_GameDB;

	CItemManagerDlg* m_pMain;
};

#endif // !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)
