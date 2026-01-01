#ifndef _BKDIALOG_H_
#define _BKDIALOG_H_

#include "BackgroundUtil.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBkDialog dialog

class CBkDialog : public CDialog, public CBackgroundUtil
{
    // Construction
  public:
    CDC *m_pDC;
    ~CBkDialog();
    CBkDialog(LPCTSTR pszResourceID, CWnd *pParent = NULL);
    CBkDialog(UINT uResourceID, CWnd *pParent = NULL);
    CBkDialog(CWnd *pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CBkDialog)
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBkDialog)
  protected:
    virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support
                                                     //}}AFX_VIRTUAL

    // Implementation
  protected:
    // Generated message map functions
    //{{AFX_MSG(CBkDialog)
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
