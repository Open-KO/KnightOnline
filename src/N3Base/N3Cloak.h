// N3Cloak.h: interface for the CN3Cloak class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3CLOAK_H__3ED1C9F5_2F40_47B7_8CEC_6881E0D9FAEE__INCLUDED_)
#define AFX_N3CLOAK_H__3ED1C9F5_2F40_47B7_8CEC_6881E0D9FAEE__INCLUDED_

#pragma once

#include "N3Base.h"

inline constexpr int CLOAK_MAX_WIDTH  = 7;
inline constexpr int CLOAK_MAX_HEIGHT = 7;

// 망토에서 장식용 버텍스가 들어가 있는 라인수. 이부분은 이동이 없다. ok?
inline constexpr int CLOAK_SKIP_LINE  = 2;

enum e_Cloak_AnchorMovePattern : uint8_t
{
	AMP_NONE = 0,
	AMP_YAWCCW,
	AMP_YAWCW,
	AMP_MOVEX,
	AMP_MOVEY,
	AMP_MOVEZ,
	AMP_MOVEXZ,
	AMP_MOVEXZ2
};

enum e_CloakMove : uint8_t
{
	CLOAK_MOVE_STOP = 0,
	CLOAK_MOVE_WALK,
	CLOAK_MOVE_RUN,
	CLOAK_MOVE_WALK_BACKWARD,
};

class CPlayerBase;
class CN3Chr;
class CN3CPlug_Cloak;
class CN3Cloak : public CN3Base
{
public:
	CN3Cloak();
	~CN3Cloak() override;

	class __Particle
	{
	public:
		__Particle()
		{
			x = y = z = 0.0f;
			vx = vy = vz = 0.0f;
			LocalForce   = {};
			mass         = 0.0f;
		}

		~__Particle()
		{
		}

		float x, y, z;
		float vx, vy, vz;
		__Vector3 LocalForce;
		float mass;
		void Set(float mass1, float x1, float y1, float z1, float vx1, float vy1, float vz1)
		{
			mass = mass1;
			x = x1, y = y1, z = z1;
			vx = vx1, vy = vy1, vz = vz1;
		}
	};

	void Init(CN3CPlug_Cloak* pPlugCloak);
	void SetLOD(int nLevel);
	void ApplyOffset(__Vector3& vDif);

protected:
	//	Anchor
	e_Cloak_AnchorMovePattern m_eAnchorPattern;
	float m_fAnchorPreserveTime;
	__Vector3 m_vOffset[CLOAK_MAX_WIDTH];

	void RestoreAnchorLine();
	void MoveAnchorLine(e_Cloak_AnchorMovePattern eType, float fPreserveTime);

	//
	CN3Texture* m_pTex;
	__VertexT1* m_pVertex;
	uint16_t* m_pIndex;
	int m_nVertexCount, m_nIndexCount;

	int m_nGridW, m_nGridH;
	int m_nLOD;

	CN3PMesh* m_pPMesh;
	float m_fOffsetRecoveryTime;
	float m_fPrevYaw;

	__Particle* m_pParticle;
	__Vector3 m_GravityForce; // 중력(.y)가 항상있어야 변형이 일어나지 않는다..
	__Vector3 m_Force;        // 외부에서 가해지는 힘.

	void UpdateLocalForce();
	void ApplyForce();
	void TickYaw(float fYaw);
	void TickByPlayerMotion(e_CloakMove eCurMove);

public:
	virtual void Tick(int nLOD, float fYaw, e_CloakMove eCurMove);
	virtual void Render(__Matrix44& mtx);
	void Release() override;
};

#endif // !defined(AFX_N3CLOAK_H__3ED1C9F5_2F40_47B7_8CEC_6881E0D9FAEE__INCLUDED_)
