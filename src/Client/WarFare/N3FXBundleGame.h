// N3FXBundleGame.h: interface for the CN3FXBundleGame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3FXBUNDLEGAME_H__1B6BEFC3_79A4_4FFE_A48B_475D1F07E26A__INCLUDED_)
#define AFX_N3FXBUNDLEGAME_H__1B6BEFC3_79A4_4FFE_A48B_475D1F07E26A__INCLUDED_

#pragma once

#include <N3Base/N3FXBundle.h>

class CN3FXBundleGame : public CN3FXBundle
{
public:
	int m_iID; //겜에서 쓰는 아디..
	int m_iIdx;
	bool m_bRegion;

	//포물선 운동을 위한...싸인곡선 이용함다..
	float m_fDistance;
	float m_fHeight;

public:
	void SetPreBundlePos(int iSourceID, int iJoint);
	void Trigger(int iSourceID = 0, int iTargetID = 0, int iTargetJoint = 0, int iSndID = -1) override;
	void Trigger(int iSourceID, const __Vector3& TargetPos, int iSndID = -1);
	bool Tick() override;

	void Duplicate(CN3FXBundleGame* pDestBundle);
	CN3FXPartBase* AllocatePart(int iPartType) const override;

	CN3FXBundleGame();
	~CN3FXBundleGame() override;
};

#endif // !defined(AFX_N3FXBUNDLEGAME_H__1B6BEFC3_79A4_4FFE_A48B_475D1F07E26A__INCLUDED_)
