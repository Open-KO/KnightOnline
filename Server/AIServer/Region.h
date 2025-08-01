// Region.h: interface for the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__F79F27E6_C1E6_4ED5_904A_934AA8163C88__INCLUDED_)
#define AFX_REGION_H__F79F27E6_C1E6_4ED5_904A_934AA8163C88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <shared/STLMap.h>

class CUser;
class CNpc;


typedef CSTLMap <int>			ZoneUserArray;
typedef CSTLMap <int>			ZoneNpcArray;

//typedef CTypedPtrArray <CPtrArray, int*>	ZoneUserArray;
//typedef CTypedPtrArray <CPtrArray, int*>	ZoneNpcArray;

class CRegion
{
public:
	ZoneUserArray	m_RegionUserArray;
	ZoneNpcArray	m_RegionNpcArray;
	BYTE	m_byMoving;			// move : 1, not moving : 0

protected:
	int		m_nIndex;

public:
	CRegion();
	virtual ~CRegion();

};

#endif // !defined(AFX_REGION_H__F79F27E6_C1E6_4ED5_904A_934AA8163C88__INCLUDED_)
