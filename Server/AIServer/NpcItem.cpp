// NpcItem.cpp: implementation of the CNpcItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NpcItem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcItem::CNpcItem()
{
	m_nRow = 0;
	m_nField = 0;
	m_ppItem = nullptr;
}

CNpcItem::~CNpcItem()
{
	if (m_ppItem != nullptr)
	{
		for (int i = 0; i < m_nField; i++)
			delete[] m_ppItem[i];
		delete[] m_ppItem;
	}
}
