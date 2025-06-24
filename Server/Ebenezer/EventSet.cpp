// EventSet.cpp : implementation file
//

#include "stdafx.h"
#include "ebenezer.h"
#include "EventSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventSet

IMPLEMENT_DYNAMIC(CEventSet, CRecordset)

CEventSet::CEventSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CEventSet)
	m_ZoneNum = 0;
	m_EventNum = 0;
	m_Type = 0;
	m_Cond1 = _T("");
	m_Cond2 = _T("");
	m_Cond3 = _T("");
	m_Cond4 = _T("");
	m_Cond5 = _T("");
	m_Exec1 = _T("");
	m_Exec2 = _T("");
	m_Exec3 = _T("");
	m_Exec4 = _T("");
	m_Exec5 = _T("");
	m_nFields = 13;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CEventSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=KN_Online;UID=knight;PWD=knight");
}

CString CEventSet::GetDefaultSQL()
{
	return _T("[dbo].[EVENT]");
}

void CEventSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CEventSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[ZoneNum]"), m_ZoneNum);
	RFX_Int(pFX, _T("[EventNum]"), m_EventNum);
	RFX_Byte(pFX, _T("[Type]"), m_Type);
	RFX_Text(pFX, _T("[Cond1]"), m_Cond1);
	RFX_Text(pFX, _T("[Cond2]"), m_Cond2);
	RFX_Text(pFX, _T("[Cond3]"), m_Cond3);
	RFX_Text(pFX, _T("[Cond4]"), m_Cond4);
	RFX_Text(pFX, _T("[Cond5]"), m_Cond5);
	RFX_Text(pFX, _T("[Exec1]"), m_Exec1);
	RFX_Text(pFX, _T("[Exec2]"), m_Exec2);
	RFX_Text(pFX, _T("[Exec3]"), m_Exec3);
	RFX_Text(pFX, _T("[Exec4]"), m_Exec4);
	RFX_Text(pFX, _T("[Exec5]"), m_Exec5);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CEventSet diagnostics

#ifdef _DEBUG
void CEventSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CEventSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
