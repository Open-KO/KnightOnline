// HierarchyView.cpp : implementation of the CHierarchyView class
//

#include "stdafx.h"
#include "UIE.h"

#include "UIEDoc.h"
#include "HierarchyView.h"
#include "MainFrm.h"
#include "UIEView.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UITrackBar.h>
#include <N3Base/N3UIScrollBar.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView

IMPLEMENT_DYNCREATE(CHierarchyView, CTreeView)

BEGIN_MESSAGE_MAP(CHierarchyView, CTreeView)
	//{{AFX_MSG_MAP(CHierarchyView)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView construction/destruction

CHierarchyView::CHierarchyView()
{
	// TODO: add construction code here

}

CHierarchyView::~CHierarchyView()
{
}

BOOL CHierarchyView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT;

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView drawing

void CHierarchyView::OnDraw(CDC* pDC)
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


void CHierarchyView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	UpdateAllInfo();
}

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView diagnostics

#ifdef _DEBUG
void CHierarchyView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CHierarchyView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CUIEDoc* CHierarchyView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUIEDoc)));
	return (CUIEDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView message handlers
void CHierarchyView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UpdateAllInfo();	
}

void CHierarchyView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	if(nullptr == pNMTreeView->itemOld.hItem) return;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if(hItem == nullptr) return;

	CN3UIBase* pUI = (CN3UIBase*)GetTreeCtrl().GetItemData(hItem);
	if(0 == ::_IsKeyDown(VK_CONTROL))
		GetDocument()->SetSelectedUI(nullptr); // 컨트롤 키를 안눌렀으면.. 단독 셀렉트..
	GetDocument()->SetSelectedUI(pUI);
	
	*pResult = 0;
}

void CHierarchyView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CHierarchyView Operations

// 모든 tree item 정보 갱신하기
void CHierarchyView::UpdateAllInfo()
{
	GetTreeCtrl().DeleteAllItems();

	UpdateTreeItem(TVI_ROOT, GetDocument()->GetRootUI());
	GetTreeCtrl().Expand(TVI_ROOT, TVE_EXPAND);
	this->SelectObject(TVI_ROOT, GetDocument()->GetSelectedUI());
}

// tree item 정보 갱신하기
void CHierarchyView::UpdateTreeItem(HTREEITEM hParent, CN3UIBase *pUIBase)
{
	if(pUIBase == nullptr) return;
	eUI_TYPE eUIType = pUIBase->UIType();
	CString str;

	CN3UIBase* pParentUI = pUIBase->GetParent();
	switch(eUIType)
	{
	case UI_TYPE_BASE:		str = _T("Base");		break;
	case UI_TYPE_BUTTON:
		{
			str = _T("Button");
			if (pParentUI && UI_TYPE_SCROLLBAR == pParentUI->UIType())
			{	// 부모가 스크롤의 버튼일 경우
				if (CN3UIScrollBar::BTN_LEFTUP == pUIBase->GetReserved())
					str = _T("Left/Up Button");
				else if(CN3UIScrollBar::BTN_RIGHTDOWN == pUIBase->GetReserved())
					str = _T("Right/Down Button");
			}
		} break;
	case UI_TYPE_STATIC:	str = _T("Static");		break;
	case UI_TYPE_PROGRESS:	str = _T("Progress");	break;
	case UI_TYPE_IMAGE:
		{
			str = _T("Image");
			if (pParentUI == nullptr) break;
			if (UI_TYPE_IMAGE == pParentUI->UIType())
			{
				ASSERT(UISTYLE_IMAGE_ANIMATE & pParentUI->m_dwStyle);	// 부모는 반드시 Animate image여야 한다.
				str.Format("Image (%d)", pUIBase->GetReserved());
			}
			else if (UI_TYPE_BUTTON == pParentUI->UIType())
			{
				if (CN3UIButton::BS_NORMAL == pUIBase->GetReserved())		str = _T("Normal Image");
				else if (CN3UIButton::BS_DOWN == pUIBase->GetReserved())	str = _T("Down Image");
				else if (CN3UIButton::BS_ON == pUIBase->GetReserved())		str = _T("On Image");
				else if (CN3UIButton::BS_DISABLE == pUIBase->GetReserved())	str = _T("Disable Image");
			}
			else if (UI_TYPE_STATIC == pParentUI->UIType() || UI_TYPE_EDIT == pParentUI->UIType())
			{
				str = _T("Background Image");
			}
			else if (UI_TYPE_PROGRESS == pParentUI->UIType())
			{
				if (CN3UIProgress::IMAGETYPE_BKGND == pUIBase->GetReserved())		str = _T("Background Image");
				else if (CN3UIProgress::IMAGETYPE_FRGND == pUIBase->GetReserved())	str = _T("Foreground Image");
			}
			else if (UI_TYPE_TRACKBAR == pParentUI->UIType())
			{
				if (CN3UITrackBar::IMAGETYPE_BKGND == pUIBase->GetReserved())		str = _T("Background Image");
				else if (CN3UITrackBar::IMAGETYPE_THUMB == pUIBase->GetReserved())	str = _T("Thumb Image");
			}
			
			if (UISTYLE_IMAGE_ANIMATE & pUIBase->m_dwStyle) str = "(Animate)"+str;
		} break;
	case UI_TYPE_SCROLLBAR:	str = _T("Scrollbar");	break;
	case UI_TYPE_STRING:	str = _T("String");		break;
	case UI_TYPE_TRACKBAR:	str = _T("Trackbar");	break;
	case UI_TYPE_EDIT:		str = _T("Edit");		break;

	//cerberus 02, 01,02
	case UI_TYPE_AREA:		str = _T("Area");		break;
	case UI_TYPE_ICONSLOT:	str = _T("Icon Slot");	break;
	case UI_TYPE_LIST:		str = _T("List");		break;

	default:				str = _T("Unknown");	break;
	}
	
	str += " - ";
	str += pUIBase->m_szID.c_str(); // 이름을 붙여준다... !!

	HTREEITEM hItem = GetTreeCtrl().InsertItem(str, hParent);	// insert
	GetTreeCtrl().SetItemData(hItem, (DWORD_PTR) pUIBase);		// pointer 저장
	GetTreeCtrl().Expand(hItem, TVE_EXPAND);					// expand

	// update child
	for (CN3UIBase* pChild : pUIBase->m_Children)
		UpdateTreeItem(hItem, pChild);
}

// UIBase 포인터로 tree item 선택하기
void CHierarchyView::SelectObject(HTREEITEM hItem, CN3UIBase* pUIBase)
{
	if (pUIBase == nullptr || hItem == nullptr)
		return;

	if (hItem != TVI_ROOT)
	{
		if (GetTreeCtrl().GetItemData(hItem) == (DWORD_PTR) pUIBase)
		{
			GetTreeCtrl().SelectItem(hItem);
			return;
		}
	}

	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hItem);
	while (hChild != nullptr)
	{
		SelectObject(hChild, pUIBase);
		hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT);
	}
}

void CHierarchyView::OnKillFocus(CWnd* pNewWnd) 
{
	CTreeView::OnKillFocus(pNewWnd);
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->EnableAccelerator(FALSE);
}

void CHierarchyView::OnSetFocus(CWnd* pOldWnd) 
{
	CTreeView::OnSetFocus(pOldWnd);
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->EnableAccelerator(TRUE);
}
