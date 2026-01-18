#if !defined(AFX_Bitset_h__INCLUDED_)
#define AFX_Bitset_h__INCLUDED_

#pragma once

class CBitset
{
public:
	// Initialize all the data members
	CBitset() : m_bits(nullptr), m_size(0)
	{
	}

	// This is our deconstructor
	~CBitset()
	{
		// If we have valid memory, get rid of it
		delete[] m_bits;
		m_bits = nullptr;
	}

	// This resizes our bitset to a size so each face has a bit associated with it
	void Resize(int count)
	{
		// Get the size of integers we need
		m_size = count / 32 + 1;

		// Make sure we haven't already allocated memory for the bits
		delete[] m_bits;
		m_bits = nullptr;

		// Allocate the bits and initialize them
		m_bits = new uint32_t[m_size];
		ClearAll();
	}

	// This does the binary math to set the desired bit
	void Set(int i)
	{
		m_bits[i >> 5] |= (1 << (i & 31));
	}

	// This returns if the desired bit slot is a 1 or a 0
	int On(int i)
	{
		return m_bits[i >> 5] & (1 << (i & 31));
	}

	// This clears a bit to 0
	void Clear(int i)
	{
		m_bits[i >> 5] &= ~(1 << (i & 31));
	}

	// This initializes the bits to 0
	void ClearAll()
	{
		memset(m_bits, 0, sizeof(uint32_t) * m_size);
	}

private:
	// Our private bit data that holds the bits and size
	uint32_t* m_bits;
	int m_size;
};

#endif // !defined(AFX_Bitset_h__INCLUDED_)
