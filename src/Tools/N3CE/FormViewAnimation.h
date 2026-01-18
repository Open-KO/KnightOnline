#if !defined(AFX_FORMVIEWANIMATION_H__35364EF4_04E3_427C_8B17_6D4D88DF823B__INCLUDED_)
#define AFX_FORMVIEWANIMATION_H__35364EF4_04E3_427C_8B17_6D4D88DF823B__INCLUDED_

#pragma once

// FormViewAnimation.h : header file
//
#include <N3Base/N3Chr.h>
#include "MacSliderCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// CFormViewAnimation form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CN3CEDoc;

class CFormViewAnimation : public CFormView
{
public:
	BOOL m_bUpdatingNow;
	__AnimData m_AnimDataTemp; // 임시 복사 및 붙이기용..

public:
	static int SortByName(const void* pArg1, const void* pArg2);
	static int SortByFrame(const void* pArg1, const void* pArg2);
	void OnOK();
	void GetData();
	void UpdateFrameSliderAndOther();
	void UpdateInfo();
	void UpdateAllInfo();
	void CalculateDelayTimeAndUpdate(int iAnimationIndexOffset);

	CN3CEDoc* GetDocument();

protected:
	CFormViewAnimation(); // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFormViewAnimation)

						  // Form Data
public:
	//{{AFX_DATA(CFormViewAnimation)
	enum
	{
		IDD = IDD_ANIMATION
	};
	CListBox m_ListSound;
	CMacSliderCtrl m_SldSceneFrm;
	CMacSliderCtrl m_SldFrm;
	CButton m_CPlay;
	CListBox m_ListAnim0;
	CListBox m_ListAnim1;
	//}}AFX_DATA

	// Attributes
public:
	// Operations
public:
	__AnimData* GetCurAniData();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormViewAnimation)
public:
	virtual void OnInitialUpdate();

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~CFormViewAnimation();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Generated message map functions
	//{{AFX_MSG(CFormViewAnimation)
	afx_msg void OnBSetFrameStart();
	afx_msg void OnBSetFrameEnd();
	afx_msg void OnBSetFramePlugTraceStart();
	afx_msg void OnBSetFramePlugTraceEnd();
	afx_msg void OnBSetFrameSound0();
	afx_msg void OnBSetFrameSound1();
	afx_msg void OnCPlay();
	afx_msg void OnSelchangeListAnimation0();
	afx_msg void OnSelchangeListAnimation1();
	afx_msg void OnBAdd();
	afx_msg void OnBInsert();
	afx_msg void OnBDelete();
	afx_msg void OnBLoad();
	afx_msg void OnBSave();
	afx_msg void OnBMoveUp();
	afx_msg void OnBMoveDown();
	afx_msg void OnChangeESceneFrmStart();
	afx_msg void OnChangeESceneFrmEnd();
	afx_msg void OnChangeEAniName();
	afx_msg void OnChangeEFrameStart();
	afx_msg void OnChangeEFrameEnd();
	afx_msg void OnChangeEFramePerSec();
	afx_msg void OnChangeEFramePlugTraceStart();
	afx_msg void OnChangeEFramePlugTraceEnd();
	afx_msg void OnChangeEFrameSound0();
	afx_msg void OnChangeEFrameSound1();
	afx_msg void OnChangeEFrameBlend0();
	afx_msg void OnCDelayedLooping();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBSetFrameStrike0();
	afx_msg void OnBSetFrameStrike1();
	afx_msg void OnChangeEFrameStrike0();
	afx_msg void OnChangeEFrameStrike1();
	afx_msg void OnEditAnimationDataAdd();
	afx_msg void OnEditAnimationDataInsert();
	afx_msg void OnEditAnimationDataDelete();
	afx_msg void OnEditAnimationDataCopy();
	afx_msg void OnEditAnimationDataPaste();
	afx_msg void OnEditAnimationDataMoveUp();
	afx_msg void OnEditAnimationDataMoveDown();
	afx_msg void OnDblclkListAnimation0();
	afx_msg void OnBNew();
	afx_msg void OnBSaveAs();
	afx_msg void OnBCalculateDelayTimeWithUpperAnimation();
	afx_msg void OnBCalculateDelayTimeWithLowerAnimation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG // debug version in FormViewAnimation.cpp
inline CN3CEDoc* CFormViewAnimation::GetDocument()
{
	return (CN3CEDoc*) m_pDocument;
}
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMVIEWANIMATION_H__35364EF4_04E3_427C_8B17_6D4D88DF823B__INCLUDED_)
