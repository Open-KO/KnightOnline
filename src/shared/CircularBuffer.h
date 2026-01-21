#ifndef SHARED_CIRCULARBUFFER_H
#define SHARED_CIRCULARBUFFER_H

#pragma once

#include <vector>

struct CircularBufferSpan
{
	char* Buffer1 = nullptr;
	int Length1   = 0;
	char* Buffer2 = nullptr;
	int Length2   = 0;
};

class CircularBuffer
{
public:
	CircularBuffer(int size);
	virtual ~CircularBuffer();

	inline int GetBufferSize() const
	{
		return static_cast<int>(_buffer.size());
	}

	virtual CircularBufferSpan PutData(char* pData, int len);
	void GetData(char* pData, int len);

	// 1 Byte Operation;
	// false : 모든데이터 다빠짐, TRUE: 정상적으로 진행중
	bool HeadIncrease(int increasement = 1);

	void SetEmpty();
	int GetValidCount() const;

protected:
	// over flow 먼저 점검한 후 IndexOverFlow 점검
	bool IsOverFlowCondition(int len) const;
	bool IsIndexOverFlow(int len) const;

protected:
	std::vector<char> _buffer;

	int _headPos;
	int _tailPos;
};

#endif // SHARED_CIRCULARBUFFER_H
