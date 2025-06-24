// MakeGradeItemTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MakeGradeItemTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeGradeItemTableSet

IMPLEMENT_DYNAMIC(CMakeGradeItemTableSet, CRecordset)

CMakeGradeItemTableSet::CMakeGradeItemTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMakeGradeItemTableSet)
	m_byItemIndex = 0;
	m_byGrade_1 = 0;
	m_byGrade_2 = 0;
	m_byGrade_3 = 0;
	m_byGrade_4 = 0;
	m_byGrade_5 = 0;
	m_byGrade_6 = 0;
	m_byGrade_7 = 0;
	m_byGrade_8 = 0;
	m_byGrade_9 = 0;
	m_nFields = 10;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMakeGradeItemTableSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMakeGradeItemTableSet::GetDefaultSQL()
{
	return _T("[dbo].[MAKE_ITEM_GRADECODE]");
}

void CMakeGradeItemTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMakeGradeItemTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[byItemIndex]"), m_byItemIndex);
	RFX_Int(pFX, _T("[byGrade_1]"), m_byGrade_1);
	RFX_Int(pFX, _T("[byGrade_2]"), m_byGrade_2);
	RFX_Int(pFX, _T("[byGrade_3]"), m_byGrade_3);
	RFX_Int(pFX, _T("[byGrade_4]"), m_byGrade_4);
	RFX_Int(pFX, _T("[byGrade_5]"), m_byGrade_5);
	RFX_Int(pFX, _T("[byGrade_6]"), m_byGrade_6);
	RFX_Int(pFX, _T("[byGrade_7]"), m_byGrade_7);
	RFX_Int(pFX, _T("[byGrade_8]"), m_byGrade_8);
	RFX_Int(pFX, _T("[byGrade_9]"), m_byGrade_9);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMakeGradeItemTableSet diagnostics

#ifdef _DEBUG
void CMakeGradeItemTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMakeGradeItemTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
