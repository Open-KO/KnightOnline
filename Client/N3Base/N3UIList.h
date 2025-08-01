// N3UIList.h: interface for the CN3UIList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_)
#define AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3UIBase.h"
#include <list>

class CN3UIString;
typedef std::list<CN3UIString*>::iterator it_pString;

class CN3UIList : public CN3UIBase  
{
protected:
	size_t					m_iCurSel;		// ÇöÀç ¼±ÅÃ..
	std::list<CN3UIString*>	m_ListString;	// String List
	class CN3UIScrollBar*	m_pScrollBarRef;

	std::string				m_szFontName;
	uint32_t					m_dwFontHeight;
	BOOL					m_bFontBold;
	BOOL					m_bFontItalic;
	D3DCOLOR				m_crFont;
	
public:
	const std::string&	FontName() { return m_szFontName; }
	uint32_t				FontHeight() { return m_dwFontHeight; }
	D3DCOLOR			FontColor() { return m_crFont; }
	BOOL				FontIsBold() { return m_bFontBold; }
	BOOL				FontIsItalic() { return m_bFontItalic; }

	void	SetFont(const std::string& szFontName, uint32_t dwHeight, BOOL bBold, BOOL bItalic);
	void	SetFontColor(D3DCOLOR color);
	void	SetFontColor(size_t iIndex, D3DCOLOR color);

	void	ResetContent();
	void	UpdateChildRegions();
	int		AddStrings(const std::string* pszStrings, int iStringCount);
	int		AddString(const std::string& szString);
	bool	InsertString(size_t iIndex, const std::string& szString);
	bool	DeleteString(size_t iIndex);
	bool	GetString(size_t iIndex, std::string& szString);
	bool	SetString(size_t iIndex, const std::string& szString);
	size_t	GetCurSel() { return m_iCurSel; }
	bool	SetCurSel(size_t iIndex) { if (iIndex >= m_ListString.size()) m_iCurSel = ~((size_t)0); else m_iCurSel = iIndex; return true; }
	CN3UIString*	GetChildStrFromList(std::string str);
	int		GetCount() { return m_ListString.size(); }

	int		GetScrollPos();
	bool	SetScrollPos(int iScrollPos);
	
	virtual void	Render();
	virtual bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual bool	Load(HANDLE hFile);
	virtual uint32_t	MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);

#ifdef _N3TOOL
	virtual bool	Save(HANDLE hFile);
	virtual void	operator = (const CN3UIList& other);
#endif

	virtual void	Release();
	CN3UIList();
	virtual ~CN3UIList();
};

#endif // !defined(AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_)
