#if !defined(AFX_SHELLTREE_H__6B1818E3_8ADA_11D1_B10E_40F603C10000__INCLUDED_)
#define AFX_SHELLTREE_H__6B1818E3_8ADA_11D1_B10E_40F603C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ShellTree.h : header file
//

#include <shlobj.h>

class CFileName  
{
public:
	CFileName(CString szFileName="");
	virtual ~CFileName();

	void		SetFileName(CString szFileName);
	CString		GetFileName();
	CString		GetRoot();
	CString		GetFileTitle();
	CString		GetDescription();
	bool		Exist();

private:
	CString		m_szFileName;
	char		m_szDrive[_MAX_DRIVE];
	char		m_szDir[_MAX_DIR];
	char		m_szFname[_MAX_FNAME];
	char		m_szExt[_MAX_EXT];
};

class CShellPidl
{
public:
	// STRUCTURES
	typedef struct tagLVID
	{
		LPSHELLFOLDER lpsfParent;
		LPITEMIDLIST  lpi;
		ULONG         ulAttribs;
	} LVITEMDATA, *LPLVITEMDATA;

	typedef struct tagID
	{
		LPSHELLFOLDER lpsfParent;
		LPITEMIDLIST  lpi;
		LPITEMIDLIST  lpifq;
	} TVITEMDATA, *LPTVITEMDATA;

public:

    // Functions that deal with PIDLs
    LPITEMIDLIST	ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    LPITEMIDLIST	GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi);
    LPITEMIDLIST	CopyITEMID(LPMALLOC lpMalloc, LPITEMIDLIST lpi);
    BOOL			GetName(LPSHELLFOLDER lpsf, LPITEMIDLIST  lpi, DWORD dwFlags, LPSTR lpFriendlyName);
    LPITEMIDLIST	CreatePidl(UINT cbSize);
    UINT			GetSize(LPCITEMIDLIST pidl);
    LPITEMIDLIST	Next(LPCITEMIDLIST pidl);

	// Utility Functions
	BOOL	DoTheMenuThing(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST  lpi, LPPOINT lppt);
	int		GetItemIcon(LPITEMIDLIST lpi, UINT uFlags);
};

class CShellTree : public CTreeCtrl, public CShellPidl
{
public:
	enum FindAttribs{type_drive,type_folder};

// Construction
public:
	CShellTree();

// Attributes
public:
    BOOL m_bOutaHere;

// Operations
public:
	void	PopulateTree();
	void	PopulateTree(int nFolder);
	void	OnFolderExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	void	GetContextMenu(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL	OnFolderSelected(NMHDR* pNMHDR, LRESULT* pResult, CString &szFolderPath);
	void	OnDeleteShellItem(NMHDR* pNMHDR, LRESULT* pResult);
	void	EnableImages();
	BOOL	GetSelectedFolderPath(CString &szFolderPath);
	bool	SearchTree(HTREEITEM treeNode, CString szSearchName, FindAttribs attr);
	void	TunnelTree(CString szFindPath);
	LPSHELLFOLDER	GetParentShellFolder(HTREEITEM folderNode);
	LPITEMIDLIST	GetRelativeIDLIST(HTREEITEM folderNode);
	LPITEMIDLIST	GetFullyQualifiedID(HTREEITEM folderNode);
	void	FindTreePidl(	HTREEITEM nextNode,
							HTREEITEM& folderNode, 
							LPLVITEMDATA lplvid, 
							bool& valid);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShellTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShellTree();

	// Generated message map functions
protected:
	void	FillTreeView(LPSHELLFOLDER lpsf, LPITEMIDLIST  lpifq, HTREEITEM     hParent);
	void	GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTV_ITEM lptvitem);
 	static int CALLBACK TreeViewCompareProc(LPARAM, LPARAM, LPARAM);
	
	//{{AFX_MSG(CShellTree)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHELLTREE_H__6B1818E3_8ADA_11D1_B10E_40F603C10000__INCLUDED_)
