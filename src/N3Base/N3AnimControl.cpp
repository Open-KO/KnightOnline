// N3AnimControl.cpp: implementation of the CN3AnimControl class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3AnimControl.h"

CN3AnimControl::CN3AnimControl()
{
	m_dwType |= OBJ_ANIM_CONTROL;
}

CN3AnimControl::~CN3AnimControl()
{
}

void CN3AnimControl::Release()
{
	m_Datas.clear(); // animation Data List

	CN3BaseFileAccess::Release();
}

bool CN3AnimControl::Load(File& file)
{
	if (!m_Datas.empty())
		Release();

	int nCount = 0;
	file.Read(&nCount, 4);

	m_Datas.clear(); // animation Data List
	for (int i = 0; i < nCount; i++)
	{
		__AnimData Data;
		Data.Load(file);
		m_Datas.push_back(Data);
	}

	return true;
}

#ifdef _N3TOOL
bool CN3AnimControl::Save(File& file)
{
	int nL    = 0;
	int iSize = static_cast<int>(m_Datas.size());
	file.Write(&iSize, 4);

	for (int i = 0; i < iSize; i++)
		m_Datas[i].Save(file);

	return true;
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
__AnimData* CN3AnimControl::Add()
{
	__AnimData Data;
	Data.szName = "No Name";
	m_Datas.push_back(Data);

	return &(m_Datas[m_Datas.size() - 1]);
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
__AnimData* CN3AnimControl::Insert(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(m_Datas.size()))
		return nullptr;

	it_Ani it = m_Datas.begin();
	for (int i = 0; i < nIndex; i++, it++)
		;

	__AnimData Data;
	Data.szName = "No Name";
	it          = m_Datas.insert(it, Data);

	return &(*it);
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3AnimControl::Swap(int nAni1, int nAni2)
{
	if (nAni1 == nAni2)
		return;

	if (nAni1 < 0 || nAni1 >= static_cast<int>(m_Datas.size()))
		return;

	if (nAni2 < 0 || nAni2 >= static_cast<int>(m_Datas.size()))
		return;

	__AnimData Tmp = m_Datas[nAni2];
	m_Datas[nAni2] = m_Datas[nAni1];
	m_Datas[nAni1] = Tmp;
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3AnimControl::Delete(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(m_Datas.size()))
		return;

	it_Ani it = m_Datas.begin();
	for (int i = 0; i < nIndex; i++, it++)
		;

	m_Datas.erase(it);
}
#endif // endof #ifdef _N3TOOL
