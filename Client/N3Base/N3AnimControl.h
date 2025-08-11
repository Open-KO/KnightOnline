// N3AnimControl.h: interface for the CN3AnimControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_)
#define AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "N3BaseFileAccess.h"
#include <string>
#include <vector>

typedef struct __AnimData
{
	friend CN3AnimControl;
public:
	std::string	szName;

	float	fFrmStart; // 상체 시작
	float	fFrmEnd; // 상체 끝
	float	fFrmPerSec; // 초당 30프레임이 표준이다..

	float	fFrmPlugTraceStart;
	float	fFrmPlugTraceEnd;
	
	float	fFrmSound0;
	float	fFrmSound1;

	float	fTimeBlend; // 다른 동작과 연결시 블렌딩 시간
	int		iBlendFlags; // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
	
	float	fFrmStrike0;
	float	fFrmStrike1;
	
	__AnimData()
	{
		fFrmPerSec = 30.0f; // 초당 30프레임이 표준이다..

		fFrmStart = fFrmEnd = 0;
		fFrmPlugTraceStart = fFrmPlugTraceEnd = 0;
		fFrmSound0 = fFrmSound1 = 0;
		fTimeBlend = 0.25f; // 기본 블렌딩 시간..
		iBlendFlags = 0; // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		fFrmStrike0 = fFrmStrike1 = 0;
	}
	
	void operator = (const __AnimData& other)
	{
		fFrmStart = other.fFrmStart;
		fFrmEnd = other.fFrmEnd;
		fFrmPerSec = other.fFrmPerSec;

		fFrmPlugTraceStart = other.fFrmPlugTraceStart;
		fFrmPlugTraceEnd = other.fFrmPlugTraceEnd;

		fFrmSound0 = other.fFrmSound0;
		fFrmSound1 = other.fFrmSound1;

		fTimeBlend = other.fTimeBlend;
		iBlendFlags = other.iBlendFlags; // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		
		fFrmStrike0 = other.fFrmStrike0;
		fFrmStrike1 = other.fFrmStrike1;

		szName = other.szName;
	}

	void Load(HANDLE hFile)
	{
		if(NULL == hFile || INVALID_HANDLE_VALUE == hFile) return;

		DWORD dwRWC = 0;

		int nL = 0;
		ReadFile(hFile, &nL, 4, &dwRWC, NULL); // 원래는 문자열 포인터가 있던자리이다.. 호환성을 위헤서.. 걍...

		ReadFile(hFile, &fFrmStart, 4, &dwRWC, NULL); // 상체 시작
		ReadFile(hFile, &fFrmEnd, 4, &dwRWC, NULL); // 상체 끝
		ReadFile(hFile, &fFrmPerSec, 4, &dwRWC, NULL); // 초당 30프레임이 표준이다..

		ReadFile(hFile, &fFrmPlugTraceStart, 4, &dwRWC, NULL);
		ReadFile(hFile, &fFrmPlugTraceEnd, 4, &dwRWC, NULL);
		
		ReadFile(hFile, &fFrmSound0, 4, &dwRWC, NULL);
		ReadFile(hFile, &fFrmSound1, 4, &dwRWC, NULL);

		ReadFile(hFile, &fTimeBlend, 4, &dwRWC, NULL);
		ReadFile(hFile, &iBlendFlags, 4, &dwRWC, NULL); // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		
		ReadFile(hFile, &fFrmStrike0, 4, &dwRWC, NULL);
		ReadFile(hFile, &fFrmStrike1, 4, &dwRWC, NULL);

		// 이름 읽기..
		szName = "";
		ReadFile(hFile, &nL, 4, &dwRWC, NULL);
		if(nL > 0)
		{
			std::vector<char> buffer(nL+1, NULL);
			ReadFile(hFile, &buffer[0], nL, &dwRWC, NULL);
			szName = &buffer[0];
		}
	}
	void Save(HANDLE hFile)
	{
		if(NULL == hFile || INVALID_HANDLE_VALUE == hFile) return;

		DWORD dwRWC = 0;

		int nL = 0;
		WriteFile(hFile, &nL, 4, &dwRWC, NULL); // 원래는 문자열 포인터가 있던자리이다.. 호환성을 위헤서.. 걍...

		WriteFile(hFile, &fFrmStart, 4, &dwRWC, NULL); // 상체 시작
		WriteFile(hFile, &fFrmEnd, 4, &dwRWC, NULL); // 상체 끝
		WriteFile(hFile, &fFrmPerSec, 4, &dwRWC, NULL); // 초당 30프레임이 표준이다..

		WriteFile(hFile, &fFrmPlugTraceStart, 4, &dwRWC, NULL);
		WriteFile(hFile, &fFrmPlugTraceEnd, 4, &dwRWC, NULL);
		
		WriteFile(hFile, &fFrmSound0, 4, &dwRWC, NULL);
		WriteFile(hFile, &fFrmSound1, 4, &dwRWC, NULL);

		WriteFile(hFile, &fTimeBlend, 4, &dwRWC, NULL);
		WriteFile(hFile, &iBlendFlags, 4, &dwRWC, NULL); // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		
		WriteFile(hFile, &fFrmStrike0, 4, &dwRWC, NULL);
		WriteFile(hFile, &fFrmStrike1, 4, &dwRWC, NULL);

		// 이름 읽기..
		nL = szName.size();
		WriteFile(hFile, &nL, 4, &dwRWC, NULL);
		if(nL > 0) WriteFile(hFile, szName.c_str(), nL, &dwRWC, NULL);
	}

#ifdef _N3TOOL
	void Offset(float fFrmOffset)
	{
		if(fFrmStart != 0) fFrmStart += fFrmOffset;
		if(fFrmEnd != 0) fFrmEnd += fFrmOffset;

		if(fFrmPlugTraceStart != 0) fFrmPlugTraceStart += fFrmOffset;
		if(fFrmPlugTraceEnd != 0) fFrmPlugTraceEnd += fFrmOffset;

		if(fFrmSound0 != 0) fFrmSound0 += fFrmOffset;
		if(fFrmSound1 != 0) fFrmSound1 += fFrmOffset;

		if(fFrmStrike0 != 0) fFrmStrike0 += fFrmOffset;
		if(fFrmStrike1 != 0) fFrmStrike1 += fFrmOffset;
	}
#endif

} __AnimData;

typedef std::vector<__AnimData>::iterator it_Ani;

class CN3AnimControl : public CN3BaseFileAccess
{
protected:
	std::vector<__AnimData>		m_Datas; // animation Data List

public:
	__AnimData* DataGet(int index)
	{
		if (index < 0
			|| index >= static_cast<int>(m_Datas.size()))
			return nullptr;

		return &m_Datas[index];
	}

	bool Load(HANDLE hFile);

	int Count() const
	{
		return static_cast<int>(m_Datas.size());
	}

#ifdef _N3TOOL
	__AnimData*		DataGet(const std::string& szName)
	{
		int iADC = static_cast<int>(m_Datas.size());
		for(int i = 0; i < iADC; i++)
		{
			if(szName == m_Datas[i].szName) return &(m_Datas[i]);
		}
		return NULL;
	}
	void			Swap(int nAni1, int nAni2);
	void			Delete(int nIndex);
	__AnimData*		Add();
	__AnimData*		Insert(int nIndex);
	bool			Save(HANDLE hFile);
#endif
	void Release();
	
	CN3AnimControl();
	virtual ~CN3AnimControl();

};

#endif // !defined(AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_)
