#if !defined(AFX_MAGICTYPE2SET_H__12235842_699A_4909_8C5E_D019ADF5A1E9__INCLUDED_)
#define AFX_MAGICTYPE2SET_H__12235842_699A_4909_8C5E_D019ADF5A1E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MagicType2Set.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMagicType2Set recordset

class CMagicType2Set : public CRecordset
{
public:
	CMagicType2Set(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMagicType2Set)

// Field/Param Data
	//{{AFX_FIELD(CMagicType2Set, CRecordset)
	long	m_iNum;
	CString	m_Name;
	CString	m_Description;
	BYTE	m_HitType;
	int		m_HitRate;
	int		m_AddDamage;
	int		m_AddRange;
	BYTE	m_NeedArrow;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagicType2Set)
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

#endif // !defined(AFX_MAGICTYPE2SET_H__12235842_699A_4909_8C5E_D019ADF5A1E9__INCLUDED_)
