// N3FXPartBillBoard.cpp: implementation of the CN3FXPartBillBoard class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameProcedure.h"
#include "N3WorldManager.h"
#include "N3FXPartBillBoardGame.h"

CN3FXPartBillBoardGame::CN3FXPartBillBoardGame()
{
}

CN3FXPartBillBoardGame::~CN3FXPartBillBoardGame()
{
}

float CN3FXPartBillBoardGame::GetGroundHeight(float x, float z)
{
	return CGameBase::ACT_WORLD->GetHeightWithTerrain(x, z) + 0.1f;
}
