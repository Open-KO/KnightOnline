#include "stdafx.h"
#include "TblEditorView.h"
#include "TblEditorDoc.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr int MAX_COLUMN_WIDTH		= 300; // Allow maximum width of 300px

static constexpr int CODEPAGE_KOREAN		= 949;
static constexpr int CODEPAGE_ENGLISH_US	= 1252;
static constexpr int CODEPAGE_TURKISH		= 1254;

IMPLEMENT_DYNCREATE(CTblEditorView, CFormView)

BEGIN_MESSAGE_MAP(CTblEditorView, CFormView)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	ON_MESSAGE(WM_USER_LIST_MODIFIED, OnListModified)
	ON_COMMAND(ID_ENCODING_KOREAN, OnSetEncoding_Korean)
	ON_COMMAND(ID_ENCODING_ENGLISH_US, OnSetEncoding_EnglishUS)
	ON_COMMAND(ID_ENCODING_TURKISH, OnSetEncoding_Turkish)
	ON_COMMAND(ID_EXIT, OnMenuExit)
END_MESSAGE_MAP()

CTblEditorView::CTblEditorView()
	: CFormView(IDD_TBLEDITOR_DIALOG)
{
	m_iEditItem			= 0;
	m_iEditSubItem		= 0;

	m_iStringCodePage	= CODEPAGE_KOREAN;
}

void CTblEditorView::RefreshTable()
{
	if (m_ListCtrl.GetSafeHwnd() == nullptr)
		return;

	CTblEditorDoc* pDoc = static_cast<CTblEditorDoc*>(m_pDocument);
	ASSERT_VALID(pDoc);

	const CTblEditorBase& tbl = pDoc->GetTbl();

	const std::map<int, std::vector<CStringA>>& rows
		= tbl.GetRows();

	m_ListCtrl.DeleteAllItems();

	int nColumnCount = m_ListCtrl.GetHeaderCtrl()->GetItemCount();
	for (int i = nColumnCount - 1; i >= 0; i--)
		m_ListCtrl.DeleteColumn(i);

	size_t columnCount = tbl.GetColumnTypes().size();
	std::vector<int> columnWidths(columnCount, 0);

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(m_ListCtrl.GetFont());

	// Add column headers and measure initial width
	for (size_t i = 0; i < columnCount; i++)
	{
		const int iColNo = static_cast<int>(i);

		CString headerText = tbl.GetFullColumnName(iColNo);

		CSize size = dc.GetTextExtent(headerText);
		columnWidths[i] = size.cx + 20;  // +20 padding

		m_ListCtrl.InsertColumn(iColNo, headerText, LVCFMT_LEFT, 100); // use a width of 100 for now
	}

	// Add rows and measure contents
	for (const auto& it : rows)
	{
		constexpr int Padding = 10;

		int rowIndex = it.first;
		const std::vector<CStringA>& fields = it.second;

		CStringA firstFieldA;
		if (!fields.empty())
			firstFieldA = fields[0];

		CString firstField = DecodeField(firstFieldA, 0);

		int nItem = m_ListCtrl.InsertItem(rowIndex, firstField);

		CSize size = dc.GetTextExtent(firstField);
		if (size.cx + Padding > columnWidths[0])
			columnWidths[0] = std::min<int>(size.cx + Padding, static_cast<int>(MAX_COLUMN_WIDTH));

		for (size_t i = 1; i < fields.size() && i < columnCount; i++)
		{
			CString text = DecodeField(fields[i], i);
			m_ListCtrl.SetItemText(nItem, static_cast<int>(i), text);

			CSize size = dc.GetTextExtent(text);
			if (size.cx + Padding > columnWidths[i])
				columnWidths[i] = std::min<int>(size.cx + Padding, static_cast<int>(MAX_COLUMN_WIDTH));
		}
	}

	// Restore font
	dc.SelectObject(pOldFont);

	// Apply real widths
	for (size_t i = 0; i < columnCount; i++)
		m_ListCtrl.SetColumnWidth(static_cast<int>(i), columnWidths[i]);
}

CString CTblEditorView::DecodeField(
	const CStringA& fieldA,
	int iColNo)
	const
{
	CTblEditorDoc* pDoc = static_cast<CTblEditorDoc*>(m_pDocument);
	ASSERT_VALID(pDoc);

	const CTblEditorBase& tbl = pDoc->GetTbl();

	CString field;

	DATA_TYPE columnType = tbl.GetColumnType(iColNo);
	if (columnType == DT_STRING)
	{
		field = CA2T(fieldA, m_iStringCodePage);
		CTblListCtrl::EscapeForDisplay(field);
	}
	else
	{
		field = fieldA;
	}

	return field;
}

void CTblEditorView::BuildTableForSave(
	std::map<int, std::vector<CStringA>>& newRows)
{
	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CTblEditorBase& tbl = pDoc->GetTbl();
	const int iRowCount = m_ListCtrl.GetItemCount();
	const int iColCount = m_ListCtrl.GetHeaderCtrl()->GetItemCount();

	for (int iRowNo = 0; iRowNo < iRowCount; iRowNo++)
	{
		std::vector<CStringA> rowData;
		rowData.reserve(tbl.GetColumnTypes().size());

		for (int iColNo = 0; iColNo < iColCount; iColNo++)
		{
			CString value = m_ListCtrl.GetItemText(iRowNo, iColNo);
			if (value.IsEmpty())
				value = tbl.GetColumnDefault(iColNo);

			CStringA valueA;

			DATA_TYPE columnType = tbl.GetColumnType(iColNo);
			if (columnType == DT_STRING)
			{
				CTblListCtrl::UnescapeForSave(value);
				valueA = CT2A(value, m_iStringCodePage);
			}
			else
			{
				valueA = value;
			}

			rowData.push_back(valueA);
		}

		newRows.insert(std::make_pair(iRowNo, rowData));
	}
}

bool CTblEditorView::SaveTable(
	const CString& savePath)
{
	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc->IsLoaded())
		return true;

	CTblEditorBase& tbl = pDoc->GetTbl();

	const int iRowCount = m_ListCtrl.GetItemCount();

	TRACE("Saving %d rows", iRowCount);

	// Trace m_DataTypes if needed
	const auto& columnTypes = tbl.GetColumnTypes();

	TRACE("Column types:\n");
	for (size_t i = 0; i < columnTypes.size(); i++)
		TRACE("  [%zu] = %d\n", i, columnTypes[i]);

	if (columnTypes.empty())
		return true;

	std::map<int, std::vector<CStringA>> newRows;
	BuildTableForSave(newRows);

	if (!tbl.SaveFile(savePath, newRows))
	{
		AfxMessageBox(IDS_SAVE_FAILED, MB_ICONERROR);
		return false;
	}

	AfxMessageBox(IDS_SAVE_SUCCESS, MB_ICONINFORMATION);

	// Only consider the file as no longer being modified if we saved over
	// the same file we loaded.
	if (savePath.CompareNoCase(pDoc->GetPathName()) == 0)
		pDoc->SetModifiedFlag(FALSE);

	return true;
}

void CTblEditorView::SetCodePage(
	int codepage)
{
	CWnd* pWnd = AfxGetApp()->GetMainWnd();
	ASSERT_VALID(pWnd);

	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CMenu* pMenu = pWnd->GetMenu();
	if (pMenu != nullptr)
	{
		pMenu->CheckMenuItem(ID_ENCODING_KOREAN,		codepage == CODEPAGE_KOREAN		? MF_CHECKED : MF_UNCHECKED);
		pMenu->CheckMenuItem(ID_ENCODING_ENGLISH_US,	codepage == CODEPAGE_ENGLISH_US	? MF_CHECKED : MF_UNCHECKED);
		pMenu->CheckMenuItem(ID_ENCODING_TURKISH,		codepage == CODEPAGE_TURKISH	? MF_CHECKED : MF_UNCHECKED);
	}

	// TODO: List should be backed by this row data to avoid all of this unnecessary work.
	CTblEditorBase& tbl = pDoc->GetTbl();
	tbl.GetRows().clear();
	BuildTableForSave(tbl.GetRows());

	m_iStringCodePage = codepage;

	int iTopIndex = m_ListCtrl.GetTopIndex();

	RefreshTable();

	m_ListCtrl.EnsureVisible(m_ListCtrl.GetItemCount() - 1, TRUE);
	m_ListCtrl.EnsureVisible(iTopIndex, FALSE);

	UpdateData(TRUE);
}

CTblEditorDoc* CTblEditorView::GetDocument()
{
	return static_cast<CTblEditorDoc*>(m_pDocument);
}

void CTblEditorView::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);

	CFormView::DoDataExchange(pDX);
}

void CTblEditorView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CTblEditorBase& tbl = pDoc->GetTbl();
	m_ListCtrl.AttachTbl(&tbl);

	m_ListCtrl.SetExtendedStyle(
		m_ListCtrl.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT
		| LVS_EX_GRIDLINES
		| LVS_EX_SUBITEMIMAGES
		| LVS_EX_ONECLICKACTIVATE);
}

void CTblEditorView::OnUpdate(
	CView* pSender,
	LPARAM lHint,
	CObject* pHint)
{
	if (lHint == CTblEditorDoc::HINT_DOCUMENT_LOADED)
		RefreshTable();

	CFormView::OnUpdate(pSender, lHint, pHint);
}

int CTblEditorView::OnCreate(
	LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	DragAcceptFiles();
	return 0;
}

void CTblEditorView::OnDropFiles(
	HDROP hDropInfo)
{
	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	TCHAR szFile[MAX_PATH + 1] = {};
	UINT uiFiles = DragQueryFile(hDropInfo, 0xFFFF, nullptr, 0);
	::DragQueryFile(hDropInfo, 0, szFile, MAX_PATH);
	::DragFinish(hDropInfo);

	if (pDoc != nullptr)
		pDoc->OnOpenDocument(szFile);

	CFormView::OnDropFiles(hDropInfo);
}

LRESULT CTblEditorView::OnListModified(
	WPARAM wParam,
	LPARAM lParam)
{
	CTblEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->SetModifiedFlag(TRUE);
	return 0;
}

void CTblEditorView::OnSetEncoding_Korean()
{
	SetCodePage(CODEPAGE_KOREAN);
}

void CTblEditorView::OnSetEncoding_EnglishUS()
{
	SetCodePage(CODEPAGE_ENGLISH_US);
}

void CTblEditorView::OnSetEncoding_Turkish()
{
	SetCodePage(CODEPAGE_TURKISH);
}

void CTblEditorView::OnMenuExit()
{
	PostQuitMessage(0);
}
