// N3UIIcon.h: interface for the CN3UIIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined __N3UIDBCLBUTTON_H__
#define __N3UIDBCLBUTTON_H__

#pragma once

#include <N3Base/N3UIImage.h>

class CN3UIDBCLButton : public CN3UIImage
{
public:
	CN3UIDBCLButton();
	~CN3UIDBCLButton() override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
};

#endif // !defined(AFX_N3UIICON_H__DD3FD0B0_334A_4DA5_81E7_F29E82C84D6D__INCLUDED_)
