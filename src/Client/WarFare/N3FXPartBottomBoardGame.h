// N3FXPartBottomBoardGame.h: interface for the CN3FXPartBottomBoard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTBOTTOMBOARDGAME_H__
#define __N3FXPARTBOTTOMBOARDGAME_H__

#pragma once

#include <N3Base/N3FXPartBottomBoard.h>

class CN3FXPartBottomBoardGame : public CN3FXPartBottomBoard
{
public:
	CN3FXPartBottomBoardGame();
	~CN3FXPartBottomBoardGame() override;
	float GetGroundHeight(float x, float z) override;
};

#endif // #ifndef __N3FXPARTBOTTOMBOARD_H__
