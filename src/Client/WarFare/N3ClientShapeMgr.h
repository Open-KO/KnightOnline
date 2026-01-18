#ifndef CLIENT_WARFARE_N3CLIENTSHAPEMGR_H
#define CLIENT_WARFARE_N3CLIENTSHAPEMGR_H

#pragma once

#include <N3Base/N3ShapeMgr.h>

class CN3ClientShapeMgr : public CN3ShapeMgr
{
public:
	using CN3ShapeMgr::CN3ShapeMgr;

	void UpdateLoadStatus(int iLoadedShapes, int iTotalShapes) override;
};

#endif // CLIENT_WARFARE_N3CLIENTSHAPEMGR_H
