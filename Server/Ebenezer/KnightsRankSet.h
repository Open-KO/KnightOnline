#if !defined(AFX_KNIGHTSRANKSET_H__BEEB9164_A159_4016_833A_940D6ED178AB__INCLUDED_)
#define AFX_KNIGHTSRANKSET_H__BEEB9164_A159_4016_833A_940D6ED178AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KnightsRankSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKnightsRankSet recordset

class CKnightsRankSet : public CRecordset
{
public:
	CKnightsRankSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CKnightsRankSet)

// Field/Param Data
	//{{AFX_FIELD(CKnightsRankSet, CRecordset)
	long	m_nRank;
	int		m_shIndex;
	CString	m_strName;
	long	m_nPoints;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKnightsRankSet)
public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KNIGHTSRANKSET_H__BEEB9164_A159_4016_833A_940D6ED178AB__INCLUDED_)
