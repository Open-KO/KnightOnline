#pragma once

#include <afxcmn.h>

constexpr int WM_USER_LIST_MODIFIED = WM_USER + 1;

class CTblEditorBase;
class CTblListCtrl : public CListCtrl
{
	DECLARE_MESSAGE_MAP()

	class CInPlaceEdit : public CEdit
	{
		DECLARE_MESSAGE_MAP()

	public:
		CInPlaceEdit()
			: m_iLeft(0)
		{
		}

		afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos)
		{
			lpwndpos->x = m_iLeft;
			CEdit::OnWindowPosChanging(lpwndpos);
		}

		inline void SetLeft(int left) {
			m_iLeft = left;
		}

	protected:
		int m_iLeft;
	};

public:
	CTblListCtrl();
	void AttachTbl(const CTblEditorBase* pTblBase);
	void DetachTbl();

	static void EscapeForDisplay(CString& text);
	static void UnescapeForSave(CString& text);

	bool Validate(CString& szNewText) const;
	void ReportValidationFailed(const TCHAR* szNewText) const;
	bool IsPrimaryKeyInUse(const CString& primaryKey) const;

protected:
	void Reset();

protected:
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnContextMenuInsertRow();
	afx_msg void OnContextMenuDeleteSelectedRow();

protected:
	CInPlaceEdit			m_Edit;
	const CTblEditorBase*	m_pTblBase;

	int						m_iItem;
	int						m_iSubItem;
};

