// KnightsRankSet.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "KnightsRankSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKnightsRankSet

IMPLEMENT_DYNAMIC(CKnightsRankSet, CRecordset)

CKnightsRankSet::CKnightsRankSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CKnightsRankSet)
	m_nRank = 0;
	m_shIndex = 0;
	m_strName = _T("");
	m_nPoints = 0;
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CKnightsRankSet::GetDefaultConnect()
{
	return CEbenezerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CKnightsRankSet::GetDefaultSQL()
{
	return _T("[dbo].[KNIGHTS_RATING]");
}

void CKnightsRankSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CKnightsRankSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[nRank]"), m_nRank);
	RFX_Int(pFX, _T("[shIndex]"), m_shIndex);
	RFX_Text(pFX, _T("[strName]"), m_strName);
	RFX_Long(pFX, _T("[nPoints]"), m_nPoints);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CKnightsRankSet diagnostics

#ifdef _DEBUG
void CKnightsRankSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CKnightsRankSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
