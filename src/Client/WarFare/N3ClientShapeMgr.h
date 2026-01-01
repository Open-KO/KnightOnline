#pragma once

#include <N3Base/N3ShapeMgr.h>

class CN3ClientShapeMgr : public CN3ShapeMgr
{
public:
	using CN3ShapeMgr::CN3ShapeMgr;

	void UpdateLoadStatus(int iLoadedShapes, int iTotalShapes) override;
};
