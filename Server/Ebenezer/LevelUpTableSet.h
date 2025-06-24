#if !defined(AFX_LEVELUPTABLESET_H__D86CBDDA_46E3_4514_B317_10362D6051DB__INCLUDED_)
#define AFX_LEVELUPTABLESET_H__D86CBDDA_46E3_4514_B317_10362D6051DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LevelUpTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLevelUpTableSet recordset

class CLevelUpTableSet : public CRecordset
{
public:
	CLevelUpTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CLevelUpTableSet)

// Field/Param Data
	//{{AFX_FIELD(CLevelUpTableSet, CRecordset)
	BYTE	m_level;
	long	m_Exp;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelUpTableSet)
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

#endif // !defined(AFX_LEVELUPTABLESET_H__D86CBDDA_46E3_4514_B317_10362D6051DB__INCLUDED_)
