#ifndef SHARED_EXPANDABLECIRCULARBUFFER_H
#define SHARED_EXPANDABLECIRCULARBUFFER_H

#pragma once

#include "CircularBuffer.h"

class ExpandableCircularBuffer : public CircularBuffer
{
public:
	ExpandableCircularBuffer(int size);
	~ExpandableCircularBuffer() override;

	CircularBufferSpan PutData(char* pData, int len) override;

protected:
	// overflow condition 일때 size를 현재의 두배로 늘림
	void BufferResize();
};

#endif // SHARED_EXPANDABLECIRCULARBUFFER_H
