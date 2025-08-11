// N3ShapeExtra.h: interface for the CN3ShapeExtra class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SHAPEEXTRA_H__F3D6E8DF_A20F_45F5_8729_2FD36796969C__INCLUDED_)
#define AFX_N3SHAPEEXTRA_H__F3D6E8DF_A20F_45F5_8729_2FD36796969C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "N3Shape.h"
#include <vector>

// std::vector<struct __Rotation>::iterator it_Rot;

class CN3ShapeExtra : public CN3Shape  
{
protected:
	struct __Rotation
	{
		float			fRadianPerSec;	// 초당 회전.. Radian...
		__Vector3		vAxis;			// 회전축..
		float			fRadianCur;		// 현재 회전값..
		float			fRadianToReach;	// 회전시킬값..

		__Rotation()
		{
			memset(this, 0, sizeof(__Rotation));
			vAxis.Set(0,1,0);
		}
	};

	std::vector<__Rotation>	m_Rotations;

public:
	void RotateTo(int iPart, const __Vector3& vAxis, float fRadianToReach, float fRadianPerSec, bool bImmediately = false); // 원하는 파트를 축에 따라 지정한 각도까지 지정한 속도로 회전시킨다..

	bool Load(HANDLE hFile);
	void Tick(float fFrm);
	
	void Release();
	CN3ShapeExtra();
	virtual ~CN3ShapeExtra();

};

#endif // !defined(AFX_N3SHAPEEXTRA_H__F3D6E8DF_A20F_45F5_8729_2FD36796969C__INCLUDED_)
