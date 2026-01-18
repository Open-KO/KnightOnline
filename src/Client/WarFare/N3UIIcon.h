// N3UIIcon.h: interface for the CN3UIIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIICON_H__DD3FD0B0_334A_4DA5_81E7_F29E82C84D6D__INCLUDED_)
#define AFX_N3UIICON_H__DD3FD0B0_334A_4DA5_81E7_F29E82C84D6D__INCLUDED_

#pragma once

#include <N3Base/N3UIImage.h>

class CN3UIIcon : public CN3UIImage
{
	D3DCOLOR m_dc; // Backup Color

public:
	CN3UIIcon();
	~CN3UIIcon() override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	void Render() override;
};

#endif // !defined(AFX_N3UIICON_H__DD3FD0B0_334A_4DA5_81E7_F29E82C84D6D__INCLUDED_)
