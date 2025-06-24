// CoefficientSet.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "CoefficientSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCoefficientSet

IMPLEMENT_DYNAMIC(CCoefficientSet, CRecordset)

CCoefficientSet::CCoefficientSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CCoefficientSet)
	m_sClass = 0;
	m_ShortSword = 0.0;
	m_Sword = 0.0;
	m_Axe = 0.0;
	m_Club = 0.0;
	m_Spear = 0.0;
	m_Pole = 0.0;
	m_Staff = 0.0;
	m_Bow = 0.0;
	m_Hp = 0.0;
	m_Mp = 0.0;
	m_Sp = 0.0;
	m_Ac = 0.0;
	m_Hitrate = 0.0;
	m_Evasionrate = 0.0;
	m_nFields = 15;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CCoefficientSet::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CCoefficientSet::GetDefaultSQL()
{
	return _T("[dbo].[COEFFICIENT]");
}

void CCoefficientSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CCoefficientSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sClass]"), m_sClass);
	RFX_Double(pFX, _T("[ShortSword]"), m_ShortSword);
	RFX_Double(pFX, _T("[Sword]"), m_Sword);
	RFX_Double(pFX, _T("[Axe]"), m_Axe);
	RFX_Double(pFX, _T("[Club]"), m_Club);
	RFX_Double(pFX, _T("[Spear]"), m_Spear);
	RFX_Double(pFX, _T("[Pole]"), m_Pole);
	RFX_Double(pFX, _T("[Staff]"), m_Staff);
	RFX_Double(pFX, _T("[Bow]"), m_Bow);
	RFX_Double(pFX, _T("[Hp]"), m_Hp);
	RFX_Double(pFX, _T("[Mp]"), m_Mp);
	RFX_Double(pFX, _T("[Sp]"), m_Sp);
	RFX_Double(pFX, _T("[Ac]"), m_Ac);
	RFX_Double(pFX, _T("[Hitrate]"), m_Hitrate);
	RFX_Double(pFX, _T("[Evasionrate]"), m_Evasionrate);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CCoefficientSet diagnostics

#ifdef _DEBUG
void CCoefficientSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CCoefficientSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
