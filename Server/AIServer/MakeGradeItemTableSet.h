#if !defined(AFX_MAKEGRADEITEMTABLESET_H__452304E2_8BD2_42BB_81C8_0F827BB85A12__INCLUDED_)
#define AFX_MAKEGRADEITEMTABLESET_H__452304E2_8BD2_42BB_81C8_0F827BB85A12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeGradeItemTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMakeGradeItemTableSet recordset

class CMakeGradeItemTableSet : public CRecordset
{
public:
	CMakeGradeItemTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMakeGradeItemTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMakeGradeItemTableSet, CRecordset)
	BYTE	m_byItemIndex;
	int	m_byGrade_1;
	int	m_byGrade_2;
	int	m_byGrade_3;
	int	m_byGrade_4;
	int	m_byGrade_5;
	int	m_byGrade_6;
	int	m_byGrade_7;
	int	m_byGrade_8;
	int	m_byGrade_9;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeGradeItemTableSet)
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

#endif // !defined(AFX_MAKEGRADEITEMTABLESET_H__452304E2_8BD2_42BB_81C8_0F827BB85A12__INCLUDED_)
