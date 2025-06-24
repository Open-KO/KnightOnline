// MagicType8Set.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "MagicType8Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType8Set

IMPLEMENT_DYNAMIC(CMagicType8Set, CRecordset)

CMagicType8Set::CMagicType8Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType8Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_Target = 0;
	m_Radius = 0;
	m_WarpType = 0;
	m_ExpRecover = 0;
	m_nFields = 7;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CMagicType8Set::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CMagicType8Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE8]");
}

void CMagicType8Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType8Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[Target]"), m_Target);
	RFX_Int(pFX, _T("[Radius]"), m_Radius);
	RFX_Byte(pFX, _T("[WarpType]"), m_WarpType);
	RFX_Int(pFX, _T("[ExpRecover]"), m_ExpRecover);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType8Set diagnostics

#ifdef _DEBUG
void CMagicType8Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType8Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
