// KnightsSet.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "KnightsSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKnightsSet

IMPLEMENT_DYNAMIC(CKnightsSet, CRecordset)

CKnightsSet::CKnightsSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CKnightsSet)
	m_IDNum = 0;
	m_Flag = 0;
	m_Nation = 0;
	m_Ranking = 0;
	m_IDName = _T("");
	m_Members = 0;
	m_Chief = _T("");
	m_ViceChief_1 = _T("");
	m_ViceChief_2 = _T("");
	m_ViceChief_3 = _T("");
	m_Gold = _T("");
	m_Domination = 0;
	m_Points = 0;
	m_Stash = _T("");
	m_nFields = 16;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CKnightsSet::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CKnightsSet::GetDefaultSQL()
{
	return _T("[dbo].[KNIGHTS]");
}

void CKnightsSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CKnightsSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[IDNum]"), m_IDNum);
	RFX_Byte(pFX, _T("[Flag]"), m_Flag);
	RFX_Byte(pFX, _T("[Nation]"), m_Nation);
	RFX_Byte(pFX, _T("[Ranking]"), m_Ranking);
	RFX_Text(pFX, _T("[IDName]"), m_IDName);
	RFX_Int(pFX, _T("[Members]"), m_Members);
	RFX_Text(pFX, _T("[Chief]"), m_Chief);
	RFX_Text(pFX, _T("[ViceChief_1]"), m_ViceChief_1);
	RFX_Text(pFX, _T("[ViceChief_2]"), m_ViceChief_2);
	RFX_Text(pFX, _T("[ViceChief_3]"), m_ViceChief_3);
	RFX_Text(pFX, _T("[Gold]"), m_Gold);
	RFX_Int(pFX, _T("[Domination]"), m_Domination);
	RFX_Long(pFX, _T("[Points]"), m_Points);
	RFX_Date(pFX, _T("[CreateTime]"), m_CreateTime);
	RFX_LongBinary(pFX, _T("[Mark]"), m_Mark);
	RFX_Text(pFX, _T("[Stash]"), m_Stash);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CKnightsSet diagnostics

#ifdef _DEBUG
void CKnightsSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CKnightsSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
