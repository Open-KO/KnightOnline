// N3Joint.h: interface for the CN3Joint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3IJoint_h__INCLUDED_)
#define AFX_N3IJoint_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "N3Transform.h"

#include <list>
class CN3Joint;
typedef std::list<CN3Joint*>::iterator	it_Joint;

const int MAX_JOINT_TRANSFORM = 64;

class CN3Joint : public CN3Transform
{
	friend class CN3Joint; // Position, Rotation, Scale 값들을 자유롭게 다룰수 있어야 한다..
public:
	__Quaternion			m_qOrient;		// Joint Orient Quaternion
	CN3AnimKey				m_KeyOrient;	// Joint Orient 키값... NULL 이면 없는거다..

protected:
	CN3Joint*				m_pParent;
	std::list<CN3Joint*>	m_Children;

public:
	void ChildDelete(CN3Joint* pChild);
#ifdef _N3TOOL
	void CopyExceptAnimationKey(CN3Joint* pJSrc);
	void AddKey(CN3Joint* pJSrc, int nIndexS, int nIndexE);
	void KeyDelete(CN3Joint* pJoint, int nKS, int nKE);
#endif // end of _N3TOOL

	void MatricesGet(__Matrix44 *pMtxs, int& nJointIndex);
	
	void Tick(float fFrm);
	bool TickAnimationKey(float fFrm);
	void ReCalcMatrix();
	void ReCalcMatrixBlended(float fFrm0, float fFrm1, float fWeight0);
	void ParentSet(CN3Joint* pParent);
	void ChildAdd(CN3Joint* pChild);

	CN3Joint* Parent()
	{
		return m_pParent;
	}

	int ChildCount() const
	{
		return static_cast<int>(m_Children.size());
	}

	CN3Joint* Child(int index)
	{
		if (index < 0
			|| index >= static_cast<int>(m_Children.size()))
			return nullptr;

		auto it = m_Children.begin();
		std::advance(it, index);
		return *it;
	}

	void NodeCount(int& nCount);
	BOOL FindPointerByID(int nID, CN3Joint *&pJoint);
#ifdef _N3TOOL
	BOOL FindIndex(const std::string& szName, int& nIndex);
	BOOL FindPointerByName(const std::string& szName, CN3Joint*& pJoint); // 이름을 넣으면 해당 노드의 포인터를 돌려준다..
	void RotSet(const __Quaternion& qtRot) { m_qRot = qtRot; this->ReCalcMatrix(); }
	void RotSet(float x, float y, float z, float w) { m_qRot.x = x; m_qRot.y = y; m_qRot.z = z; m_qRot.w = w; this->ReCalcMatrix(); }
	void Render(const __Matrix44* pMtxParent = NULL, float fUnitSize = 0.1f);
#endif // end of _N3TOOL

	void Release();
	bool Load(HANDLE hFile);
#ifdef _N3TOOL
	bool Save(HANDLE hFile);
#endif // end of _N3TOOL

	CN3Joint();
	virtual ~CN3Joint();

};

#endif // !defined(AFX_N3IJoint_h__INCLUDED_)
