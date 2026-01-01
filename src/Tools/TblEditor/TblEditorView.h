#pragma once

#include <vector>
#include <string>
#include <map>

#include "TblListCtrl.h"
#include "TblEditorBase.h"

class CTblEditorDoc;
class CTblEditorBase;
class CTblEditorView : public CFormView
{
protected: // create from serialization only
	CTblEditorView();
	DECLARE_DYNCREATE(CTblEditorView)

	void RefreshTable();
	CString DecodeField(const CStringA& fieldA, int iColNo) const;
	void BuildTableForSave(std::map<int, std::vector<CStringA>>& newRows);
	bool SaveTable(const CString& savePath);
	void SetCodePage(int codepage);

public:
	CTblEditorDoc* GetDocument();
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LRESULT OnListModified(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetEncoding_Korean();
	afx_msg void OnSetEncoding_EnglishUS();
	afx_msg void OnSetEncoding_Turkish();
	afx_msg void OnMenuExit();

	DECLARE_MESSAGE_MAP()

protected:
	CTblListCtrl	m_ListCtrl;
	int				m_iStringCodePage;

	// for editing
	int				m_iEditItem;
	int				m_iEditSubItem;
};
