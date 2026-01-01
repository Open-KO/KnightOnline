#if !defined(AFX_TEXVIEWER_H__E7AB26A3_E760_421C_B0CE_E2118D0FB88A__INCLUDED_)
#define AFX_TEXVIEWER_H__E7AB26A3_E760_421C_B0CE_E2118D0FB88A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TexViewer.h : header file
//
#define UM_UPDATE_INFO	((WM_USER)+101)
/////////////////////////////////////////////////////////////////////////////
// CTexViewer window
#define MAX_IMAGETYPE		200
class CN3Texture;
class CTexViewer : public CWnd
{
// Construction
public:
	CTexViewer();

// Attributes
public:
	enum	eEDITMODE {EDITMODE_SELECT=0, EDITMODE_ZOOM, EDITMODE_HAND, NUM_EDITMODE};
	eEDITMODE		GetEditMode() const {return m_eEditMode;}
	CRect			GetSelectedRect() const {return m_rcSelectedRect;}
	CSize			GetTexSize() const {return m_TexSize;}
protected:
	CN3Texture*		m_pTex;				// texture
	CSize			m_TexSize;			// texture size
	float			m_fScale;			// 화면 배율
	CPoint			m_ptLeftTopInImage;	// 이 윈도우 좌측 상단에 보이는 texture의 좌측 상단 좌표
	eEDITMODE		m_eEditMode;		// 현재 어떤 편집 상황인지.(예, 영역선택, zoom in/out...)
	CRect			m_rcSelectedRect;	// 선택된 사각형(image좌표 기준)
	BOOL			m_bDrag;			// 드래그 중인가?
	CPen			m_WhiteDashPen;		// 흰색 점선
	CPoint			m_ptMouseOld;		// 마우스의 이전 지점 기억
	BOOL			m_bDeselect;		// deselect 할 것인가?
	CPoint			m_ptClickOffset;	// 선택 영역을 움직이려고 할때 click했을경우의
										// 선택창의 클릭지점 상대좌표(선택 영역lefttop 0,0 기준)(image pixel좌표계)
										
	enum	eDRAGTYPE {DRAGTYPE_NONE=0, DRAGTYPE_MOVE, DRAGTYPE_LEFT, DRAGTYPE_RIGHT,
					DRAGTYPE_TOP, DRAGTYPE_BOTTOM, DRAGTYPE_LEFTTOP, DRAGTYPE_RIGHTTOP,
					DRAGTYPE_LEFTBOTTOM, DRAGTYPE_RIGHTBOTTOM, DRAGTYPE_SELECT};
	eDRAGTYPE		m_eDragType;			// Drag 상태

	// 커서
	HCURSOR			m_hCursorSelect;
	HCURSOR			m_hCursorZoomIn;
	HCURSOR			m_hCursorZoomOut;
	HCURSOR			m_hCursorHand;

	HCURSOR			m_hCursorSizeAll;
	HCURSOR			m_hCursorSizeWE;
	HCURSOR			m_hCursorSizeNS;
	HCURSOR			m_hCursorSizeNWSE;
	HCURSOR			m_hCursorSizeNESW;

	// image type관련
	int				m_iImageTypeCount;								// 선택되어야 할 이미지 종류의 수
	CRect			m_ImageRects[MAX_IMAGETYPE];					// m_iImageTypeCount만큼의 ImageRect
	int				m_iCurSelectedImage;							// 현재 선택된 ImageType
// Operations
public:
	void			Release();
	BOOL			Zoom(BOOL bZoomIn);		// in : 확대, out : 축소
	BOOL			Zoom(float fScale);		// f배로 Zoom 하기 
	void			Render();				// texture render하기
	void			SetTexture(LPCTSTR pszFName);	// texture 지정
	eEDITMODE		SetEditMode(eEDITMODE eMode);	// mode 바꾸기 (zoom, hand, select) 실패하면 이전 mode를 돌려준다.
	void			SetLeftTopInImage(CPoint ptLeftTop);	// 이미지의 좌측 상단 좌표 바꾸기
	BOOL			GetSelectedUVRect(struct __FLOAT_RECT* pFRect) const;	// 현재 선택된 UV좌표 얻기
	void			SetSelectedUVRect(const struct __FLOAT_RECT* pFRect);	// 현재 선택된 UV좌표 넣기

	// image type관련
	void			SetImageTypeCount(int iCount) {m_iImageTypeCount = iCount;}	// image type 갯수 정하기
	BOOL			SetImageTypeIndex(int iIndex);	// zero base 선택된 image type정하기
	CRect			GetImageRect(int iIndex);
	BOOL			AutoMultiRectSelect(BOOL bHorizon, CString& strErrMsg);
protected:
	BOOL			ScreenToImage(POINT	*pPoint);	// screen좌표를 image좌표로
	BOOL			ScreenToImage(RECT* pRect);		// screen좌표를 image좌표로
	BOOL			ImageToScreen(POINT	*pPoint);	// image좌표를 screen좌표로
	BOOL			ImageToScreen(RECT* pRect);		// image좌표를 screen좌표로
	eDRAGTYPE		CheckDragType(CRect rcSel, CPoint point);
	void			ProcessDrag(CPoint point);		// 영역 변형일 경우 처리하는 루틴
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexViewer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTexViewer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTexViewer)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXVIEWER_H__E7AB26A3_E760_421C_B0CE_E2118D0FB88A__INCLUDED_)
