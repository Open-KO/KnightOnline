// WarMessage.h: interface for the CWarMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WARMESSAGE_H__F05854B5_6723_4F5C_BDAB_329F1939FEA6__INCLUDED_)
#define AFX_WARMESSAGE_H__F05854B5_6723_4F5C_BDAB_329F1939FEA6__INCLUDED_

#pragma once

#include <N3Base/DFont.h> // D3DFONT_BOLD

class CWarMessage
{
private:
	float m_fTime;
	CDFont* m_pMessageFont;
	POINT m_ptMessage;

public:
	void Tick();
	void RenderMessage();
	void SetMessage(const std::string& szText, uint32_t dwFlags = D3DFONT_BOLD, uint32_t dwColor = 0xffffffff);
	void Release();
	void InitFont();
	CWarMessage();
	virtual ~CWarMessage();
};

#endif // !defined(AFX_WARMESSAGE_H__F05854B5_6723_4F5C_BDAB_329F1939FEA6__INCLUDED_)
