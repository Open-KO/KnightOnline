// BattleSet.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "BattleSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBattleSet

IMPLEMENT_DYNAMIC(CBattleSet, CRecordset)

CBattleSet::CBattleSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CBattleSet)
	m_sIndex = 0;
	m_byNation = 0;
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CBattleSet::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CBattleSet::GetDefaultSQL()
{
	return _T("[dbo].[BATTLE]");
}

void CBattleSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CBattleSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sIndex]"), m_sIndex);
	RFX_Byte(pFX, _T("[byNation]"), m_byNation);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CBattleSet diagnostics

#ifdef _DEBUG
void CBattleSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CBattleSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
