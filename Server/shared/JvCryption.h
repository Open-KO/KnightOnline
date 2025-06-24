#pragma once

#define USE_CRYPTION

#include "crc32.h"

class CJvCryption
{
private:
	uint64_t m_public_key, m_tkey;

public:
	inline CJvCryption()
		: m_public_key(0)
	{
	}

	inline uint64_t GetPublicKey() const {
		return m_public_key;
	}

	inline void SetPublicKey(uint64_t key) {
		m_public_key = key;
	}

	uint64_t GenerateKey();

	void Init();

	void JvEncryptionFast(int len, uint8_t* datain, uint8_t* dataout);

	void JvDecryptionFast(int len, uint8_t* datain, uint8_t* dataout) {
		JvEncryptionFast(len, datain, dataout);
	}

	int JvDecryptionWithCRC32(int len, uint8_t* datain, uint8_t* dataout);
};
