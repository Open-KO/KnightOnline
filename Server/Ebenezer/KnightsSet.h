#if !defined(AFX_KNIGHTSSET_H__1942BDDE_8833_404F_83C6_EFE16E0435F4__INCLUDED_)
#define AFX_KNIGHTSSET_H__1942BDDE_8833_404F_83C6_EFE16E0435F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KnightsSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKnightsSet recordset

class CKnightsSet : public CRecordset
{
public:
	CKnightsSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CKnightsSet)

// Field/Param Data
	//{{AFX_FIELD(CKnightsSet, CRecordset)
	int		m_IDNum;
	BYTE	m_Flag;
	BYTE	m_Nation;
	BYTE	m_Ranking;
	CString	m_IDName;
	int		m_Members;
	CString	m_Chief;
	CString	m_ViceChief_1;
	CString	m_ViceChief_2;
	CString	m_ViceChief_3;
	CString	m_Gold;
	int		m_Domination;
	long	m_Points;
	CTime	m_CreateTime;
	CLongBinary	m_Mark;
	CString	m_Stash;
	int		m_MarkVersion;
	int		m_AllianceKnights;
	int		m_Cape;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKnightsSet)
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

#endif // !defined(AFX_KNIGHTSSET_H__1942BDDE_8833_404F_83C6_EFE16E0435F4__INCLUDED_)
