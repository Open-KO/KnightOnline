// UIEDoc.h : interface of the CUIEDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIEDOC_H__EE8E9346_D90D_48A7_89CE_DC0950272A0E__INCLUDED_)
#define AFX_UIEDOC_H__EE8E9346_D90D_48A7_89CE_DC0950272A0E__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>
#include <list>

typedef std::list<CN3UIBase*>::iterator it_UI;

class CUIEDoc : public CDocument
{
protected: // create from serialization only
	CUIEDoc();
	DECLARE_DYNCREATE(CUIEDoc)

protected:
	CN3UIBase m_RootUI; // 이 것에 child(tool에서 만드는 모든 control)들을 붙이면 된다.
	std::list<CN3UIBase*> m_SelectedUIs;

public:
	CN3UIBase* GetRootUI()
	{
		return &m_RootUI;
	}

	int GetSelectedUICount() const
	{
		return static_cast<int>(m_SelectedUIs.size());
	}

	CN3UIBase* GetSelectedUI(int iIndex = 0)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_SelectedUIs.size()))
			return nullptr;

		auto it = m_SelectedUIs.begin();
		std::advance(it, iIndex);
		return *it;
	}

	void SetSelectedUI(CN3UIBase* pUI);

protected:
	void Release();                            // Document 변수 초기화
	BOOL SetImageInfos(CN3UIImage* pUI);       // image정보 넣는 루틴
	BOOL SetStringInfos(CN3UIString* pUI);     // string
	BOOL SetTrackBarInfos(CN3UITrackBar* pUI); // track bar
	BOOL SetButtonInfos(CN3UIButton* pUI);     // button

public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIEDoc)
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CUIEDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// Generated message map functions
protected:
	//{{AFX_MSG(CUIEDoc)
	afx_msg void OnFileExport();
	afx_msg void OnFileImport();
	afx_msg void OnFileImportTooltip();
	afx_msg void OnFileExportTooltip();
	afx_msg void OnUpdateFileExportTooltip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnInsertImage();
	afx_msg void OnInsertString();
	afx_msg void OnInsertButton();
	afx_msg void OnInsertStatic();
	afx_msg void OnInsertEdit();
	afx_msg void OnInsertProgress();
	afx_msg void OnInsertTrackbar();
	afx_msg void OnInsertScrollbar();
	afx_msg void OnInsertArea();
	afx_msg void OnInsertIconslot();
	afx_msg void OnInsertList();
	afx_msg void OnEditDuplicate();
	afx_msg void OnEditDelete();
	afx_msg void OnEditZorder();
	afx_msg void OnEditMakeGroup();
	afx_msg void OnEditParent();
	afx_msg void OnEditUnparent();
	afx_msg void OnEditMoveToLowest();
	afx_msg void OnEditMoveToLower();
	afx_msg void OnEditMoveToUpper();
	afx_msg void OnEditReplaceTex();
	afx_msg void OnInsertGroup();
	afx_msg void OnBatchToolChangeImagePath();
	afx_msg void OnBatchToolChangeFont();
	afx_msg void OnBatchToolGatherImageFileName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIEDOC_H__EE8E9346_D90D_48A7_89CE_DC0950272A0E__INCLUDED_)
