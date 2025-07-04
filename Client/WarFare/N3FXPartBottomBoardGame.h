// N3FXPartBottomBoardGame.h: interface for the CN3FXPartBottomBoard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTBOTTOMBOARDGAME_H__
#define __N3FXPARTBOTTOMBOARDGAME_H__

#include <N3Base/N3FXPartBottomBoard.h>

class CN3FXPartBottomBoardGame : public CN3FXPartBottomBoard
{
public:
	float	GetGroundHeight(float x, float z);

public:
	CN3FXPartBottomBoardGame();
	virtual ~CN3FXPartBottomBoardGame();
};

#endif // #ifndef __N3FXPARTBOTTOMBOARD_H__
