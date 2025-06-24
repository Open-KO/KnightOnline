// NpcItemSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "NpcItemSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNpcItemSet

IMPLEMENT_DYNAMIC(CNpcItemSet, CRecordset)

CNpcItemSet::CNpcItemSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CNpcItemSet)
	m_sIndex = 0;
	m_iItem01 = 0;
	m_sPersent01 = 0;
	m_iItem02 = 0;
	m_sPersent02 = 0;
	m_iItem03 = 0;
	m_sPersent03 = 0;
	m_iItem04 = 0;
	m_sPersent04 = 0;
	m_iItem05 = 0;
	m_sPersent05 = 0;
	m_nFields = 11;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CNpcItemSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CNpcItemSet::GetDefaultSQL()
{
	return _T("[dbo].[K_MONSTER_ITEM]");
	//return _T("[dbo].[MONSTER_ITEM_TEST]");
}

void CNpcItemSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CNpcItemSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sIndex]"), m_sIndex);
	RFX_Long(pFX, _T("[iItem01]"), m_iItem01);
	RFX_Int(pFX, _T("[sPersent01]"), m_sPersent01);
	RFX_Long(pFX, _T("[iItem02]"), m_iItem02);
	RFX_Int(pFX, _T("[sPersent02]"), m_sPersent02);
	RFX_Long(pFX, _T("[iItem03]"), m_iItem03);
	RFX_Int(pFX, _T("[sPersent03]"), m_sPersent03);
	RFX_Long(pFX, _T("[iItem04]"), m_iItem04);
	RFX_Int(pFX, _T("[sPersent04]"), m_sPersent04);
	RFX_Long(pFX, _T("[iItem05]"), m_iItem05);
	RFX_Int(pFX, _T("[sPersent05]"), m_sPersent05);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CNpcItemSet diagnostics

#ifdef _DEBUG
void CNpcItemSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CNpcItemSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
