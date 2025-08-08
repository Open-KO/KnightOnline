#if !defined(AFX_MAKEITEMTABLESET_H__D5EA9892_40E4_46E3_90E2_764D85335CE6__INCLUDED_)
#define AFX_MAKEITEMTABLESET_H__D5EA9892_40E4_46E3_90E2_764D85335CE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeItemTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMakeItemTableSet recordset

class CMakeItemTableSet : public CRecordset
{
public:
	CMakeItemTableSet(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CMakeItemTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMakeItemTableSet, CRecordset)
	int		m_sIndex;
	CString	m_strItemInfo;
	int		m_sItemCode;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeItemTableSet)
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

#endif // !defined(AFX_MAKEITEMTABLESET_H__D5EA9892_40E4_46E3_90E2_764D85335CE6__INCLUDED_)
