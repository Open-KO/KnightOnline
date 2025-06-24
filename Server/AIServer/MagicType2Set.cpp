// MagicType2Set.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MagicType2Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType2Set

IMPLEMENT_DYNAMIC(CMagicType2Set, CRecordset)

CMagicType2Set::CMagicType2Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType2Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_HitType = 0;
	m_HitRate = 0;
	m_AddDamage = 0;
	m_AddRange = 0;
	m_NeedArrow = 0;
	m_nFields = 8;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMagicType2Set::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMagicType2Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE2]");
}

void CMagicType2Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType2Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[HitType]"), m_HitType);
	RFX_Int(pFX, _T("[HitRate]"), m_HitRate);
	RFX_Int(pFX, _T("[AddDamage]"), m_AddDamage);
	RFX_Int(pFX, _T("[AddRange]"), m_AddRange);
	RFX_Byte(pFX, _T("[NeedArrow]"), m_NeedArrow);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType2Set diagnostics

#ifdef _DEBUG
void CMagicType2Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType2Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
