#if !defined(AFX_MAKELAREITEMTABLESET_H__52F441CA_9CEB_469A_97E7_1AC154A492DE__INCLUDED_)
#define AFX_MAKELAREITEMTABLESET_H__52F441CA_9CEB_469A_97E7_1AC154A492DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeLareItemTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMakeLareItemTableSet recordset

class CMakeLareItemTableSet : public CRecordset
{
public:
	CMakeLareItemTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMakeLareItemTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMakeLareItemTableSet, CRecordset)
	BYTE	m_byLevelGrade;
	int		m_sLareItem;
	int		m_sMagicItem;
	int		m_sGereralItem;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeLareItemTableSet)
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

#endif // !defined(AFX_MAKELAREITEMTABLESET_H__52F441CA_9CEB_469A_97E7_1AC154A492DE__INCLUDED_)
