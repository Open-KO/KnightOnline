#ifndef _CBUTTONST_DEMODLG_H_
#define _CBUTTONST_DEMODLG_H_

#include "BtnST.h"
#include "HyperLink.h"
#include "MyTabCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCButtonST_DemoDlg : public CDialog
{
  public:
    CCButtonST_DemoDlg(CWnd *pParent = NULL);

    //{{AFX_DATA(CCButtonST_DemoDlg)
    enum
    {
        IDD = IDD_CBUTTONST_DEMO_DIALOG
    };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CCButtonST_DemoDlg)
  protected:
    virtual void DoDataExchange(CDataExchange *pDX);
    //}}AFX_VIRTUAL

  protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CCButtonST_DemoDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

  private:
    CButtonST m_btnClose;

    CMyTabCtrl m_tabMyTabCtrl;
    CHyperLink m_EMailLink;
    CHyperLink m_HomePageLink;

    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
