#if !defined(AFX_NPCITEMSET_H__D2328197_D834_45BD_9542_42CA64414ACD__INCLUDED_)
#define AFX_NPCITEMSET_H__D2328197_D834_45BD_9542_42CA64414ACD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NpcItemSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNpcItemSet recordset

class CNpcItemSet : public CRecordset
{
public:
	CNpcItemSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CNpcItemSet)

// Field/Param Data
	//{{AFX_FIELD(CNpcItemSet, CRecordset)
	int		m_sIndex;
	long	m_iItem01;
	int		m_sPersent01;
	long	m_iItem02;
	int		m_sPersent02;
	long	m_iItem03;
	int		m_sPersent03;
	long	m_iItem04;
	int		m_sPersent04;
	long	m_iItem05;
	int		m_sPersent05;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNpcItemSet)
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

#endif // !defined(AFX_NPCITEMSET_H__D2328197_D834_45BD_9542_42CA64414ACD__INCLUDED_)
