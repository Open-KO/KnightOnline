// MagicType5Set.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "MagicType5Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType5Set

IMPLEMENT_DYNAMIC(CMagicType5Set, CRecordset)

CMagicType5Set::CMagicType5Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType5Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_Type = 0;
	m_ExpRecover = 0;
	m_NeedStone = 0;
	m_nFields = 6;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CMagicType5Set::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CMagicType5Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE5]");
}

void CMagicType5Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType5Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[Type]"), m_Type);
	RFX_Byte(pFX, _T("[ExpRecover]"), m_ExpRecover);
	RFX_Int(pFX, _T("[NeedStone]"), m_NeedStone);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType5Set diagnostics

#ifdef _DEBUG
void CMagicType5Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType5Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
