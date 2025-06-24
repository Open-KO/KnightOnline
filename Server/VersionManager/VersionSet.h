#if !defined(AFX_VERSIONSET_H__AE3C6553_0D3F_4A4A_AD05_57EF200CB92C__INCLUDED_)
#define AFX_VERSIONSET_H__AE3C6553_0D3F_4A4A_AD05_57EF200CB92C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VersionSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVersionSet recordset

class CVersionSet : public CRecordset
{
public:
	CVersionSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CVersionSet)

// Field/Param Data
	//{{AFX_FIELD(CVersionSet, CRecordset)
	int		m_sVersion;
	CString	m_strFileName;
	BYTE	m_bCompressed;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVersionSet)
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

#endif // !defined(AFX_VERSIONSET_H__AE3C6553_0D3F_4A4A_AD05_57EF200CB92C__INCLUDED_)
