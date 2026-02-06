#include "pch.h"
#include "CircularBuffer.h"

#include <spdlog/spdlog.h>
#include <cassert>
#include <cstring> // std::memcpy()

CircularBuffer::CircularBuffer(int size)
{
	assert(size > 0);
	_buffer.resize(size);
	_headPos = 0;
	_tailPos = 0;
}

CircularBuffer::~CircularBuffer()
{
}

CircularBufferSpan CircularBuffer::PutData(char* pData, int len)
{
	CircularBufferSpan span {};

	if (len <= 0)
	{
		spdlog::error("CircularBuffer::PutData len is <= 0");
		return span;
	}

	if (IsOverFlowCondition(len))
		return span;

	if (IsIndexOverFlow(len))
	{
		int FirstCopyLen  = GetBufferSize() - _tailPos;
		int SecondCopyLen = len - FirstCopyLen;
		assert(FirstCopyLen);

		span.Buffer1 = &_buffer[_tailPos];
		span.Length1 = FirstCopyLen;

		std::memcpy(span.Buffer1, pData, FirstCopyLen);

		if (SecondCopyLen > 0)
		{
			span.Buffer2 = &_buffer[0];
			span.Length2 = SecondCopyLen;

			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			std::memcpy(span.Buffer2, &pData[FirstCopyLen], SecondCopyLen);
			_tailPos = SecondCopyLen;
		}
		else
		{
			_tailPos = 0;
		}
	}
	else
	{
		span.Buffer1 = &_buffer[_tailPos];
		span.Length1 = len;

		std::memcpy(span.Buffer1, pData, len);
		_tailPos += len;
	}

	return span;
}

void CircularBuffer::GetData(char* pData, int len)
{
	assert(len > 0 && len <= GetValidCount());
	if (len < GetBufferSize() - _headPos)
	{
		std::memcpy(pData, &_buffer[_headPos], len);
	}
	else
	{
		int fc = GetBufferSize() - _headPos;
		int sc = len - fc;
		std::memcpy(pData, &_buffer[_headPos], fc);

		if (sc > 0)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			std::memcpy(&pData[fc], &_buffer[0], sc);
		}
	}
}

// 1 Byte Operation;
// false : 모든데이터 다빠짐, TRUE: 정상적으로 진행중
bool CircularBuffer::HeadIncrease(int increasement /*= 1*/)
{
	assert(increasement <= GetValidCount());
	_headPos += increasement;
	_headPos %= GetBufferSize();
	return _headPos != _tailPos;
}

void CircularBuffer::SetEmpty()
{
	_headPos = _tailPos = 0;
}

int CircularBuffer::GetValidCount() const
{
	int count = _tailPos - _headPos;
	if (count < 0)
		count = GetBufferSize() + count;
	return count;
}

// over flow 먼저 점검한 후 IndexOverFlow 점검
bool CircularBuffer::IsOverFlowCondition(int len) const
{
	return (len >= GetBufferSize() - GetValidCount());
}

bool CircularBuffer::IsIndexOverFlow(int len) const
{
	return (len + _tailPos >= GetBufferSize());
}
