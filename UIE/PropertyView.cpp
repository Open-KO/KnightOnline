// PropertyView.cpp : implementation file
//

#include "stdafx.h"
#include "UIE.h"
#include "PropertyView.h"
#include "HierarchyView.h"
#include "UIEDoc.h"
#include "MainFrm.h"
#include "DlgTexture.h"
#include "UIEView.h"
#include "DlgAnimate.h"

#include "..\Client\N3Base\N3UIBase.h"
#include "..\Client\N3Base\N3UIImage.h"
#include "..\Client\N3Base\N3UIString.h"
#include "..\Client\N3Base\N3UIButton.h"
#include "..\Client\N3Base\N3UIProgress.h"
#include "..\Client\N3Base\N3UITrackBar.h"
#include "..\Client\N3Base\N3UIScrollBar.h"
#include "..\Client\N3Base\N3UIEdit.h"
#include "..\Client\N3Base\N3UIArea.h"
#include "..\Client\N3Base\N3UIList.h"
#include "..\Client\N3Base\N3Texture.h"
#include "..\Client\N3Base\N3SndObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyView

IMPLEMENT_DYNCREATE(CPropertyView, CFormView)

CPropertyView::CPropertyView()
	: CFormView(CPropertyView::IDD)
{
	//{{AFX_DATA_INIT(CPropertyView)
	//}}AFX_DATA_INIT
}

CPropertyView::~CPropertyView()
{
}

void CPropertyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyView)
	DDX_Control(pDX, IDC_UI_STATIC, m_UIStatic);
	DDX_Control(pDX, IDC_UI_ICONSLOT, m_UIIconSlot);
	DDX_Control(pDX, IDC_UI_AREA, m_UIArea);
	DDX_Control(pDX, IDC_UI_EDIT, m_UIEdit);
	DDX_Control(pDX, IDC_UI_TRACKBAR, m_UITrackBar);
	DDX_Control(pDX, IDC_UI_SCROLLBAR, m_UIScrollBar);
	DDX_Control(pDX, IDC_UI_PROGRESS, m_UIProgress);
	DDX_Control(pDX, IDC_UI_STRING, m_UIString);
	DDX_Control(pDX, IDC_UI_IMAGE, m_UIImage);
	DDX_Control(pDX, IDC_UI_BUTTON, m_UIButton);
	DDX_Control(pDX, IDC_UI_BASE, m_UIBase);
	DDX_Control(pDX, IDC_UI_LIST, m_UIList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyView, CFormView)
	//{{AFX_MSG_MAP(CPropertyView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyView diagnostics

#ifdef _DEBUG
void CPropertyView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPropertyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CUIEDoc* CPropertyView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUIEDoc)));
	return (CUIEDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertyView message handlers

void CPropertyView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	static BOOL bInitCtrl = FALSE;
	if(FALSE == bInitCtrl)
	{
		bInitCtrl = TRUE;
		// base
		m_UIBase.AddPropItem("ID string", "", PIT_EDIT, "");				// m_iID
		m_UIBase.AddPropItem("Region left", "", PIT_BUTTON, "");		// m_rcRegion
		m_UIBase.AddPropItem("Region top", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("Region right", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("Region bottom", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("MoveRect left", "", PIT_BUTTON, "");	// m_rcMovable
		m_UIBase.AddPropItem("MoveRect top", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("MoveRect right", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("MoveRect bottom", "", PIT_BUTTON, "");
		m_UIBase.AddPropItem("Tooltip text", "", PIT_EDIT, "");		// m_pszTooltipText
		m_UIBase.AddPropItem("Open sound", "", PIT_FILE, "");		// sound
		m_UIBase.AddPropItem("Close sound", "", PIT_FILE, "");		// sound
		m_UIBase.AddPropItem("Delete sound", "", PIT_BUTTON, "");		// sound 설정 지우기
		m_UIBase.AddPropItem("Visible", "", PIT_BUTTON, "");		// visible

		// image
		m_UIImage.AddPropItem("Texture", "", PIT_FILE, "");			// texture
		m_UIImage.AddPropItem("UV left", "", PIT_BUTTON, "");			// m_frcUVRect (UV좌표)
		m_UIImage.AddPropItem("UV top", "", PIT_BUTTON, "");
		m_UIImage.AddPropItem("UV right", "", PIT_BUTTON, "");
		m_UIImage.AddPropItem("UV bottom", "", PIT_BUTTON, "");
		m_UIImage.AddPropItem("Color", "", PIT_COLOR, "");			// m_Color
		m_UIImage.AddPropItem("Animate Frame", "", PIT_EDIT, "");			// Animate frame 초당 몇 프레임으로 에니메이트 될 것인가?
		m_UIImage.AddPropItem("Make Animation", "", PIT_BUTTON, "");			// Animation으로 만들기

		// string
		m_UIString.AddPropItem("Line", "", PIT_COMBO, "SINGLE LINE|MULTI LINE|");		// style(line)
		m_UIString.AddPropItem("H align", "", PIT_COMBO, "LEFT|CENTER|RIGHT|");			// style(horizontal align)
		m_UIString.AddPropItem("V align", "", PIT_COMBO, "TOP|VCENTER|BOTTOM|");	// style(vertical align)
		m_UIString.AddPropItem("Font", "", PIT_BUTTON, "");			// font
		m_UIString.AddPropItem("Font size", "", PIT_BUTTON, "");		// font size
		m_UIString.AddPropItem("Font style", "", PIT_BUTTON, "");		// font style
		m_UIString.AddPropItem("Text Color", "", PIT_COLOR, "");			// string color
		m_UIString.AddPropItem("Text", "", PIT_EDIT, "");			// text

		// string
		m_UIList.AddPropItem("Font", "", PIT_BUTTON, "");			// font
		m_UIList.AddPropItem("Font size", "", PIT_BUTTON, "");		// font size
		m_UIList.AddPropItem("Font style", "", PIT_BUTTON, "");		// font style
		m_UIList.AddPropItem("Text Color", "", PIT_COLOR, "");		// string color

		// button
		m_UIButton.AddPropItem("Style","", PIT_COMBO, "Normal Button|Check Button|");
		m_UIButton.AddPropItem("ClickRect left","", PIT_BUTTON, "");
		m_UIButton.AddPropItem("ClickRect top","", PIT_BUTTON, "");
		m_UIButton.AddPropItem("ClickRect right","", PIT_BUTTON, "");
		m_UIButton.AddPropItem("ClickRect bottom","", PIT_BUTTON, "");
		m_UIButton.AddPropItem("On sound", "", PIT_FILE, "");		// sound
		m_UIButton.AddPropItem("Click sound", "", PIT_FILE, "");		// sound
		m_UIButton.AddPropItem("Delete sound", "", PIT_BUTTON, "");		// sound 설정 지우기

		// static
		m_UIStatic.AddPropItem("Click sound", "", PIT_FILE, "");		// sound
		m_UIStatic.AddPropItem("Delete sound", "", PIT_BUTTON, "");		// sound 설정 지우기
		m_UIStatic.AddPropItem("Delete bkgnd image", "", PIT_BUTTON,"");

		// edit
		m_UIEdit.AddPropItem("Style", "", PIT_COMBO,"Normal Edit|Password Edit|Number Only Edit|");
		m_UIEdit.AddPropItem("Typing sound", "", PIT_FILE, "");		// sound
		m_UIEdit.AddPropItem("Delete sound", "", PIT_BUTTON, "");		// sound 설정 지우기

		// progress
		m_UIProgress.AddPropItem("Style","", PIT_COMBO, "오른쪽 증가|왼쪽 증가|아래로 증가|위로 증가|");
		m_UIProgress.AddPropItem("MaxValue", "", PIT_EDIT, "");
		m_UIProgress.AddPropItem("MinValue", "", PIT_EDIT, "");
		m_UIProgress.AddPropItem("CurValue", "", PIT_EDIT, "");
		m_UIProgress.AddPropItem("Delete BkGnd", "click", PIT_BUTTON, "");	// 배경 이미지 지우는 버튼

		// trackbar
		m_UITrackBar.AddPropItem("Style","", PIT_COMBO, "가로|세로|");
		m_UITrackBar.AddPropItem("Max pos", "", PIT_EDIT, "");
		m_UITrackBar.AddPropItem("Min pos", "", PIT_EDIT, "");
		m_UITrackBar.AddPropItem("Cur pos", "", PIT_EDIT, "");
		m_UITrackBar.AddPropItem("Page size", "", PIT_EDIT, "");
		m_UITrackBar.AddPropItem("Delete bkgnd image", "", PIT_BUTTON,"");

		// scrollbar
		m_UIScrollBar.AddPropItem("Style","", PIT_COMBO, "가로|세로|");
		m_UIScrollBar.AddPropItem("Line size", "", PIT_EDIT, "");

		// area
		m_UIArea.AddPropItem("Type","", PIT_COMBO, "area_none|area_slot|area_inv|area_trade_npc|area_per_trade_my|\
area_per_trade_other|area_drop_item|area_skill_tree|area_skill_hotkey|area_repair_inv|area_repair_npc|area_trade_my|area_per_trade_inv|");

		// icon slot
		m_UIIconSlot.AddPropItem("Slot ID", "", PIT_EDIT, "");
	}
	UpdateInfo();
}

BOOL CPropertyView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	CMainFrame*			pFrm = (CMainFrame*)AfxGetMainWnd();
	CHierarchyView*		pHView = pFrm->GetHierarchyView();
	CN3UIBase* pUI = GetDocument()->GetSelectedUI();
	if(pUI == NULL) return TRUE;
	CPropertyItem* pItem = (CPropertyItem*)lParam;

	// base
	if((void*)wParam == (&m_UIBase))
	{
		if(pItem->m_propName == "ID string")
		{
			HTREEITEM hItem = pHView->GetTreeCtrl().GetSelectedItem();
			if(NULL != hItem)
			{
				CString str = pHView->GetTreeCtrl().GetItemText(hItem);
				int iTmp = str.Find(" - ");
				str = str.Left(iTmp);
				str += " - ";
				str += pItem->m_curValue;
				pHView->GetTreeCtrl().SetItemText(hItem, str);
			}
			pUI->SetID(pItem->m_curValue);
		}
		else if(pItem->m_propName == "Region left" || pItem->m_propName == "Region top" ||
			pItem->m_propName == "Region right" || pItem->m_propName == "Region bottom")
		{
			// region 갱신하는 함수 만들어서 처리하기
			pFrm->GetRightPane()->SelectRectType(CUIEView::RT_REGION);
		}
		else if(pItem->m_propName == "MoveRect left" || pItem->m_propName == "MoveRect top" ||
			pItem->m_propName == "MoveRect right" || pItem->m_propName == "MoveRect bottom")
		{
			// move rect 갱신하는 함수 만들어서 처리하기
			pFrm->GetRightPane()->SelectRectType(CUIEView::RT_MOVE);
		}
		else if(pItem->m_propName == "Tooltip text") pUI->SetTooltipText(pItem->m_curValue);
		else if(pItem->m_propName == "Open sound")
		{
			pUI->SetSndOpen((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pUI->GetSndFName_OpenUI().c_str();
		}
		else if(pItem->m_propName == "Close sound")
		{
			pUI->SetSndClose((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pUI->GetSndFName_CloseUI().c_str();
		}
		else if(pItem->m_propName == "Delete sound")
		{
			pUI->SetSndOpen("");
			pUI->SetSndClose("");
			CPropertyItem* pTmp = NULL;
			pTmp = m_UIBase.GetPropItem("Open sound");
			if (pTmp) pTmp->m_curValue = "";
			pTmp = m_UIBase.GetPropItem("Close sound");
			if (pTmp) pTmp->m_curValue = "";
			Invalidate();
		}
		else if(pItem->m_propName == "Visible")
		{
			pUI->SetVisible(!pUI->IsVisible());	// 보였다 안보였다하게 하기
			GetDocument()->UpdateAllViews(NULL);
		}
	}
	// image
	if((void*)wParam == (&m_UIImage) && UI_TYPE_IMAGE == pUI->UIType())
	{
		CN3UIImage* pImage = (CN3UIImage*)pUI;
		if (pItem->m_propName == "Texture")
		{
			CN3BaseFileAccess tmpBase;
			tmpBase.FileNameSet((LPCTSTR)pItem->m_curValue);	// Base경로에 대해서 상대적 경로를 넘겨준다.

			pImage->SetTex(tmpBase.FileName());
			pItem->m_curValue = tmpBase.FileName().c_str();	//tex file name 다시 설정
			m_UIImage.Invalidate();
		}
		else if (pItem->m_propName == "UV left" || pItem->m_propName == "UV top" ||
			pItem->m_propName == "UV right" || pItem->m_propName == "UV bottom")
		{
			// UV 설정하는 함수 만들어서 처리하기
			CN3Texture* pTex = pImage->GetTex();
			if (pTex && pTex->FileName().size() > 0)
			{
				CDlgTexture dlg;
				dlg.SetTexture(pTex->FileName().c_str());
				dlg.SetSelectedUVRect(pImage->GetUVRect());
				if (IDOK == dlg.DoModal())
				{
					__FLOAT_RECT frc;
					if (dlg.GetSelectedUVRect(&frc))
					{
						pImage->SetUVRect(frc.left, frc.top, frc.right, frc.bottom);
						UpdateUIImageInfo();
					}
				}

				// 예외상황 : 부모가 Progress일때는 아래 함수를 불러줘야 한다.
				CN3UIBase* pParent = pUI->GetParent();
				if (pParent && UI_TYPE_PROGRESS == pParent->UIType()) ((CN3UIProgress*)pParent)->SetFrGndUVFromFrGndImage();
			}
			else AfxGetMainWnd()->MessageBox("You should set texture file");
		}
		else if (pItem->m_propName == "Color") pImage->SetColor(pItem->D3DColorGet());
		else if (pItem->m_propName == "Animate Frame") pImage->m_fAnimFrame = atof(pItem->m_curValue);
		else if (pItem->m_propName == "Make Animation")
		{	// Animation image로 만들기
			CN3UIBase* pParentUI = pImage->GetParent();
			while(1)
			{
				if (NULL == pParentUI || (UI_TYPE_IMAGE == pParentUI->UIType()))
				{	// 반드시 부모는 image가 아니어야 한다.
					pFrm->MessageBox("Animate image의 자식 image는 animate image로 만들 수 없습니다.");
					break;	
				}
				CDlgAnimate dlgAnim;
				if (IDCANCEL == dlgAnim.DoModal())	break;// animate되는 그림이 몇장인가?
				pImage->SetAnimImage(dlgAnim.m_iCount);	// 장수 세팅
				if (dlgAnim.m_iCount<=0)	break; // 1장 이상이면 texture와 uv좌표 세팅
				char szTexFName[_MAX_PATH];
				if (FALSE == SelectTexture(szTexFName))	break;// texture이름 정하기
				// 여러장의 이미지 세팅하게 하기
				CDlgTexture	dlgTex;
				dlgTex.SetTexture(szTexFName);
				char szNames[1000][20];
				char* szImageTypeNames[1000];
				int i;
				for (i=0; i<dlgAnim.m_iCount; ++i)
				{
					wsprintf(szNames[i], "%d", i);
					szImageTypeNames[i] = szNames[i];
				}
				dlgTex.SetImageTypes(dlgAnim.m_iCount, szImageTypeNames);
				if (IDCANCEL == dlgTex.DoModal()) break;	// 취소의 경우
				CN3UIImage* pChildImage;
				for (i=0; i<dlgAnim.m_iCount; ++i)
				{
					__FLOAT_RECT frcUV;
					dlgTex.GetImageRect(i, &frcUV);
					pChildImage = pImage->GetChildImage(i);
					if (NULL == pChildImage) continue;
					pChildImage->SetTex(szTexFName);	// texture설정
					pChildImage->SetUVRect(frcUV.left, frcUV.top, frcUV.right, frcUV.bottom);		// uv 설정
				}
				// 위치 설정
				CRect rcRegion = dlgTex.GetImageRect(0);
				if (-1 == rcRegion.left) break;			// 영역이 비정상적이면 그냥 넘어간다.
				rcRegion.OffsetRect(-rcRegion.TopLeft());
				pImage->SetRegion(rcRegion);
				pFrm->GetRightPane()->SelectRectType(CUIEView::RT_REGION);
				GetDocument()->UpdateAllViews(NULL);
				break;
			}
		}
	}
	// string
	if((void*)wParam == (&m_UIString) && UI_TYPE_STRING == pUI->UIType())
	{
		CN3UIString* pString = (CN3UIString*)pUI;
		if (pItem->m_propName == "Line")
		{
			DWORD dwStyle = pString->m_dwStyle;
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_SINGLELINE;
			else if (1 == pItem->m_crColor) dwStyle &= (~UISTYLE_STRING_SINGLELINE);
			pString->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "H align")
		{
			DWORD dwStyle = pString->m_dwStyle;
			dwStyle &= (~UISTYLE_STRING_ALIGNLEFT);
			dwStyle &= (~UISTYLE_STRING_ALIGNCENTER);
			dwStyle &= (~UISTYLE_STRING_ALIGNRIGHT);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNLEFT;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNCENTER;
			else if (2 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNRIGHT;
			pString->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "V align")
		{
			DWORD dwStyle = pString->m_dwStyle;
			dwStyle &= (~UISTYLE_STRING_ALIGNTOP);
			dwStyle &= (~UISTYLE_STRING_ALIGNVCENTER);
			dwStyle &= (~UISTYLE_STRING_ALIGNBOTTOM);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNTOP;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNVCENTER;
			else if (2 == pItem->m_crColor) dwStyle |= UISTYLE_STRING_ALIGNBOTTOM;
			pString->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "Font" || pItem->m_propName == "Font size" || pItem->m_propName == "Font style")
		{
			// font 정하는 루틴
			LOGFONT logfont;	ZeroMemory(&logfont, sizeof(logfont));
			CDFont* pDFont = pString->m_pDFont;
			if (pDFont)
			{
				logfont.lfHeight = pDFont->GetFontHeightInLogicalUnit();
				const std::string strFontName(pDFont->GetFontName());
				lstrcpy(logfont.lfFaceName, strFontName.c_str());
				DWORD dwFontFlags = pDFont->GetFontFlags();
				logfont.lfItalic = (dwFontFlags & D3DFONT_ITALIC) ? TRUE:FALSE;
				logfont.lfWeight = (dwFontFlags & D3DFONT_BOLD) ? FW_BOLD:FW_NORMAL;
			}
			CFontDialog dlg(&logfont);
			if (IDOK == dlg.DoModal())
			{
				__ASSERT(dlg.GetSize()>0, "font height가 0보다 작습니다.");
				const std::string strFontName(dlg.GetFaceName());
				pString->SetFont( strFontName, dlg.GetSize()/10, dlg.IsBold(), dlg.IsItalic());
				UpdateUIStringInfo();	// string 정보 다시 갱신하기
			}
		}
		else if (pItem->m_propName == "Text Color") pString->SetColor(pItem->D3DColorGet());
		else if (pItem->m_propName == "Text") pString->SetString((LPCTSTR)pItem->m_curValue);
	}
	// List
	if((void*)wParam == (&m_UIList) && UI_TYPE_LIST == pUI->UIType())
	{
		CN3UIList* pUIList = (CN3UIList*)pUI;
		if (pItem->m_propName == "Font" || pItem->m_propName == "Font size" || pItem->m_propName == "Font style")
		{
			// font 정하는 루틴
			LOGFONT logfont;	ZeroMemory(&logfont, sizeof(logfont));
			logfont.lfHeight = 0; // pUIList->FontHeightInLogicalUnit();
			lstrcpy(logfont.lfFaceName, pUIList->FontName().c_str());
			logfont.lfItalic = pUIList->FontIsItalic();
			logfont.lfWeight = pUIList->FontIsBold();

			CFontDialog dlg(&logfont);
			if (IDOK == dlg.DoModal())
			{
				__ASSERT(dlg.GetSize()>0, "font height가 0보다 작습니다.");
				std::string szFontName = std::string(CT2A(dlg.GetFaceName()));
				pUIList->SetFont( szFontName, dlg.GetSize()/10, dlg.IsBold(), dlg.IsItalic());
				UpdateUIListInfo();	// List 정보 다시 갱신하기
			}
		}
		else if (pItem->m_propName == "Text Color") pUIList->SetFontColor(pItem->D3DColorGet());
	}
	// button
	if((void*)wParam == (&m_UIButton) && UI_TYPE_BUTTON == pUI->UIType())
	{
		CN3UIButton* pBtn = (CN3UIButton*)pUI;
		if (pItem->m_propName == "Style")
		{
			DWORD dwStyle = pBtn->m_dwStyle;
			dwStyle &= (~UISTYLE_BTN_NORMAL);
			dwStyle &= (~UISTYLE_BTN_CHECK);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_BTN_NORMAL;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_BTN_CHECK;
			pBtn->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "ClickRect left" || pItem->m_propName == "ClickRect top" ||
			pItem->m_propName == "ClickRect right" || pItem->m_propName == "ClickRect bottom")
		{
			// 클릭 영역 지정하는 코드 넣기
			pFrm->GetRightPane()->SelectRectType(CUIEView::RT_CLICK);
		}
		else if (pItem->m_propName == "On sound")
		{
			pBtn->SetSndOn((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pBtn->GetSndFName_On().c_str();
		}
		else if (pItem->m_propName == "Click sound")
		{
			pBtn->SetSndClick((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pBtn->GetSndFName_Click().c_str();
		}
		else if(pItem->m_propName == "Delete sound")
		{
			pBtn->SetSndOn("");
			pBtn->SetSndClick("");
			CPropertyItem* pTmp = NULL;
			pTmp = m_UIButton.GetPropItem("On sound");
			if (pTmp) pTmp->m_curValue = "";
			pTmp = m_UIButton.GetPropItem("Click sound");
			if (pTmp) pTmp->m_curValue = "";
			Invalidate();
		}
	}
	// static 
	if((void*)wParam == (&m_UIStatic) && (UI_TYPE_STATIC == pUI->UIType() || UI_TYPE_EDIT == pUI->UIType()))
	{
		CN3UIStatic* pStatic = (CN3UIStatic*)pUI;
		if (pItem->m_propName == "Click sound")
		{
			pStatic->SetSndClick((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pStatic->GetSndFName_Click().c_str();
		}
		else if (pItem->m_propName == "Delete bkgnd image")
		{
			pStatic->DeleteImage();
			GetDocument()->UpdateAllViews(NULL);
		}
		else if(pItem->m_propName == "Delete sound")
		{
			pStatic->SetSndClick("");
			CPropertyItem* pTmp = NULL;
			pTmp = m_UIStatic.GetPropItem("Click sound");
			if (pTmp) pTmp->m_curValue = "";
			Invalidate();
		}
	}

	// edit
	if((void*)wParam == (&m_UIEdit) && UI_TYPE_EDIT == pUI->UIType())
	{
		CN3UIEdit* pEdit = (CN3UIEdit*)pUI;
		if (pItem->m_propName == "Style")
		{
			DWORD dwStyle = pEdit->m_dwStyle;
			dwStyle &= (~UISTYLE_EDIT_PASSWORD);
			dwStyle &= (~UISTYLE_EDIT_NUMBERONLY);
			if (1 == pItem->m_crColor) dwStyle |= UISTYLE_EDIT_PASSWORD;
			if (2 == pItem->m_crColor) dwStyle |= UISTYLE_EDIT_NUMBERONLY;
			pEdit->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "Typing sound")
		{
			pEdit->SetSndTyping((LPCTSTR)pItem->m_curValue);
			pItem->m_curValue = pEdit->GetSndFName_Typing().c_str();
		}
		else if (pItem->m_propName == "Delete bkgnd image")
		{
			pEdit->DeleteImage();
			GetDocument()->UpdateAllViews(NULL);
		}
		else if(pItem->m_propName == "Delete sound")
		{
			pEdit->SetSndTyping("");
			CPropertyItem* pTmp = NULL;
			pTmp = m_UIEdit.GetPropItem("Typing sound");
			if (pTmp) pTmp->m_curValue = "";
			Invalidate();
		}
	}
	// area
	if((void*)wParam == (&m_UIArea) && UI_TYPE_AREA == pUI->UIType())
	{
		CN3UIArea* pArea = (CN3UIArea*)pUI;
		if (pItem->m_propName == "Type")
		{
			if ( lstrcmpi(pItem->m_curValue, "AREA_NONE") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_NONE;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_SLOT") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_SLOT;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_INV") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_INV;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_TRADE_NPC") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_TRADE_NPC;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_PER_TRADE_MY") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_PER_TRADE_MY;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_PER_TRADE_OTHER") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_PER_TRADE_OTHER;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_DROP_ITEM") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_DROP_ITEM;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_SKILL_TREE") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_SKILL_TREE;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_SKILL_HOTKEY") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_SKILL_HOTKEY;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_REPAIR_INV") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_REPAIR_INV;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_REPAIR_NPC") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_REPAIR_NPC;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_TRADE_MY") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_TRADE_MY;
			else if ( lstrcmpi(pItem->m_curValue, "AREA_PER_TRADE_INV") == 0 )
				pArea->m_eAreaType = UI_AREA_TYPE_PER_TRADE_INV;
		}
	}
	// progress
	if((void*)wParam == (&m_UIProgress) && UI_TYPE_PROGRESS == pUI->UIType())
	{
		CN3UIProgress* pProg = (CN3UIProgress*)pUI;
		if (pItem->m_propName == "Style")
		{
			DWORD dwStyle = pProg->m_dwStyle;
			dwStyle &= (~UISTYLE_PROGRESS_LEFT2RIGHT);
			dwStyle &= (~UISTYLE_PROGRESS_RIGHT2LEFT);
			dwStyle &= (~UISTYLE_PROGRESS_TOP2BOTTOM);
			dwStyle &= (~UISTYLE_PROGRESS_BOTTOM2TOP);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_PROGRESS_LEFT2RIGHT;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_PROGRESS_RIGHT2LEFT;
			else if (2 == pItem->m_crColor) dwStyle |= UISTYLE_PROGRESS_TOP2BOTTOM;
			else if (3 == pItem->m_crColor) dwStyle |= UISTYLE_PROGRESS_BOTTOM2TOP;
			pProg->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "MaxValue") pProg->SetRange(pProg->GetMin(), atoi(pItem->m_curValue));
		else if (pItem->m_propName == "MinValue") pProg->SetRange(atoi(pItem->m_curValue), pProg->GetMax());
		else if (pItem->m_propName == "CurValue") pProg->SetCurValue(atoi(pItem->m_curValue));
		else if (pItem->m_propName == "Delete BkGnd")
		{
			pProg->DeleteBkGndImage();
			GetDocument()->UpdateAllViews(NULL);
		}
	}
	// trackbar
	if((void*)wParam == (&m_UITrackBar) && UI_TYPE_TRACKBAR == pUI->UIType())
	{
		CN3UITrackBar* pTB = (CN3UITrackBar*)pUI;
		if (pItem->m_propName == "Style")
		{
			DWORD dwStyle = pTB->m_dwStyle;
			dwStyle &= (~UISTYLE_TRACKBAR_HORIZONTAL);
			dwStyle &= (~UISTYLE_TRACKBAR_VERTICAL);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_TRACKBAR_HORIZONTAL;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_TRACKBAR_VERTICAL;
			pTB->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "Max pos") pTB->SetRangeMax(atoi(pItem->m_curValue));
		else if (pItem->m_propName == "Min pos") pTB->SetRangeMin(atoi(pItem->m_curValue));
		else if (pItem->m_propName == "Cur pos") pTB->SetCurrentPos(atoi(pItem->m_curValue));
		else if (pItem->m_propName == "Page size") pTB->SetPageSize(atoi(pItem->m_curValue));
		else if (pItem->m_propName == "Delete bkgnd image")
		{
			pTB->DeleteBkImage();
			GetDocument()->UpdateAllViews(NULL);
		}
	}
	//scrollbar
	if((void*)wParam == (&m_UIScrollBar) && UI_TYPE_SCROLLBAR == pUI->UIType())
	{
		CN3UIScrollBar* pSB = (CN3UIScrollBar*)pUI;
		if (pItem->m_propName == "Style")
		{
			DWORD dwStyle = pSB->m_dwStyle;
			dwStyle &= (~UISTYLE_SCROLLBAR_HORIZONTAL);
			dwStyle &= (~UISTYLE_SCROLLBAR_VERTICAL);
			if (0 == pItem->m_crColor) dwStyle |= UISTYLE_SCROLLBAR_HORIZONTAL;
			else if (1 == pItem->m_crColor) dwStyle |= UISTYLE_SCROLLBAR_VERTICAL;
			pSB->SetStyle(dwStyle);
		}
		else if (pItem->m_propName == "Line size") pSB->SetLineSize(atoi(pItem->m_curValue));
	}
	// Icon slot
	if((void*)wParam == (&m_UIIconSlot) && UI_TYPE_ICONSLOT == pUI->UIType())
	{
		if (pItem->m_propName == "Slot ID")
		{
			pUI->SetReserved(atoi(pItem->m_curValue));
		}
	}

	GetDocument()->SetModifiedFlag();
	pFrm->GetRightPane()->Invalidate();
	return TRUE;
//	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CPropertyView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UpdateInfo();	
}

void CPropertyView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	if (m_UIBase.GetSafeHwnd() ) Resize();
	
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyView Operations
void CPropertyView::UpdateInfo()
{
	if(m_UIBase.GetSafeHwnd() == NULL) return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();

	// 일단 모두 감추기
	m_UIBase.ShowWindow(SW_HIDE);
	m_UIImage.ShowWindow(SW_HIDE);
	m_UIString.ShowWindow(SW_HIDE);
	m_UIButton.ShowWindow(SW_HIDE);
	m_UIStatic.ShowWindow(SW_HIDE);
	m_UIEdit.ShowWindow(SW_HIDE);
	m_UITrackBar.ShowWindow(SW_HIDE);
	m_UIScrollBar.ShowWindow(SW_HIDE);
	m_UIProgress.ShowWindow(SW_HIDE);
	m_UIArea.ShowWindow(SW_HIDE);
	m_UIIconSlot.ShowWindow(SW_HIDE);
	m_UIList.ShowWindow(SW_HIDE);
	if (NULL == pUIBase)	return;	// 선택된 UI가 없으므로 그냥 리턴

	CPoint ptWndPos = -GetScrollPosition();
	// base의 값 보이기(base는 일단 모두 가지고 있는 값이므로 표시해준다.)
	
	UpdateUIBaseInfo();					// 정보 갱신

	// image
	UpdateUIImageInfo();					// 정보 갱신
	// string
	UpdateUIStringInfo();					// 정보 갱신
	// button
	UpdateUIButtonInfo();					// 정보 갱신
	// static
	UpdateUIStaticInfo();					// 정보 갱신
	// edit
	UpdateUIEditInfo();						// 정보 갱신
	// progress
	UpdateUIProgressInfo();					// 정보 갱신
	// trackbar
	UpdateUITrackBarInfo();					// 정보 갱신
	// scrollbar
	UpdateUIScrollBarInfo();				// 정보 갱신
	// area
	UpdateUIAreaInfo();						// 정보 갱신
	// icon slot
	UpdateUIIconSlotInfo();					// 정보 갱신
	// List
	UpdateUIListInfo();						// 정보 갱신
	
	Resize();
}

void CPropertyView::Resize()
{
	CPoint ptWndPos = -GetScrollPosition();
	if (m_UIBase.IsWindowVisible()) SetPropertyListPos(&m_UIBase, ptWndPos);	// Property list window 위치조정
	if (m_UIImage.IsWindowVisible()) SetPropertyListPos(&m_UIImage, ptWndPos);	// Property list window 위치조정
	if (m_UIString.IsWindowVisible()) SetPropertyListPos(&m_UIString, ptWndPos);	// Property list window 위치조정
	if (m_UIButton.IsWindowVisible()) SetPropertyListPos(&m_UIButton, ptWndPos);	// Property list window 위치조정
	if (m_UIStatic.IsWindowVisible()) SetPropertyListPos(&m_UIStatic, ptWndPos);	// Property list window 위치조정
	if (m_UIEdit.IsWindowVisible()) SetPropertyListPos(&m_UIEdit, ptWndPos);	// Property list window 위치조정
	if (m_UIProgress.IsWindowVisible()) SetPropertyListPos(&m_UIProgress, ptWndPos);	// Property list window 위치조정
	if (m_UITrackBar.IsWindowVisible()) SetPropertyListPos(&m_UITrackBar, ptWndPos);	// Property list window 위치조정
	if (m_UIScrollBar.IsWindowVisible()) SetPropertyListPos(&m_UIScrollBar, ptWndPos);	// Property list window 위치조정
	if (m_UIArea.IsWindowVisible()) SetPropertyListPos(&m_UIArea, ptWndPos);	// Property list window 위치조정
	if (m_UIIconSlot.IsWindowVisible()) SetPropertyListPos(&m_UIIconSlot, ptWndPos);
	if (m_UIList.IsWindowVisible()) SetPropertyListPos(&m_UIList, ptWndPos);
}

void CPropertyView::SetPropertyListPos(CPropertyList* pList, CPoint& ptWndPos)
{
	if (NULL == pList) return;
	// 위치 조정 및 보이게 하기
	CRect rc;
	GetClientRect(&rc);
	int iHeight = pList->GetItemHeight(0) * pList->GetCount() + 4;
	pList->SetWindowPos(NULL, ptWndPos.x, ptWndPos.y, rc.Width(), iHeight, SWP_NOZORDER|SWP_SHOWWINDOW);
	ptWndPos.y += iHeight + 3;
}

// base
void CPropertyView::UpdateUIBaseInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	m_UIBase.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIBase.GetPropItem("ID string");							// id
	if (pItem) pItem->m_curValue = pUIBase->m_szID.c_str();

	pItem = m_UIBase.GetPropItem("Region left");					// region
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcRegion.left);
	pItem = m_UIBase.GetPropItem("Region top");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcRegion.top);
	pItem = m_UIBase.GetPropItem("Region right");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcRegion.right);
	pItem = m_UIBase.GetPropItem("Region bottom");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcRegion.bottom);

	pItem = m_UIBase.GetPropItem("MoveRect left");					// region
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcMovable.left);
	pItem = m_UIBase.GetPropItem("MoveRect top");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcMovable.top);
	pItem = m_UIBase.GetPropItem("MoveRect right");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcMovable.right);
	pItem = m_UIBase.GetPropItem("MoveRect bottom");
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->m_rcMovable.bottom);

	pItem = m_UIBase.GetPropItem("Tooltip text");					// tooltip text
	if (pItem) pItem->m_curValue = pUIBase->m_szToolTip.c_str();

	pItem = m_UIBase.GetPropItem("Open sound");					// open sound
	if (pItem) pItem->m_curValue = pUIBase->GetSndFName_OpenUI().c_str();

	pItem = m_UIBase.GetPropItem("Close sound");					// close sound
	if (pItem) pItem->m_curValue = pUIBase->GetSndFName_CloseUI().c_str();

	pItem = m_UIBase.GetPropItem("Visible");					// visible
	if (pItem)
	{
		if (pUIBase->IsVisible()) pItem->m_curValue = _T("Show");
		else pItem->m_curValue = _T("Hide");
	}

	m_UIBase.Invalidate();
}

// image
void CPropertyView::UpdateUIImageInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIImage* pUI = NULL;
	if (UI_TYPE_IMAGE == pUIBase->UIType()) pUI = (CN3UIImage*)pUIBase;
	else return;
	m_UIImage.ShowWindow(SW_SHOW);
	
	CPropertyItem* pItem = NULL;
	pItem = m_UIImage.GetPropItem("Texture");							// texture 이름
	if (pItem)
	{
		pItem->m_curValue = pUI->GetTexFN().c_str();
		CN3Texture* pTex = pUI->GetTex();
		if (pTex) pItem->m_curValue = pTex->FileName().c_str();
		else pItem->m_curValue += " : Load fail.";
	}
	pItem = m_UIImage.GetPropItem("UV left");							// m_frcUVRect (UV좌표)
	if (pItem) pItem->m_curValue.Format("%f", pUI->m_frcUVRect.left);
	pItem = m_UIImage.GetPropItem("UV top");
	if (pItem) pItem->m_curValue.Format("%f", pUI->m_frcUVRect.top);
	pItem = m_UIImage.GetPropItem("UV right");
	if (pItem) pItem->m_curValue.Format("%f", pUI->m_frcUVRect.right);
	pItem = m_UIImage.GetPropItem("UV bottom");
	if (pItem) pItem->m_curValue.Format("%f", pUI->m_frcUVRect.bottom);

	pItem = m_UIImage.GetPropItem("Color");							// m_Color
	if (pItem) pItem->D3DColorSet(pUI->m_Color);

	pItem = m_UIImage.GetPropItem("Animate Frame");
	if (pItem) pItem->m_curValue.Format("%f", pUI->m_fAnimFrame);

	pItem = m_UIImage.GetPropItem("Make Animation");				// animate
	if (pItem)
	{
		if ( UISTYLE_IMAGE_ANIMATE & pUI->m_dwStyle) pItem->m_curValue = "Animate Image";
		else pItem->m_curValue = "Normal Image";
	}
	
	m_UIImage.Invalidate();
}

// string
void CPropertyView::UpdateUIStringInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIString* pUI = NULL;
	if (UI_TYPE_STRING == pUIBase->UIType()) pUI = (CN3UIString*)pUIBase;
	else return;
	m_UIString.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;

	pItem = m_UIString.GetPropItem("Line");							// Line
	if (pItem)
	{
		if (UISTYLE_STRING_SINGLELINE & pUI->m_dwStyle)	pItem->m_crColor = 0;
		else pItem->m_crColor = 1;
	}
	pItem = m_UIString.GetPropItem("H align");							// H align
	if (pItem)
	{
		if (UISTYLE_STRING_ALIGNCENTER & pUI->m_dwStyle)	pItem->m_crColor = 1;
		else if (UISTYLE_STRING_ALIGNRIGHT & pUI->m_dwStyle)	pItem->m_crColor = 2;
		else pItem->m_crColor = 0;
	}
	pItem = m_UIString.GetPropItem("V align");							// V align
	if (pItem)
	{
		if (UISTYLE_STRING_ALIGNVCENTER & pUI->m_dwStyle)	pItem->m_crColor = 1;
		else if (UISTYLE_STRING_ALIGNBOTTOM & pUI->m_dwStyle)	pItem->m_crColor = 2;
		else pItem->m_crColor = 0;
	}
	pItem = m_UIString.GetPropItem("Font");							// font
	if (pItem)
	{
		std::string str;
		str = pUI->m_pDFont->GetFontName();
		if ( pUI->m_pDFont) pItem->m_curValue = str.c_str();
		else pItem->m_curValue = _T("");
	}
	pItem = m_UIString.GetPropItem("Font size");							// font size
	if (pItem)
	{
		if ( pUI->m_pDFont) pItem->m_curValue.Format("%d", pUI->m_pDFont->GetFontHeight() );
		else pItem->m_curValue = _T("");
	}
	pItem = m_UIString.GetPropItem("Font style");							// font style
	if (pItem)
	{
		pItem->m_curValue = _T("");
		if ( pUI->m_pDFont)
		{
			DWORD dwFlag = pUI->m_pDFont->GetFontFlags();
			if (D3DFONT_BOLD & dwFlag)	pItem->m_curValue += "굵게 ";
			if (D3DFONT_ITALIC & dwFlag)pItem->m_curValue += "기울임꼴";
		}
		if (pItem->m_curValue.GetLength()==0) pItem->m_curValue = _T("보통");
	}
	pItem = m_UIString.GetPropItem("Text Color");					// string color
	if (pItem) pItem->D3DColorSet(pUI->m_Color);

	pItem = m_UIString.GetPropItem("Text");							// text
	if (pItem) pItem->m_curValue = pUI->GetString().c_str();

	m_UIString.Invalidate();
}

void CPropertyView::UpdateUIListInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIList* pUI = NULL;
	if (UI_TYPE_LIST == pUIBase->UIType()) pUI = (CN3UIList*)pUIBase;
	else return;
	m_UIList.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;

	pItem = m_UIList.GetPropItem("Font");							// font
	if (pItem)
	{
		std::string str;
		str = pUI->FontName();
		pItem->m_curValue = pUI->FontName().c_str();
	}
	pItem = m_UIList.GetPropItem("Font size");							// font size
	if (pItem)
	{
		pItem->m_curValue.Format("%d", pUI->FontHeight() );
	}
	pItem = m_UIList.GetPropItem("Font style");							// font style
	if (pItem)
	{
		pItem->m_curValue = _T("");
		if (pUI->FontIsBold())	pItem->m_curValue += "굵게 ";
		if (pUI->FontIsItalic()) pItem->m_curValue += "기울임꼴";
		if (pItem->m_curValue.GetLength()==0) pItem->m_curValue = _T("보통");
	}
	pItem = m_UIList.GetPropItem("Text Color");					// string color
	if (pItem) pItem->D3DColorSet(pUI->FontColor());

	m_UIList.Invalidate();
}

// button
void CPropertyView::UpdateUIButtonInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIButton* pUI = NULL;
	if (UI_TYPE_BUTTON == pUIBase->UIType()) pUI = (CN3UIButton*)pUIBase;
	else return;
	m_UIButton.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIButton.GetPropItem("Style");							// style
	if (pItem)
	{
		if (UISTYLE_BTN_NORMAL & pUI->m_dwStyle) pItem->m_crColor = 0;
		else if (UISTYLE_BTN_CHECK & pUI->m_dwStyle) pItem->m_crColor = 1;
	}
	pItem = m_UIButton.GetPropItem("ClickRect left");							// click rect
	if (pItem) pItem->m_curValue.Format("%d", pUI->GetClickRect().left);
	pItem = m_UIButton.GetPropItem("ClickRect top");
	if (pItem) pItem->m_curValue.Format("%d", pUI->GetClickRect().top);
	pItem = m_UIButton.GetPropItem("ClickRect right");							
	if (pItem) pItem->m_curValue.Format("%d", pUI->GetClickRect().right);
	pItem = m_UIButton.GetPropItem("ClickRect bottom");							
	if (pItem) pItem->m_curValue.Format("%d", pUI->GetClickRect().bottom);

	pItem = m_UIButton.GetPropItem("On sound");					// on sound
	if (pItem) pItem->m_curValue = pUI->GetSndFName_On().c_str();

	pItem = m_UIButton.GetPropItem("Click sound");					// click sound
	if (pItem) pItem->m_curValue = pUI->GetSndFName_Click().c_str();

	m_UIButton.Invalidate();
}
// Slot
void CPropertyView::UpdateUIIconSlotInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	if (UI_TYPE_ICONSLOT != pUIBase->UIType()) return;
	m_UIIconSlot.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIIconSlot.GetPropItem("Slot ID");							// style
	if (pItem) pItem->m_curValue.Format("%d", pUIBase->GetReserved());
	m_UIIconSlot.Invalidate();
}

// Area
void CPropertyView::UpdateUIAreaInfo()			// Area정보 갱신
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIArea* pUI = NULL;
	if (UI_TYPE_AREA == pUIBase->UIType()) pUI = (CN3UIArea*)pUIBase;
	else return;
	m_UIArea.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIArea.GetPropItem("Type");							// style
	if (pItem)
	{
		switch ( pUI->m_eAreaType )	
		{
			case UI_AREA_TYPE_NONE:
				pItem->m_crColor = 0;
				break;

			case UI_AREA_TYPE_SLOT:
				pItem->m_crColor = 1;
				break;

			case UI_AREA_TYPE_INV:
				pItem->m_crColor = 2;
				break;

			case UI_AREA_TYPE_TRADE_NPC:
				pItem->m_crColor = 3;
				break;

			case UI_AREA_TYPE_PER_TRADE_MY:
				pItem->m_crColor = 4;
				break;

			case UI_AREA_TYPE_PER_TRADE_OTHER:
				pItem->m_crColor = 5;
				break;

			case UI_AREA_TYPE_DROP_ITEM:
				pItem->m_crColor = 6;
				break;

			case UI_AREA_TYPE_SKILL_TREE:
				pItem->m_crColor = 7;
				break;

			case UI_AREA_TYPE_SKILL_HOTKEY:
				pItem->m_crColor = 8;
				break;

			case UI_AREA_TYPE_REPAIR_INV:
				pItem->m_crColor = 9;
				break;

			case UI_AREA_TYPE_REPAIR_NPC:
				pItem->m_crColor = 10;
				break;

			case UI_AREA_TYPE_TRADE_MY:
				pItem->m_crColor = 11;
				break;

			case UI_AREA_TYPE_PER_TRADE_INV:
				pItem->m_crColor = 12;
				break;
		}
	}
	m_UIArea.Invalidate();
}

// static
void CPropertyView::UpdateUIStaticInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIStatic* pUI = NULL;
	if (UI_TYPE_STATIC == pUIBase->UIType() ||
		UI_TYPE_EDIT == pUIBase->UIType() ) pUI = (CN3UIStatic*)pUIBase;	// edit도 static에서 상속 받은 것이다.
	else return;
	m_UIStatic.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIStatic.GetPropItem("Click sound");					// click sound
	if (pItem) pItem->m_curValue = pUI->GetSndFName_Click().c_str();

	m_UIStatic.Invalidate();
}

// edit
void CPropertyView::UpdateUIEditInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIEdit* pUI = NULL;
	if (UI_TYPE_EDIT == pUIBase->UIType()) pUI = (CN3UIEdit*)pUIBase;
	else return;
	m_UIEdit.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIEdit.GetPropItem("Style");							// style
	if (pItem)
	{
		if (UISTYLE_EDIT_PASSWORD & pUI->m_dwStyle) pItem->m_crColor = 1;
		else if (UISTYLE_EDIT_NUMBERONLY & pUI->m_dwStyle) pItem->m_crColor = 2;
		else pItem->m_crColor = 0;
	}
	pItem = m_UIEdit.GetPropItem("Typing sound");					// typing sound
	if (pItem) pItem->m_curValue = pUI->GetSndFName_Typing().c_str();

	m_UIEdit.Invalidate();
}

// progress
void CPropertyView::UpdateUIProgressInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIProgress* pUI = NULL;
	if (UI_TYPE_PROGRESS == pUIBase->UIType()) pUI = (CN3UIProgress*)pUIBase;
	else return;
	m_UIProgress.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIProgress.GetPropItem("Style");		// style
	if (pItem)
	{
		if (UISTYLE_PROGRESS_LEFT2RIGHT & pUI->m_dwStyle) pItem->m_crColor = 0;
		else if (UISTYLE_PROGRESS_RIGHT2LEFT & pUI->m_dwStyle) pItem->m_crColor = 1;
		else if (UISTYLE_PROGRESS_TOP2BOTTOM & pUI->m_dwStyle) pItem->m_crColor = 2;
		else if (UISTYLE_PROGRESS_BOTTOM2TOP & pUI->m_dwStyle) pItem->m_crColor = 3;
	}
	pItem = m_UIProgress.GetPropItem("MaxValue");							// MaxValue
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetMax());
	pItem = m_UIProgress.GetPropItem("MinValue");							// MinValue
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetMin());
	pItem = m_UIProgress.GetPropItem("CurValue");							// CurValue
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetCurValue());

	m_UIProgress.Invalidate();
}

// trackbar
void CPropertyView::UpdateUITrackBarInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UITrackBar* pUI = NULL;
	if (UI_TYPE_TRACKBAR == pUIBase->UIType()) pUI = (CN3UITrackBar*)pUIBase;
	else return;
	m_UITrackBar.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UITrackBar.GetPropItem("Style");		// style
	if (pItem)
	{
		if (UISTYLE_TRACKBAR_VERTICAL & pUI->m_dwStyle) pItem->m_crColor = 1;
		else pItem->m_crColor = 0;
	}
	pItem = m_UITrackBar.GetPropItem("Max pos");							// Max pos
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetMaxPos());
	pItem = m_UITrackBar.GetPropItem("Min pos");							// Min pos
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetMinPos());
	pItem = m_UITrackBar.GetPropItem("Cur pos");							// Cur pos
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetPos());
	pItem = m_UITrackBar.GetPropItem("Page size");							// Page size
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetPageSize());

	m_UITrackBar.Invalidate();
}

// scrollbar
void CPropertyView::UpdateUIScrollBarInfo()
{
	CN3UIBase* pUIBase = GetDocument()->GetSelectedUI();
	if (NULL == pUIBase) return;
	CN3UIScrollBar* pUI = NULL;
	if (UI_TYPE_SCROLLBAR == pUIBase->UIType()) pUI = (CN3UIScrollBar*)pUIBase;
	else return;
	m_UIScrollBar.ShowWindow(SW_SHOW);

	CPropertyItem* pItem = NULL;
	pItem = m_UIScrollBar.GetPropItem("Style");		// style
	if (pItem)
	{
		if (UISTYLE_SCROLLBAR_VERTICAL & pUI->m_dwStyle) pItem->m_crColor = 1;
		else pItem->m_crColor = 0;
	}
	pItem = m_UIScrollBar.GetPropItem("Line size");							// Line size
	if (pItem) pItem->m_curValue.Format("%d",pUI->GetLineSize());

	m_UIScrollBar.Invalidate();
}

