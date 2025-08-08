// PropertyList.cpp : implementation file
//
//#include "StdAfx.h"
#include "PropertyList.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyList
D3DVECTOR CPropertyItem::VectorGet()
{
	float xyz[3] = {0,0,0};
	int j = 0, k = 0;
	for(int i = 0; i < 3; i++)
	{
		CString bufr;
		if(i < 2) 
		{
			j = m_curValue.Find(',', j);
			bufr = m_curValue.Mid(k, j-k);
		}
		else 
			bufr = m_curValue.Mid(k);

		xyz[i] = (float)atof(bufr);

		j++;
		k = j;
	}

	D3DVECTOR v = { xyz[0], xyz[1], xyz[2] };
	return v;
}

void CPropertyItem::VectorSet(const D3DVECTOR& v)
{
	m_curValue.Format("%f, %f, %f", v.x, v.y, v.z);
}

CPropertyList::CPropertyList()
{
}

CPropertyList::~CPropertyList()
{
}


BEGIN_MESSAGE_MAP(CPropertyList, CListBox)
	//{{AFX_MSG_MAP(CPropertyList)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_CBN_KILLFOCUS(IDC_PROPCMBBOX, OnKillfocusCmbBox)
	ON_CBN_SELCHANGE(IDC_PROPCMBBOX, OnSelchangeCmbBox)
	ON_EN_KILLFOCUS(IDC_PROPEDITBOX, OnKillfocusEditBox)
	ON_EN_CHANGE(IDC_PROPEDITBOX, OnChangeEditBox)
	ON_BN_CLICKED(IDC_PROP_BUTTON, OnButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyList message handlers

BOOL CPropertyList::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CListBox::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~(LBS_OWNERDRAWVARIABLE | LBS_SORT);
	cs.style |= LBS_OWNERDRAWFIXED;

	m_bTracking = FALSE;
	m_nDivider = 0;
	m_bDivIsSet = FALSE;

	return TRUE;
}

void CPropertyList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemHeight = 20; //pixels
}


void CPropertyList::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC dc;
	dc.Attach(lpDIS->hDC);
	CRect rectFull = lpDIS->rcItem;
	CRect rect = rectFull;
	if (m_nDivider==0)
		m_nDivider = rect.Width() / 2;
	rect.left = m_nDivider;
	CRect rect2 = rectFull;
	rect2.right = rect.left - 1;
	UINT nIndex = lpDIS->itemID;

	if (nIndex != (UINT) -1)
	{
		//draw two rectangles, one for each row column
		dc.FillSolidRect(rect2,RGB(192,192,192));
		dc.DrawEdge(rect2,EDGE_SUNKEN,BF_BOTTOMRIGHT);
		dc.DrawEdge(rect,EDGE_SUNKEN,BF_BOTTOM);

		//get the CPropertyItem for the current row
		CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(nIndex);

		//write the property name in the first rectangle
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(pItem->m_propName,CRect(rect2.left+3,rect2.top+3,
											rect2.right-3,rect2.bottom+3),
					DT_LEFT | DT_SINGLELINE);

		// 만약 컬러면..
		if(pItem->m_nItemType == PIT_COLOR)
		{
			dc.FillSolidRect(CRect(rect.left+2,rect.top+2,rect.right-4,rect.bottom-4), pItem->m_crColor); // 배경 색 그리기..
		}
		else if(pItem->m_nItemType == PIT_COMBO)
		{
			int nSel = pItem->m_crColor;
			if(nSel >= 0 && nSel < pItem->m_szCBItemsOrFilter.GetSize())
			{
				pItem->m_curValue = pItem->m_szCBItemsOrFilter.GetAt(nSel);
			}
			else
			{
				pItem->m_curValue = "";
			}
		}
//		else if(pItem->m_nItemType == PIT_CHECK)
//		{
//			if(pItem->m_crColor == 0) pItem->m_curValue = "On";
//			else pItem->m_curValue = "Off";
//		}

		//write the initial property value in the second rectangle
		dc.DrawText(pItem->m_curValue,CRect(rect.left+3,rect.top+3,
											rect.right+3,rect.bottom+3),
					DT_LEFT | DT_SINGLELINE);
	}
	dc.Detach();
}

int CPropertyList::AddItem(CString txt)
{
	int nIndex = AddString(txt);
	return nIndex;
}

int CPropertyList::AddPropItem(const char* szPropName, const char* szVal, int nItemType, const char* szCmbItems, COLORREF crColor)
{
	CPropertyItem* pItem = new CPropertyItem(szPropName, szVal, nItemType, szCmbItems, crColor);
	
	int nIndex = AddString(_T(""));
	SetItemDataPtr(nIndex,pItem);
	return nIndex;
}

int CPropertyList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bDivIsSet = FALSE;
	m_nDivider = 0;
	m_bTracking = FALSE;

	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	m_SSerif8Font.CreatePointFont(80,_T("MS Sans Serif"));

	return 0;
}

void CPropertyList::OnSelchange() 
{
	CRect rect;

	//m_curSel = GetCurSel();

	GetItemRect(m_curSel,rect);
	rect.left = m_nDivider;

	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);

	if (m_ButtonPush)
		m_ButtonPush.ShowWindow(SW_HIDE);

	if (pItem->m_nItemType==PIT_COMBO)
	{
		//display the combo box.  If the combo box has already been
		//created then simply move it to the new location, else create it
		m_nLastBox = 0;
		if (m_ComboBox)
			m_ComboBox.MoveWindow(rect);
		else
		{	
			rect.bottom += 300;
			m_ComboBox.Create(CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_CHILD | WS_BORDER,
							rect,this,IDC_PROPCMBBOX);
			m_ComboBox.SetFont(&m_SSerif8Font);
		}

		//add the choices for this particular property
		m_ComboBox.ResetContent();
		int nCmbC = pItem->m_szCBItemsOrFilter.GetSize();
		for(int i = 0; i <  nCmbC; i++)
		{
			m_ComboBox.AddString(pItem->m_szCBItemsOrFilter.GetAt(i));
		}
		m_ComboBox.SetCurSel(pItem->m_crColor);

		m_ComboBox.ShowWindow(SW_SHOW);
		m_ComboBox.SetFocus();
	}
	else if (pItem->m_nItemType==PIT_EDIT)
	{
		//display edit box
		m_nLastBox = 1;
		m_prevSel = m_curSel;
		rect.bottom -= 3;
		if (m_EditBox)
			m_EditBox.MoveWindow(rect);
		else
		{	
			m_EditBox.Create(ES_LEFT | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER,
							rect,this,IDC_PROPEDITBOX);
			m_EditBox.SetFont(&m_SSerif8Font);
		}

		m_EditBox.ShowWindow(SW_SHOW);
		m_EditBox.SetFocus();
		//set the text in the edit box to the property's current value
		m_EditBox.SetWindowText(pItem->m_curValue);
	}
	else if (pItem->m_nItemType==PIT_COLOR)
	{
		// Display Button
		DisplayPushButton(rect);

		//display edit box
		m_nLastBox = 1;
		m_prevSel = m_curSel;
		rect.bottom -= 3;
		rect.right -= 25;
		if (m_EditBox)
			m_EditBox.MoveWindow(rect);
		else
		{	
			m_EditBox.Create(ES_LEFT | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER,
							rect,this,IDC_PROPEDITBOX);
			m_EditBox.SetFont(&m_SSerif8Font);
		}

		m_EditBox.ShowWindow(SW_SHOW);
		m_EditBox.SetFocus();
		//set the text in the edit box to the property's current value
		m_EditBox.SetWindowText(pItem->m_curValue);
	}
//	else if(pItem->m_nItemType == PIT_CHECK)
//	{
//		DisplayCheckButton(rect);
//	}
	else 
		DisplayPushButton(rect);
}

void CPropertyList::DisplayPushButton(CRect region)
{
	//displays a button if the property is a file/color/font chooser
	m_nLastBox = 2;
	m_prevSel = m_curSel;

	if (region.Width() > 25)
		region.left = region.right - 25;
	region.bottom -= 3;

	if (m_ButtonPush)
		m_ButtonPush.MoveWindow(region);
	else
	{	
		m_ButtonPush.Create("...",BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
						region,this,IDC_PROP_BUTTON);
		m_ButtonPush.SetFont(&m_SSerif8Font);
	}

	m_ButtonPush.ShowWindow(SW_SHOW);
	m_ButtonPush.SetFocus();
}
/*
void CPropertyList::DisplayCheckButton(CRect region)
{
	//displays a button if the property is a file/color/font chooser
	m_nLastBox = 2;
	m_prevSel = m_curSel;

	if (region.Width() > 25)
		region.left = region.right - 25;
	region.bottom -= 3;

	if (m_ButtonCheck)
		m_ButtonCheck.MoveWindow(region);
	else
	{	
		m_ButtonCheck.Create("...",BS_CHECKBOX | WS_VISIBLE | WS_CHILD,
						region,this,IDC_PROP_BUTTON);
		m_ButtonCheck.SetFont(&m_SSerif8Font);
	}

	m_ButtonCheck.ShowWindow(SW_SHOW);
	m_ButtonCheck.SetFocus();
}
*/
void CPropertyList::OnKillFocus(CWnd* pNewWnd) 
{
	//m_ButtonPush.ShowWindow(SW_HIDE);

	CListBox::OnKillFocus(pNewWnd);
}

void CPropertyList::OnKillfocusCmbBox() 
{
	m_ComboBox.ShowWindow(SW_HIDE);

	Invalidate();
}

void CPropertyList::OnKillfocusEditBox()
{
	CString newStr;
	m_EditBox.ShowWindow(SW_HIDE);

	Invalidate();
}

void CPropertyList::OnSelchangeCmbBox()
{
	CString selStr;
	if (m_ComboBox)
	{
		m_ComboBox.GetLBText(m_ComboBox.GetCurSel(),selStr);
		CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);
		pItem->m_curValue = selStr;
		pItem->m_crColor = m_ComboBox.GetCurSel();

		CWnd* pWnd = GetParent();
		if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..
	}
}

void CPropertyList::OnChangeEditBox()
{
	CString newStr;
	m_EditBox.GetWindowText(newStr);
	
	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);
	pItem->m_curValue = newStr;

	CWnd* pWnd = GetParent();
	if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..
}

void CPropertyList::OnButton()
{
	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);

	//display the appropriate common dialog depending on what type
	//of chooser is associated with the property
	if (pItem->m_nItemType == PIT_COLOR)
	{
		CColorDialog ClrDlg(pItem->m_crColor);
		
		if (IDOK == ClrDlg.DoModal())
		{
			DWORD dwAlpha = pItem->m_crColor & 0xff000000;
			pItem->m_crColor = dwAlpha | ClrDlg.GetColor();
			Invalidate();
		}

		CWnd* pWnd = GetParent();
		if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..

	}
	else if (pItem->m_nItemType == PIT_FILE)
	{
		CString SelectedFile; 
		CFileDialog FileDlg(TRUE, NULL, NULL, NULL, pItem->m_szCBItemsOrFilter.GetAt(0));
		
		CString currPath = pItem->m_curValue;
		FileDlg.m_ofn.lpstrTitle = "Select file";
		if (currPath.GetLength() > 0)
			FileDlg.m_ofn.lpstrInitialDir = currPath.Left(
				currPath.GetLength() - currPath.ReverseFind('\\'));

		if(IDOK == FileDlg.DoModal())
		{
			SelectedFile = FileDlg.GetPathName();
			
			m_ButtonPush.ShowWindow(SW_HIDE);
			
			pItem->m_curValue = SelectedFile;
			Invalidate();
		}

		CWnd* pWnd = GetParent();
		if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..

	}
	else if (pItem->m_nItemType == PIT_FILE_MULTI)
	{
		char szBuff[10240] = "";
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
		CFileDialog dlg(TRUE, NULL, NULL, dwFlags, pItem->m_szCBItemsOrFilter.GetAt(0));
		dlg.m_ofn.nMaxFile = 10240;
		dlg.m_ofn.lpstrFile = szBuff;

		if(IDOK == dlg.DoModal())
		{
			POSITION pos = dlg.GetStartPosition();
			CStringArray szFNs;
			
			while(pos != NULL)
			{
				szFNs.Add(dlg.GetNextPathName(pos));
			}

			pItem->m_curValue = "";
			int iSC = szFNs.GetSize();
			if(iSC > 0)
			{
				pItem->m_curValue += szFNs[iSC-1];
				pItem->m_curValue += '\n';

				for(int i = 1; i < iSC-1; i++) // 1 부터 시작하는 이유는 파일 대화상자에서 여러 파일을 부르면 첨과 끝 파일 이름이 바뀌기 때문이다.
				{
					pItem->m_curValue += szFNs[i];
					pItem->m_curValue += '\n';
				}

				pItem->m_curValue += szFNs[0];
				pItem->m_curValue += '\n';

				m_ButtonPush.ShowWindow(SW_HIDE);
				Invalidate();

				CWnd* pWnd = GetParent();
				if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..
			}
		}
	}
	else if (pItem->m_nItemType == PIT_FONT)
	{	
		CFontDialog FontDlg(NULL,CF_EFFECTS | CF_SCREENFONTS,NULL,this);
		
		if(IDOK == FontDlg.DoModal())
		{
			CString faceName = FontDlg.GetFaceName();
			
			m_ButtonPush.ShowWindow(SW_HIDE);

			pItem->m_curValue = faceName;
			Invalidate();
		}

		CWnd* pWnd = GetParent();
		if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..
	}
	else
	{
		CWnd* pWnd = GetParent();
		if(pWnd) pWnd->SendMessage(WM_NOTIFY, (DWORD)this, (DWORD)pItem); // 부모 윈도우에 메시지 보내기..
	}
}

void CPropertyList::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		//if columns were being resized then this indicates
		//that mouse is up so resizing is done.  Need to redraw
		//columns to reflect their new widths.
		
		m_bTracking = FALSE;
		//if mouse was captured then release it
		if (GetCapture()==this)
			::ReleaseCapture();

		::ClipCursor(NULL);

		CClientDC dc(this);
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		//set the divider position to the new value
		m_nDivider = point.x;

		//redraw
		Invalidate();
	}
	else
	{
		BOOL loc;
		int i = ItemFromPoint(point,loc);
		m_curSel = i;
		CListBox::OnLButtonUp(nFlags, point);
	}
}

void CPropertyList::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ((point.x>=m_nDivider-5) && (point.x<=m_nDivider+5))
	{
		//if mouse clicked on divider line, then start resizing

		::SetCursor(m_hCursorSize);

		CRect windowRect;
		GetWindowRect(windowRect);
		windowRect.left += 10; windowRect.right -= 10;
		//do not let mouse leave the list box boundary
		::ClipCursor(windowRect);
		
		if (m_ComboBox)
			m_ComboBox.ShowWindow(SW_HIDE);
		if (m_EditBox)
			m_EditBox.ShowWindow(SW_HIDE);

		CRect clientRect;
		GetClientRect(clientRect);

		m_bTracking = TRUE;
		m_nDivTop = clientRect.top;
		m_nDivBtm = clientRect.bottom;
		m_nOldDivX = point.x;

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));

		//capture the mouse
		SetCapture();
	}
	else
	{
		m_bTracking = FALSE;
		CListBox::OnLButtonDown(nFlags, point);
	}
}

void CPropertyList::OnMouseMove(UINT nFlags, CPoint point) 
{	
	if (m_bTracking)
	{
		//move divider line to the mouse pos. if columns are
		//currently being resized
		CClientDC dc(this);
		//remove old divider line
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));
		//draw new divider line
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		m_nOldDivX = point.x;
	}
	else if ((point.x >= m_nDivider-5) && (point.x <= m_nDivider+5))
		//set the cursor to a sizing cursor if the cursor is over the row divider
		::SetCursor(m_hCursorSize);
	else
		CListBox::OnMouseMove(nFlags, point);
}

void CPropertyList::InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo)
{
	int nOldMode = pDC->SetROP2(R2_NOT);
	
	pDC->MoveTo(ptFrom);
	pDC->LineTo(ptTo);

	pDC->SetROP2(nOldMode);
}

void CPropertyList::PreSubclassWindow() 
{
	m_bDivIsSet = FALSE;
	m_nDivider = 0;
	m_bTracking = FALSE;
	m_curSel = 1;

	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	m_SSerif8Font.CreatePointFont(80,_T("MS Sans Serif"));
}

CPropertyItem* CPropertyList::GetPropItem(const char *szPropName)
{
	int nC = CListBox::GetCount();
	for(int i = 0; i < nC; i++)
	{
		CPropertyItem* pItem = (CPropertyItem*)GetItemDataPtr(i);
		if(pItem == NULL) continue;
		if(lstrcmpi(pItem->m_propName, szPropName) == 0) return pItem;
	}

	return NULL;
}

void CPropertyList::SetDividerWidth(int x)
{
	if(x < 0) return;

	m_nDivider = x;
	Invalidate();
}

CPropertyItem::CPropertyItem(const char* szPropName, const char* szVal, int nItemType, const char* szComboItemsOrFilter, COLORREF crColor)
{
	m_propName = szPropName;
	m_curValue = szVal;
	m_nItemType = nItemType;
	m_crColor = crColor;
	
	if(nItemType == PIT_COMBO)
	{
		CString strTmp = szComboItemsOrFilter;
		int nF = 0, nFPrev = 0;
		while(nF != -1)
		{
			nFPrev = nF;
			nF = strTmp.Find('|', nF);
			if(nF != -1)
			{
				m_szCBItemsOrFilter.Add(strTmp.Mid(nFPrev, nF - nFPrev));
				nF++;
			}
		}
	}
	else
	{
		m_szCBItemsOrFilter.Add(szComboItemsOrFilter);
	}
}

D3DCOLORVALUE CPropertyItem::D3DColorValueGet()
{
	D3DCOLORVALUE color;
	color.r = (float)(m_crColor & 0x000000ff) / 255.0f;
	color.g = (float)((m_crColor & 0x0000ff00) >> 8) / 255.0f;
	color.b = (float)((m_crColor & 0x00ff0000) >> 16) / 255.0f;
	color.a = (float)atof(m_curValue);

	return color;
}

void CPropertyItem::D3DColorValueSet(D3DCOLORVALUE color)
{
	m_crColor =	(((DWORD)(color.r * 255.0f)) << 0) | 
				(((DWORD)(color.g * 255.0f)) << 8) | 
				(((DWORD)(color.b * 255.0f)) << 16);
	m_curValue.Format("%4.2f", color.a);
}

D3DCOLOR CPropertyItem::D3DColorGet()
{
	D3DCOLOR color;
	DWORD dwA, dwR, dwG, dwB;
	dwA = ((DWORD)(atof(m_curValue) * 255)) << 24;
	dwR = (m_crColor & 0x000000ff) << 16;
	dwG = (m_crColor & 0x0000ff00);
	dwB = (m_crColor & 0x00ff0000) >> 16;
	
	color = dwA | dwR | dwG | dwB;

	return color;
}

void CPropertyItem::D3DColorSet(D3DCOLOR color)
{
	DWORD dwR, dwG, dwB;
	float fA = (float)((color & 0xff000000) >> 24)/255.0f;
	m_curValue.Format("%4.2f", fA);
	dwR = (color & 0x00ff0000) >> 16;
	dwG = (color & 0x0000ff00);
	dwB = (color & 0x000000ff) << 16;
	m_crColor = dwR | dwG | dwB;
}

void CPropertyList::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	CPropertyItem* pItem = (CPropertyItem*)(lpDeleteItemStruct->itemData);
	delete pItem; pItem = NULL;
	
	CListBox::DeleteItem(lpDeleteItemStruct);
}
