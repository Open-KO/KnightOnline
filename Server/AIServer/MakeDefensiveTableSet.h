#if !defined(AFX_MAKEDEFENSIVETABLESET_H__DB434CB6_7AFE_4FD3_9B74_F26C1E4D0ED4__INCLUDED_)
#define AFX_MAKEDEFENSIVETABLESET_H__DB434CB6_7AFE_4FD3_9B74_F26C1E4D0ED4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeDefensiveTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMakeDefensiveTableSet recordset

class CMakeDefensiveTableSet : public CRecordset
{
public:
	CMakeDefensiveTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMakeDefensiveTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMakeDefensiveTableSet, CRecordset)
	BYTE	m_byLevel;
	int		m_sClass_1;
	int		m_sClass_2;
	int		m_sClass_3;
	int		m_sClass_4;
	int		m_sClass_5;
	int		m_sClass_6;
	int		m_sClass_7;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeDefensiveTableSet)
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

#endif // !defined(AFX_MAKEDEFENSIVETABLESET_H__DB434CB6_7AFE_4FD3_9B74_F26C1E4D0ED4__INCLUDED_)
