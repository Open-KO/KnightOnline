// N3FXPartBillBoardGame.h: interface for the CN3FXPartBillBoard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTBILLBOARDGAME_H__
#define __N3FXPARTBILLBOARDGAME_H__

#pragma once

#include <N3Base/N3FXPartBillBoard.h>

class CN3FXPartBillBoardGame : public CN3FXPartBillBoard
{
public:
	CN3FXPartBillBoardGame();
	~CN3FXPartBillBoardGame() override;
	float GetGroundHeight(float x, float z) override;
};

#endif // !defined(AFX_N3FXPARTBILLBOARD_H__3449DE4C_B687_459A_BF2C_A1FB98895B17__INCLUDED_)
