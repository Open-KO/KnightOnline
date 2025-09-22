// LOGIC_ELSE.h: interface for the LOGIC_ELSE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGIC_ELSE_H__B1062E53_5608_11D3_BE54_00105A6B97E2__INCLUDED_)
#define AFX_LOGIC_ELSE_H__B1062E53_5608_11D3_BE54_00105A6B97E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LOGIC_ELSE
{
public:
	void Parse_and(const char* line, const std::wstring& filename, int lineNumber);
	void Init();
	BYTE m_LogicElse;
	bool m_bAnd;
	int m_LogicElseInt[MAX_LOGIC_ELSE_INT];

	LOGIC_ELSE();
	virtual ~LOGIC_ELSE();
};

#endif // !defined(AFX_LOGIC_ELSE_H__B1062E53_5608_11D3_BE54_00105A6B97E2__INCLUDED_)
