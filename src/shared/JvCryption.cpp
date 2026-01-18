#include "pch.h"
#include "JvCryption.h"
#include "version.h"

// Cryption
constexpr uint64_t g_private_key = 0x1234567890123456;

void CJvCryption::Init()
{
	m_tkey = m_public_key ^ g_private_key;
}

uint64_t CJvCryption::GenerateKey()
{
#ifdef USE_CRYPTION
	// because of their sucky encryption method, 0 means it effectively won't be encrypted.
	// We don't want that happening...
	do
	{
		// NOTE: Debugging
		m_public_key = RandUInt64(); //0xDCE04F8975278163;
	}
	while (m_public_key == 0);

	Init();
#endif

	return m_public_key;
}

void CJvCryption::JvEncryptionFast(int len, uint8_t* datain, uint8_t* dataout)
{
#ifdef USE_CRYPTION
	const uint8_t* pkey = reinterpret_cast<const uint8_t*>(&m_tkey);
	uint8_t lkey = 0, rsk = 0;
	int rkey = 2157;

	lkey     = (len * 157) & 0xff;

	for (int i = 0; i < len; i++)
	{
		rsk         = (rkey >> 8) & 0xff;
		dataout[i]  = ((datain[i] ^ rsk) ^ pkey[(i % 8)]) ^ lkey;
		rkey       *= 2171;
	}

	return;
#endif

	dataout = datain;
}

int CJvCryption::JvDecryptionWithCRC32(int len, uint8_t* datain, uint8_t* dataout)
{
#ifdef USE_CRYPTION
	int result = -1;
	JvDecryptionFast(len, datain, dataout);

	if (crc32(dataout, len - 4, -1) == *(uint32_t*) (len - 4 + dataout))
		result = len - 4;

	return result;
#else
	dataout = datain;
	return (len - 4);
#endif
}
