#if !defined(AFX_KNIGHTSUSERSET_H__1246D470_46DD_415D_A548_86829EAE6465__INCLUDED_)
#define AFX_KNIGHTSUSERSET_H__1246D470_46DD_415D_A548_86829EAE6465__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KnightsUserSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKnightsUserSet recordset

class CKnightsUserSet : public CRecordset
{
public:
	CKnightsUserSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CKnightsUserSet)

// Field/Param Data
	//{{AFX_FIELD(CKnightsUserSet, CRecordset)
	int		m_sIDNum;
	CString	m_strUserID;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKnightsUserSet)
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

#endif // !defined(AFX_KNIGHTSUSERSET_H__1246D470_46DD_415D_A548_86829EAE6465__INCLUDED_)
