// LightMgr.h: interface for the CLightMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__LIGHTMGR_H__)
#define __LIGHTMGR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Light.h>

#include <list>

const float LIGHT_VALIDRANGE = 100.0f;
enum eIdxLight {LGT_DEFAULT0 = 0, LGT_DEFAULT1 = 1, LGT_DEFAULT2 = 2, LGT_ADDITIONAL0 = 3, LGT_ADDITIONAL1 = 4,
				LGT_ADDITIONAL2 = 5, LGT_ADDITIONAL3 = 6, LGT_ADDITIONAL4 = 7, LGT_MAX = 8 };

class CLightMgr : public CN3Base  
{
public:
	std::list<CN3Light*>	m_Lights;
	CN3Light*				m_pActiveLight[LGT_MAX];

public:
	void		Release();
	void		Tick();
	void		AddLight(CN3Light* pLgt);
	CN3Light*	Light(int idx) { return m_pActiveLight[idx]; }

	void		LoadZoneLight(const char* szFN);
	
	CLightMgr();
	virtual ~CLightMgr();
};

#endif // #if !defined(__LIGHTMGR_H__)
