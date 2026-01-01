//
//	Class:		CButtonST
//
//	Compiler:	Visual C++
//	Tested on:	Visual C++ 5.0
//				Visual C++ 6.0
//
//	Version:	See GetVersionC() or GetVersionI()
//
//	Created:	xx/xxxx/1998
//	Updated:	14/June/2001
//
//	Author:		Davide Calabro'		davide_calabro@yahoo.com
//
#ifndef _BTNST_H
#define _BTNST_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// CBtnST.h : header file
//

// Comment this if you don't want that CButtonST hilights itself
// also when the window is inactive (like happens in Internet Explorer)
// #define ST_LIKEIE

// Return values
#ifndef BTNST_OK
#define BTNST_OK 0
#endif
#ifndef BTNST_INVALIDRESOURCE
#define BTNST_INVALIDRESOURCE 1
#endif
#ifndef BTNST_FAILEDMASK
#define BTNST_FAILEDMASK 2
#endif
#ifndef BTNST_INVALIDINDEX
#define BTNST_INVALIDINDEX 3
#endif

class CButtonST : public CButton
{
  public:
    CButtonST();
    ~CButtonST();

    enum
    {
        ST_ALIGN_HORIZ,
        ST_ALIGN_VERT,
        ST_ALIGN_HORIZ_RIGHT
    };

    enum
    {
        BTNST_COLOR_BK_IN = 0, // Background color when mouse is INside
        BTNST_COLOR_FG_IN,     // Text color when mouse is INside
        BTNST_COLOR_BK_OUT,    // Background color when mouse is OUTside
        BTNST_COLOR_FG_OUT,    // Text color when mouse is OUTside

        BTNST_MAX_COLORS
    };

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CButtonST)
  public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual BOOL PreTranslateMessage(MSG *pMsg);

  protected:
    virtual void PreSubclassWindow();
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

  public:
    DWORD SetAutoRepeat(BOOL bSet, DWORD dwMilliseconds = 100);
    DWORD SetURL(LPCTSTR lpszURL);
    DWORD GetColor(BYTE byColorIndex, COLORREF *crpColor);
    DWORD SetColor(BYTE byColorIndex, COLORREF crColor, BOOL bRepaint = TRUE);
    DWORD SetDefaultColors(BOOL bRepaint = TRUE);
    int GetCheck();
    void SetCheck(int nCheck, BOOL bRepaint = TRUE);
    void DrawTransparent(BOOL bRepaint = FALSE);

    BOOL GetDefault();

    void SetTooltipText(int nId, BOOL bActivate = TRUE);
    void SetTooltipText(LPCTSTR lpszText, BOOL bActivate = TRUE);
    void ActivateTooltip(BOOL bEnable = TRUE);

    BOOL SetBtnCursor(int nCursorId = NULL);

    void SetFlatFocus(BOOL bDrawFlatFocus, BOOL bRepaint = FALSE);

    void SetAlign(int nAlign);
    int GetAlign();

    void SetFlat(BOOL bState = TRUE);
    BOOL GetFlat();

    void DrawBorder(BOOL bEnable = TRUE);
    DWORD SetIcon(int nIconInId, int nIconOutId = NULL);
    DWORD SetIcon(HICON hIconIn, HICON hIconOut = NULL);

    DWORD SetBitmaps(int nBitmapIn, COLORREF crTransColorIn, int nBitmapOut = NULL, COLORREF crTransColorOut = 0);
    DWORD SetBitmaps(
        HBITMAP hBitmapIn, COLORREF crTransColorIn, HBITMAP hBitmapOut = NULL, COLORREF crTransColorOut = 0);

    static short GetVersionI()
    {
        return 32;
    }
    static LPCTSTR GetVersionC()
    {
        return (LPCTSTR) _T("3.2");
    }

  protected:
    //{{AFX_MSG(CButtonST)
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
    afx_msg void OnKillFocus(CWnd *pNewWnd);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSysColorChange();
    afx_msg BOOL OnClicked();
    afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
    afx_msg void OnEnable(BOOL bEnable);
    afx_msg void OnCancelMode();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    //}}AFX_MSG

    afx_msg HBRUSH CtlColor(CDC *pDC, UINT nCtlColor);
    virtual DWORD OnDrawBackground(CDC *pDC, LPCRECT pRect);
    virtual DWORD OnDrawBorder(CDC *pDC, LPCRECT pRect);

    BOOL m_bDrawTransparent;
    BOOL m_bMouseOnButton;
    BOOL m_bIsPressed;
    BOOL m_bIsFocused;
    BOOL m_bIsDisabled;
    COLORREF m_crColors[BTNST_MAX_COLORS];

  private:
    void CancelHover();
    void PrepareImageRect(
        BOOL bHasTitle, RECT *rpItem, CRect *rpTitle, BOOL bIsPressed, DWORD dwWidth, DWORD dwHeight, CRect *rpImage);
    HBITMAP CreateBitmapMask(HBITMAP hSourceBitmap, DWORD dwWidth, DWORD dwHeight, COLORREF crTransColor);
    void DrawTheBitmap(CDC *pDC, BOOL bHasTitle, RECT *rItem, CRect *rCaption, BOOL bIsPressed, BOOL bIsDisabled);
    void FreeResources(BOOL bCheckForNULL = TRUE);
    void DrawTheIcon(CDC *pDC, BOOL bHasTitle, RECT *rpItem, CRect *rpTitle, BOOL bIsPressed, BOOL bIsDisabled);
    void InitToolTip();
    void PaintBk(CDC *pDC);

    int m_nAlign;
    BOOL m_bDrawBorder;
    BOOL m_bIsFlat;
    BOOL m_bDrawFlatFocus;
    BOOL m_bAutoRepeat;
    HWND m_hWndAutoRepeat;
    UINT m_nMsgAutoRepeat;
    DWORD m_dwPeriodAutoRepeat;

    HCURSOR m_hCursor;
    CToolTipCtrl m_ToolTip;

#pragma pack(1)
    typedef struct _STRUCT_ICONS
    {
        HICON hIcon;    // Handle to icon
        DWORD dwWidth;  // Width of icon
        DWORD dwHeight; // Height of icon
    } STRUCT_ICONS;
#pragma pack()

#pragma pack(1)
    typedef struct _STRUCT_BITMAPS
    {
        HBITMAP hBitmap;        // Handle to bitmap
        DWORD dwWidth;          // Width of bitmap
        DWORD dwHeight;         // Height of bitmap
        HBITMAP hMask;          // Handle to mask bitmap
        COLORREF crTransparent; // Transparent color
    } STRUCT_BITMAPS;
#pragma pack()

    STRUCT_ICONS m_csIcons[2];
    STRUCT_BITMAPS m_csBitmaps[2];

    CDC m_dcBk;
    CBitmap m_bmpBk;
    CBitmap *m_pbmpOldBk;

    BOOL m_bIsDefault;
    BOOL m_bIsCheckBox;
    int m_nCheck;

    TCHAR m_szURL[_MAX_PATH];

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
