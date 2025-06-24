// VersionSet.cpp : implementation file
//

#include "stdafx.h"
#include "VersionManager.h"
#include "VersionSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVersionSet

IMPLEMENT_DYNAMIC(CVersionSet, CRecordset)

CVersionSet::CVersionSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CVersionSet)
	m_sVersion = 0;
	m_strFileName = _T("");
	m_bCompressed = 0;
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CVersionSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=Knight_Account;UID=knight;PWD=dnlwhrpdlxm");
}

CString CVersionSet::GetDefaultSQL()
{
	return _T("[dbo].[VERSION]");
}

void CVersionSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CVersionSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sVersion]"), m_sVersion);
	RFX_Text(pFX, _T("[strFileName]"), m_strFileName);
	RFX_Byte(pFX, _T("[bCompressed]"), m_bCompressed);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CVersionSet diagnostics

#ifdef _DEBUG
void CVersionSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CVersionSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
