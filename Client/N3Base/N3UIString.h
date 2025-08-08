// N3UIString.h: interface for the CN3UIString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UISTRING_H__E3559B01_72AE_4651_804D_B96D22738ED8__INCLUDED_)
#define AFX_N3UISTRING_H__E3559B01_72AE_4651_804D_B96D22738ED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "N3UIBase.h"
#include "DFont.h"

class CDFont;
class CN3UIString : public CN3UIBase  
{
	friend class CN3UIEdit;
#ifdef _N3TOOL
friend class CPropertyView;	// 툴에서 각 변수들을 접근하기 위해서 
#endif

protected:
	CDFont*			m_pDFont;			// DFont(실제 글자를 화면에 찍어주는 클래스다)
	POINT			m_ptDrawPos;		// 실제 화면에 표시될 글자의 제일 왼쪽 상단 좌표
	std::string 	m_szString;			// string buffer
	D3DCOLOR		m_Color;			// 글자 색
	int				m_iLineCount;		// 현재 세팅된 글씨들의 line수
	std::vector<int>	m_NewLineIndices;	// multiline일경우 새로운 라인의 인덱스들
	int				m_iStartLine;		// multiline일경우 글씨가 찍히는 시작 라인번호
	int				m_iIdk0;

public:
	CN3UIString();
	virtual ~CN3UIString();

// Attributes
public:
	void				SetColor(D3DCOLOR color) { m_Color = color; }
	D3DCOLOR			GetColor() const { return m_Color; }
	const std::string&	GetString() { return m_szString; }
	int					GetLineCount() const {return m_iLineCount;}
	int					GetStartLine() const {return m_iStartLine;}
	int					GetStringRealWidth(int iNum);
	int					GetStringRealWidth(std::string& szText);

	virtual	uint32_t	MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	virtual void	Render();
	virtual void	Release();	
	virtual void	Init(CN3UIBase* pParent);
	virtual BOOL	MoveOffset(int iOffsetX, int iOffsetY);// 글씨찍는 위치도 바뀌어 준다.
	virtual bool	Load(HANDLE hFile);
	void			ClearOnlyStringBuffer() { m_szString = ""; }	// string 버퍼만 지운다.
	void			SetStartLine(int iLine);	// multiline일경우 시작하는 라인 변경하기

	virtual void	operator = (const CN3UIString& other);

#ifdef _N3TOOL
	virtual bool	Save(HANDLE hFile);
	virtual void	ChangeFont(const std::string& szFont);
#endif
	
	virtual void	SetRegion(const RECT& Rect);
	virtual void	SetStyle(uint32_t dwStyle);
	virtual void	SetStyle(uint32_t dwType, uint32_t dwStyleEx);

	virtual void	SetString(const std::string& szString);
	virtual void	SetStringAsInt(int iVal);
	void			SetString_NoWordWrap(const std::string& szString);	// 글자 정렬 하지 않는다.
	virtual void	SetFont(const std::string& szFontName, uint32_t dwHeight, BOOL bBold, BOOL bItalic); // dwHeight는 point size이다.
	BOOL			GetTextExtent(const std::string& szString, int iStrLen, SIZE* pSize )
	{
		if (m_pDFont) return m_pDFont->GetTextExtent(szString, iStrLen, pSize);
		return FALSE;
	}
	uint32_t				GetFontColor() const {if (m_pDFont) return m_pDFont->GetFontColor(); return 0xffffffff;}
	const std::string&	GetFontName() const {if (m_pDFont) return m_pDFont->GetFontName(); return s_szStringTmp;} 
	uint32_t				GetFontHeight() const {if (m_pDFont) return m_pDFont->GetFontHeight(); return 0;} 
	uint32_t				GetFontFlags() const {if (m_pDFont) return m_pDFont->GetFontFlags(); return 0;}
protected:
	void				WordWrap();		// wordwrap
};

#endif // !defined(AFX_N3UISTRING_H__E3559B01_72AE_4651_804D_B96D22738ED8__INCLUDED_)
