// N3AnimControl.h: interface for the CN3AnimControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_)
#define AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_

#pragma once

#include "N3BaseFileAccess.h"
#include <string>
#include <vector>

typedef struct __AnimData
{
	friend CN3AnimControl;

public:
	std::string szName;

	float fFrmStart;  // 상체 시작
	float fFrmEnd;    // 상체 끝
	float fFrmPerSec; // 초당 30프레임이 표준이다..

	float fFrmPlugTraceStart;
	float fFrmPlugTraceEnd;

	float fFrmSound0;
	float fFrmSound1;

	float fTimeBlend; // 다른 동작과 연결시 블렌딩 시간
	int iBlendFlags;  // 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연

	float fFrmStrike0;
	float fFrmStrike1;

	__AnimData()
	{
		fFrmPerSec         = 30.0f; // 초당 30프레임이 표준이다..
		fFrmStart          = 0.0f;
		fFrmEnd            = 0.0f;
		fFrmPlugTraceStart = 0.0f;
		fFrmPlugTraceEnd   = 0.0f;
		fFrmSound0         = 0.0f;
		fFrmSound1         = 0.0f;
		fTimeBlend         = 0.25f; // 기본 블렌딩 시간..

		// 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		iBlendFlags        = 0;
		fFrmStrike0        = 0.0f;
		fFrmStrike1        = 0.0f;
	}

	__AnimData& operator=(const __AnimData& other)
	{
		if (this == &other)
			return *this;

		fFrmStart          = other.fFrmStart;
		fFrmEnd            = other.fFrmEnd;
		fFrmPerSec         = other.fFrmPerSec;

		fFrmPlugTraceStart = other.fFrmPlugTraceStart;
		fFrmPlugTraceEnd   = other.fFrmPlugTraceEnd;

		fFrmSound0         = other.fFrmSound0;
		fFrmSound1         = other.fFrmSound1;

		fTimeBlend         = other.fTimeBlend;

		// 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		iBlendFlags        = other.iBlendFlags;

		fFrmStrike0        = other.fFrmStrike0;
		fFrmStrike1        = other.fFrmStrike1;

		szName             = other.szName;

		return *this;
	}

	void Load(File& file)
	{
		int nL = 0;
		file.Read(&nL, 4);         // 원래는 문자열 포인터가 있던자리이다.. 호환성을 위헤서.. 걍...

		file.Read(&fFrmStart, 4);  // 상체 시작
		file.Read(&fFrmEnd, 4);    // 상체 끝
		file.Read(&fFrmPerSec, 4); // 초당 30프레임이 표준이다..

		file.Read(&fFrmPlugTraceStart, 4);
		file.Read(&fFrmPlugTraceEnd, 4);

		file.Read(&fFrmSound0, 4);
		file.Read(&fFrmSound1, 4);

		file.Read(&fTimeBlend, 4);

		// 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		file.Read(&iBlendFlags, 4);

		file.Read(&fFrmStrike0, 4);
		file.Read(&fFrmStrike1, 4);

		// 이름 읽기..
		file.Read(&nL, 4);
		if (nL > 0)
		{
			szName.assign(nL, '\0');
			file.Read(&szName[0], nL);
		}
		else
		{
			szName.clear();
		}
	}

	void Save(File& file)
	{
		int nL = 0;
		file.Write(&nL, 4);         // 원래는 문자열 포인터가 있던자리이다.. 호환성을 위헤서.. 걍...

		file.Write(&fFrmStart, 4);  // 상체 시작
		file.Write(&fFrmEnd, 4);    // 상체 끝
		file.Write(&fFrmPerSec, 4); // 초당 30프레임이 표준이다..

		file.Write(&fFrmPlugTraceStart, 4);
		file.Write(&fFrmPlugTraceEnd, 4);

		file.Write(&fFrmSound0, 4);
		file.Write(&fFrmSound1, 4);

		file.Write(&fTimeBlend, 4);

		// 블렌딩 플래그 0 이면 걍 블렌딩.. 1이면 루핑시 블렌딩 타임만큼 시간 지연
		file.Write(&iBlendFlags, 4);

		file.Write(&fFrmStrike0, 4);
		file.Write(&fFrmStrike1, 4);

		// 이름 읽기..
		nL = static_cast<int>(szName.size());
		file.Write(&nL, 4);
		if (nL > 0)
			file.Write(szName.c_str(), nL);
	}

#ifdef _N3TOOL
	void Offset(float fFrmOffset)
	{
		if (fFrmStart != 0)
			fFrmStart += fFrmOffset;

		if (fFrmEnd != 0)
			fFrmEnd += fFrmOffset;

		if (fFrmPlugTraceStart != 0)
			fFrmPlugTraceStart += fFrmOffset;

		if (fFrmPlugTraceEnd != 0)
			fFrmPlugTraceEnd += fFrmOffset;

		if (fFrmSound0 != 0)
			fFrmSound0 += fFrmOffset;

		if (fFrmSound1 != 0)
			fFrmSound1 += fFrmOffset;

		if (fFrmStrike0 != 0)
			fFrmStrike0 += fFrmOffset;

		if (fFrmStrike1 != 0)
			fFrmStrike1 += fFrmOffset;
	}
#endif

} __AnimData;

typedef std::vector<__AnimData>::iterator it_Ani;

class CN3AnimControl : public CN3BaseFileAccess
{
protected:
	std::vector<__AnimData> m_Datas; // animation Data List

public:
	__AnimData* DataGet(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_Datas.size()))
			return nullptr;

		return &m_Datas[index];
	}

	bool Load(File& file) override;

	int Count() const
	{
		return static_cast<int>(m_Datas.size());
	}

#ifdef _N3TOOL
	__AnimData* DataGet(const std::string& szName)
	{
		int iADC = static_cast<int>(m_Datas.size());
		for (int i = 0; i < iADC; i++)
		{
			if (szName == m_Datas[i].szName)
				return &m_Datas[i];
		}
		return nullptr;
	}

	void Swap(int nAni1, int nAni2);
	void Delete(int nIndex);
	__AnimData* Add();
	__AnimData* Insert(int nIndex);
	bool Save(File& file) override;
#endif
	void Release() override;

	CN3AnimControl();
	~CN3AnimControl() override;
};

#endif // !defined(AFX_N3ANIMCONTROL_H__37E9A095_FF76_4DD5_95A2_4CA1ABC227B0__INCLUDED_)
