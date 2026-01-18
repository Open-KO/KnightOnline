#ifndef SHARED_CIRCULARBUFFER_H
#define SHARED_CIRCULARBUFFER_H

#pragma once

#include <spdlog/spdlog.h>
#include <cassert>

struct CircularBufferSpan
{
	char* Buffer1 = nullptr;
	int Length1   = 0;
	char* Buffer2 = nullptr;
	int Length2   = 0;
};

class CCircularBuffer
{
public:
	inline CCircularBuffer(int size)
	{
		assert(size > 0);
		m_iBufSize = size;
		m_pBuffer  = new char[m_iBufSize];

		m_iHeadPos = 0;
		m_iTailPos = 0;
	}

	inline ~CCircularBuffer()
	{
		assert(m_pBuffer != nullptr);
		delete[] m_pBuffer;
		m_pBuffer = nullptr;
	}

	CircularBufferSpan PutData(char* pData, int len, bool resize = true);
	void GetData(char* pData, int len);
	int GetOutData(char* pData); //HeadPos, 변화

	char& GetHeadData()
	{
		return m_pBuffer[m_iHeadPos];
	}

	// 1 Byte Operation;
	// false : 모든데이터 다빠짐, TRUE: 정상적으로 진행중
	bool HeadIncrease(int increasement = 1);

	void SetEmpty()
	{
		m_iHeadPos = m_iTailPos = 0;
	}

	int& GetBufferSize()
	{
		return m_iBufSize;
	}

	int& GetHeadPos()
	{
		return m_iHeadPos;
	}

	int& GetTailPos()
	{
		return m_iTailPos;
	}

	int GetValidCount() const;

protected:
	// over flow 먼저 점검한 후 IndexOverFlow 점검
	inline bool IsOverFlowCondition(int len) const
	{
		return (len >= m_iBufSize - GetValidCount());
	}

	inline bool IsIndexOverFlow(int len) const
	{
		return (len + m_iTailPos >= m_iBufSize);
	}

	void BufferResize(); //overflow condition 일때 size를 현재의 두배로 늘림

protected:
	int m_iBufSize;
	char* m_pBuffer;

	int m_iHeadPos;
	int m_iTailPos;
};

inline int CCircularBuffer::GetValidCount() const
{
	int count = m_iTailPos - m_iHeadPos;
	if (count < 0)
		count = m_iBufSize + count;
	return count;
}

inline void CCircularBuffer::BufferResize()
{
	int prevBufSize   = m_iBufSize;
	m_iBufSize      <<= 1;
	char* pNewData    = new char[m_iBufSize];
	memcpy(pNewData, m_pBuffer, prevBufSize);

	if (m_iTailPos < m_iHeadPos)
	{
		memcpy(pNewData + prevBufSize, m_pBuffer, m_iTailPos);
		m_iTailPos += prevBufSize;
	}

	delete[] m_pBuffer;
	m_pBuffer = pNewData;
}

inline CircularBufferSpan CCircularBuffer::PutData(char* pData, int len, bool resize /*= true*/)
{
	CircularBufferSpan span {};

	if (len <= 0)
	{
		spdlog::error("CircularBuffer::PutData len is <= 0");
		return span;
	}

	if (resize)
	{
		while (IsOverFlowCondition(len))
			BufferResize();
	}
	else
	{
		if (IsOverFlowCondition(len))
			return span;
	}

	if (IsIndexOverFlow(len))
	{
		int FirstCopyLen  = m_iBufSize - m_iTailPos;
		int SecondCopyLen = len - FirstCopyLen;
		assert(FirstCopyLen);

		span.Buffer1 = &m_pBuffer[m_iTailPos];
		span.Length1 = FirstCopyLen;

		memcpy(span.Buffer1, pData, FirstCopyLen);

		if (SecondCopyLen > 0)
		{
			span.Buffer2 = m_pBuffer;
			span.Length2 = SecondCopyLen;

			memcpy(span.Buffer2, pData + FirstCopyLen, SecondCopyLen);
			m_iTailPos = SecondCopyLen;
		}
		else
		{
			m_iTailPos = 0;
		}
	}
	else
	{
		span.Buffer1 = &m_pBuffer[m_iTailPos];
		span.Length1 = len;

		memcpy(span.Buffer1, pData, len);
		m_iTailPos += len;
	}

	return span;
}

inline int CCircularBuffer::GetOutData(char* pData)
{
	int len = GetValidCount();
	int fc  = m_iBufSize - m_iHeadPos;
	if (len > fc)
	{
		int sc = len - fc;
		memcpy(pData, m_pBuffer + m_iHeadPos, fc);
		memcpy(pData + fc, m_pBuffer, sc);
		m_iHeadPos = sc;
		assert(m_iHeadPos == m_iTailPos);
	}
	else
	{
		memcpy(pData, m_pBuffer + m_iHeadPos, len);
		m_iHeadPos += len;
		if (m_iHeadPos == m_iBufSize)
			m_iHeadPos = 0;
	}
	return len;
}

inline void CCircularBuffer::GetData(char* pData, int len)
{
	assert(len > 0 && len <= GetValidCount());
	if (len < m_iBufSize - m_iHeadPos)
	{
		memcpy(pData, m_pBuffer + m_iHeadPos, len);
	}
	else
	{
		int fc = m_iBufSize - m_iHeadPos;
		int sc = len - fc;
		memcpy(pData, m_pBuffer + m_iHeadPos, fc);
		if (sc > 0)
			memcpy(pData + fc, m_pBuffer, sc);
	}
}

inline bool CCircularBuffer::HeadIncrease(int increasement)
{
	assert(increasement <= GetValidCount());
	m_iHeadPos += increasement;
	m_iHeadPos %= m_iBufSize;
	return m_iHeadPos != m_iTailPos;
}

#endif // SHARED_CIRCULARBUFFER_H
