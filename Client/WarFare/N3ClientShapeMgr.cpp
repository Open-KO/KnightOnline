#include "StdAfx.h"
#include "N3ClientShapeMgr.h"
#include "GameProcedure.h"
#include "UILoading.h"

void CN3ClientShapeMgr::UpdateLoadStatus(int iLoadedShapes, int iTotalShapes)
{
#ifdef _REPENT
	CGameProcedure::RenderLoadingBar(80 + 15 * iLoadedShapes / iTotalShapes);
#else
	int iLoading = (iLoadedShapes + 1) * 100 / iTotalShapes;
	std::string buff = fmt::format("Loading Objects... {} %", iLoading);
	CGameProcedure::s_pUILoading->Render(buff, iLoading);
#endif
}
