// UILoading.h: interface for the UILoading class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UILoading_H__81E8BA13_2261_4A3D_9A94_BF0E7C49C4DD__INCLUDED_)
#define AFX_UILoading_H__81E8BA13_2261_4A3D_9A94_BF0E7C49C4DD__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUILoading : public CN3UIBase
{
protected:
	CN3UIString* m_pText_Version;
	CN3UIString* m_pText_Info;
	CN3UIProgress* m_pProgress_Loading;

public:
	bool Load(File& file) override;
	CUILoading();
	~CUILoading() override;

	// NOLINTNEXTLINE(clang-diagnostic-overloaded-virtual)
	virtual void Render(const std::string& szInfo, int iPercentage);
	void Release() override;
};

#endif // !defined(AFX_UILoading_H__81E8BA13_2261_4A3D_9A94_BF0E7C49C4DD__INCLUDED_)
