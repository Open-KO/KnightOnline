// N3FXPartBottomBoard.cpp: implementation of the CN3FXPartBottomBoard class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameProcedure.h"
#include "N3WorldManager.h"
#include "N3FXPartBottomBoardGame.h"

CN3FXPartBottomBoardGame::CN3FXPartBottomBoardGame()
{
}

CN3FXPartBottomBoardGame::~CN3FXPartBottomBoardGame()
{
}

float CN3FXPartBottomBoardGame::GetGroundHeight(float x, float z)
{
	return CGameBase::ACT_WORLD->GetHeightWithTerrain(x, z) + 0.1f;
}
