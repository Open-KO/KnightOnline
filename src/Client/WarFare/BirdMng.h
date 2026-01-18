// BirdMng.h: interface for the CBirdMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_)
#define AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>
#include "Bird.h"

class CBirdMng : public CN3Base
{
public:
	CBirdMng();
	~CBirdMng() override;

	// Attributes
protected:
	std::vector<CBird> _birds;

	// Operations
public:
	void Release() override;
	void Tick();
	void Render();
	void LoadFromFile(const std::string& szFN);
};

#endif // !defined(AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_)
