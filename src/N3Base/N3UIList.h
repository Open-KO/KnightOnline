// N3UIList.h: interface for the CN3UIList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_)
#define AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_

#pragma once

#include "N3UIBase.h"
#include <list>

class CN3UIString;
typedef std::list<CN3UIString*>::iterator it_pString;

class CN3UIList : public CN3UIBase
{
protected:
	int m_iCurSel;                        // 현재 선택..
	std::list<CN3UIString*> m_ListString; // String List
	class CN3UIScrollBar* m_pScrollBarRef;

	std::string m_szFontName;
	uint32_t m_dwFontHeight;
	BOOL m_bFontBold;
	BOOL m_bFontItalic;
	D3DCOLOR m_crFont;

public:
	const std::string& FontName() const
	{
		return m_szFontName;
	}

	uint32_t FontHeight() const
	{
		return m_dwFontHeight;
	}

	D3DCOLOR FontColor() const
	{
		return m_crFont;
	}

	BOOL FontIsBold() const
	{
		return m_bFontBold;
	}

	BOOL FontIsItalic() const
	{
		return m_bFontItalic;
	}

	void SetFont(const std::string& szFontName, uint32_t dwHeight, BOOL bBold, BOOL bItalic);
	void SetFontColor(D3DCOLOR color);
	void SetFontColor(int iIndex, D3DCOLOR color);

	void ResetContent();
	void UpdateChildRegions();
	int AddStrings(const std::string* pszStrings, int iStringCount);
	int AddString(const std::string& szString);
	bool InsertString(int iIndex, const std::string& szString);
	bool DeleteString(int iIndex);
	bool GetString(int iIndex, std::string& szString);
	bool SetString(int iIndex, const std::string& szString);

	int GetCurSel() const
	{
		return m_iCurSel;
	}

	bool SetCurSel(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
			m_iCurSel = -1;
		else
			m_iCurSel = iIndex;
		return true;
	}

	CN3UIString* GetChildStrFromList(const std::string& str);

	int GetCount() const
	{
		return static_cast<int>(m_ListString.size());
	}

	int GetScrollPos() const;
	bool SetScrollPos(int iScrollPos);

	void Render() override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool Load(File& file) override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;

#ifdef _N3TOOL
	bool Save(File& file) override;
	CN3UIList& operator=(const CN3UIList& other);
#endif

	void Release() override;
	CN3UIList();
	~CN3UIList() override;
};

#endif // !defined(AFX_N3UILIST_H__89CAFCE3_0042_4F6C_A195_E830111DA03F__INCLUDED_)
