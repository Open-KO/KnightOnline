// LevelUpTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "LevelUpTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLevelUpTableSet

IMPLEMENT_DYNAMIC(CLevelUpTableSet, CRecordset)

CLevelUpTableSet::CLevelUpTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CLevelUpTableSet)
	m_level = 0;
	m_Exp = 0;
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CLevelUpTableSet::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CLevelUpTableSet::GetDefaultSQL()
{
	return _T("[dbo].[LEVEL_UP]");
}

void CLevelUpTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CLevelUpTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[level]"), m_level);
	RFX_Long(pFX, _T("[Exp]"), m_Exp);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CLevelUpTableSet diagnostics

#ifdef _DEBUG
void CLevelUpTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CLevelUpTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
