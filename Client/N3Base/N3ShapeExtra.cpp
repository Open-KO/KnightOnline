// N3ShapeExtra.cpp: implementation of the CN3ShapeExtra class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3ShapeExtra.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CN3ShapeExtra::CN3ShapeExtra()
{
	m_dwType |= OBJ_SHAPE_EXTRA;
}

CN3ShapeExtra::~CN3ShapeExtra()
{
	m_Rotations.clear();
}

void CN3ShapeExtra::Release()
{
	CN3Shape::Release();
	
	m_Rotations.clear();
}

bool CN3ShapeExtra::Load(HANDLE hFile)
{
	bool bSuccess = CN3Shape::Load(hFile);

	int iPC = m_Parts.size();
	m_Rotations.clear();
	if(iPC <= 0) return bSuccess;

	m_Rotations.assign(iPC, __Rotation());

	return bSuccess;
}

void CN3ShapeExtra::Tick(float fFrm)
{
	if(false == m_bVisible) 
	{
		m_bDontRender = true;
		return; // 강제로 렌더링 하지 않는다.
	}

	CN3Shape::Tick();
	if(m_bDontRender) return;
	if(m_Parts.empty()) return;

	bool bNeedRemakeCollisionMeshes = false;
	int iPC = m_Parts.size();
	float fDir = 0, fRotDelta = 0;
	__Rotation* pRot = NULL;
	__Quaternion qRot;
	CN3SPart* pPart = NULL;
	for(int i = 0; i < iPC; i++)
	{
		pPart = m_Parts[i];
		if(pPart->m_bOutOfCameraRange) continue;

		__Rotation* pRot = &(m_Rotations[i]);
		if(	pRot->fRadianPerSec == 0 || 
			pRot->fRadianCur == pRot->fRadianToReach) continue;
		(pRot->fRadianCur < pRot->fRadianToReach) ? fDir = 1.0f : fDir = -1.0f; // 도는 방향..
		
		fRotDelta = pRot->fRadianPerSec * fDir * CN3Base::s_fSecPerFrm;
		pRot->fRadianCur += fRotDelta;
		if(std::abs(pRot->fRadianToReach - pRot->fRadianCur) <= fRotDelta) /// 원하는 곳까지 다 열렸다!!
		{
			bNeedRemakeCollisionMeshes = true;
			pRot->fRadianPerSec = 0;
			pRot->fRadianCur = pRot->fRadianToReach;
		}

		qRot.RotationAxis(pRot->vAxis, pRot->fRadianCur);
		pPart->m_Matrix = qRot;
		pPart->m_Matrix.PosSet(pPart->m_vPivot);
		pPart->m_Matrix *= m_Matrix;
	}

	if(bNeedRemakeCollisionMeshes) 
		this->MakeCollisionMeshByParts(); // 충돌메시를 다시 만든다..
}

void CN3ShapeExtra::RotateTo(int iPart, const __Vector3& vAxis, float fRadianToReach, float fRadianPerSec, bool bImmediately)
{
	if (iPart < 0
		|| iPart >= static_cast<int>(m_Rotations.size()))
		return;

	__Rotation* pRot = &(m_Rotations[iPart]);
	
	pRot->fRadianToReach = fRadianToReach;
	pRot->fRadianPerSec = fRadianPerSec;
	pRot->vAxis = vAxis;
	if(bImmediately) pRot->fRadianCur = pRot->fRadianToReach - 0.01f; // 이러면 직빵으로 열린다.. 약간 값을 빼주는 이유는 한번은 틱을 돌게 하기 위해서이다.
}
