#include "StdAfx.h"
#include "N3ClientShapeMgr.h"
#include "GameProcedure.h"
#include "UILoading.h"

void CN3ClientShapeMgr::UpdateLoadStatus(int iLoadedShapes, int iTotalShapes)
{
#ifdef _REPENT
	CGameProcedure::RenderLoadingBar(80 + 15 * iLoadedShapes / iTotalShapes);
#else
	char szBuff[128] = {};

	int iLoading = (iLoadedShapes + 1) * 100 / iTotalShapes;
	snprintf(szBuff, sizeof(szBuff), "Loading Objects... %d %%", iLoading);
	CGameProcedure::s_pUILoading->Render(szBuff, iLoading);
#endif
}
