#if !defined(AFX_DLGEDITEVENTATTR_H__85B7238D_3801_44D8_BF53_A63A0F01460B__INCLUDED_)
#define AFX_DLGEDITEVENTATTR_H__85B7238D_3801_44D8_BF53_A63A0F01460B__INCLUDED_

#pragma once

// DlgEditEventAttr.h : header file
//

class CEventCell;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditEventAttr dialog

class CDlgEditEventAttr : public CDialog
{
	// Construction
public:
	void SetEventCell(CEventCell* pEvent);
	CDlgEditEventAttr(CWnd* pParent = nullptr); // standard constructor

												// Variables..
	CEventCell* m_pRefEvent;

	// Dialog Data
	//{{AFX_DATA(CDlgEditEventAttr)
	enum
	{
		IDD = IDD_EDIT_EVENT_ATTR
	};
	int m_EventType;
	int m_EventID;
	int m_ZoneID;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditEventAttr)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditEventAttr)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITEVENTATTR_H__85B7238D_3801_44D8_BF53_A63A0F01460B__INCLUDED_)
