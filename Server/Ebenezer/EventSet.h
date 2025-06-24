#if !defined(AFX_EVENTSET_H__20DA4BFC_4D30_4A59_89BF_4A1998F13B69__INCLUDED_)
#define AFX_EVENTSET_H__20DA4BFC_4D30_4A59_89BF_4A1998F13B69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventSet recordset

class CEventSet : public CRecordset
{
public:
	CEventSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CEventSet)

// Field/Param Data
	//{{AFX_FIELD(CEventSet, CRecordset)
	BYTE	m_ZoneNum;
	int		m_EventNum;
	BYTE	m_Type;
	CString	m_Cond1;
	CString	m_Cond2;
	CString	m_Cond3;
	CString	m_Cond4;
	CString	m_Cond5;
	CString	m_Exec1;
	CString	m_Exec2;
	CString	m_Exec3;
	CString	m_Exec4;
	CString	m_Exec5;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventSet)
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

#endif // !defined(AFX_EVENTSET_H__20DA4BFC_4D30_4A59_89BF_4A1998F13B69__INCLUDED_)
