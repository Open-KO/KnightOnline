// CircularBuffer.cpp: implementation of the CCircularBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "Ebenezer.h"
#include "CircularBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCircularBuffer::CCircularBuffer(int size)
{
	ASSERT(size > 0);
	m_iBufSize = size;
	m_pBuffer = new char[m_iBufSize];

	m_iHeadPos = 0;
	m_iTailPos = 0;
}

CCircularBuffer::~CCircularBuffer()
{
	ASSERT(m_pBuffer);
	delete[] m_pBuffer;
	m_pBuffer = nullptr;
}
