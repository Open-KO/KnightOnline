#if !defined(AFX_DLGSHAPELIST_H__25FFD3FA_9331_4682_A4C0_80124277A0AB__INCLUDED_)
#define AFX_DLGSHAPELIST_H__25FFD3FA_9331_4682_A4C0_80124277A0AB__INCLUDED_

#pragma once

// DlgShapeList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgShapeList dialog
#include "Resource.h"
#include <N3Base/N3Base.h>

class CN3Scene;
class CMapMng;

class CDlgShapeList : public CDialog
{
	// Construction
public:
	CDlgShapeList(CWnd* pParent = nullptr); // standard constructor

	CN3Scene* m_pSceneRef;
	BOOL m_IsSourceObj;                     // Display용이냐? (툴에서 오브젝트 목록)
	CMapMng* m_pMapMng;

											// Dialog Data
	//{{AFX_DATA(CDlgShapeList)
	enum
	{
		IDD = IDD_SHAPE_LIST
	};
	CListBox m_ListShape;
	//}}AFX_DATA

	void UpdateTree(CN3Scene* pScene);
	void SelectObject(void* pItemData);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgShapeList)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgShapeList)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListObj();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBtnSort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSHAPELIST_H__25FFD3FA_9331_4682_A4C0_80124277A0AB__INCLUDED_)
