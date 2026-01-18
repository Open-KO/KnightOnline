#if !defined(AFX_DLGMAPVIEW_H__78BB52D1_A276_4F30_9B9D_7A1915985098__INCLUDED_)
#define AFX_DLGMAPVIEW_H__78BB52D1_A276_4F30_9B9D_7A1915985098__INCLUDED_

#pragma once

// DlgMapView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CDlgMapView dialog
#include <N3Base/BitMapFile.h>

class CBitMapFile;
class CMainFrame;
class CDlgMapView : public CDialog
{
	// Construction
public:
	CDlgMapView(CWnd* pParent = nullptr); // standard constructor

	void UpData(void);
	void ColorMapTemp(void);
	void SetMapView(void);
	void Tick(void);
	void Render(void);
	// Dialog Data
	//{{AFX_DATA(CDlgMapView)
	enum
	{
		IDD = IDD_ALL_MAP_VIEW
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMapView)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

public:
	BOOL Select_Map_Edit;
	CMainFrame* m_pFrame;
	CBitMapFile bmpf;

	RECT DragRect;
	POINT MousePoint;
	BOOL lbuttondown;
	CBitMapFile BMF;
	LPDIRECT3DDEVICE9 m_pD3DDevice;

	int Brush_Size;
	HBRUSH hbrush, OldBrush;
	HPEN PenRed, oldpen, PenBlue;

	int Map_View_Size;

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgMapView)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnMapUpdata();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMapViewCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAPVIEW_H__78BB52D1_A276_4F30_9B9D_7A1915985098__INCLUDED_)
