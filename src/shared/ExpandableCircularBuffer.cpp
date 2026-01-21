#include "pch.h"
#include "ExpandableCircularBuffer.h"

#include <spdlog/spdlog.h>
#include <cstring> // std::memcpy()

ExpandableCircularBuffer::ExpandableCircularBuffer(int size) : CircularBuffer(size)
{
}

ExpandableCircularBuffer::~ExpandableCircularBuffer()
{
}

CircularBufferSpan ExpandableCircularBuffer::PutData(char* pData, int len)
{
	CircularBufferSpan span {};

	if (len <= 0)
	{
		spdlog::error("ExpandableCircularBuffer::PutData len is <= 0");
		return span;
	}

	while (IsOverFlowCondition(len))
		BufferResize();

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

// overflow condition 일때 size를 현재의 두배로 늘림
void ExpandableCircularBuffer::BufferResize()
{
	int prevSize = GetBufferSize();
	int newSize  = prevSize << 1;

	_buffer.resize(newSize);

	if (_tailPos < _headPos)
	{
		std::memcpy(&_buffer[prevSize], _buffer.data(), _tailPos);
		_tailPos += prevSize;
	}
}
