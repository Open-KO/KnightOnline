#ifndef CLIENT_N3BASE_WAVEFORMAT_H
#define CLIENT_N3BASE_WAVEFORMAT_H

#pragma once

#include <inttypes.h> // uint32_t, uint16_t

#pragma pack(push, 1)
/// \brief Represents the 'data' chunk of a WAV file.
/// \details Contains the raw audio sample data size in bytes.
struct WAVE_Data
{
	/// Chunk ID, always "data".
	char		ID[4]	= { 'd', 'a', 't', 'a' };

	/// Size of the audio data in bytes (NumSamples * NumChannels * (BitsPerSample / 8)).
	uint32_t	Size	= 0;
};

/// \brief Represents the 'fmt ' chunk of a WAV file.
/// \details Contains format information such as sample rate, channel count, and bit depth.
struct WAVE_Format
{
	/// Chunk ID, always "fmt ".
	char		ID[4]			= { 'f', 'm', 't', ' ' };

	/// Size of the format chunk, not including ID and Size fields (8 bytes).
	uint32_t	Size			= sizeof(WAVE_Format) - 8;

	/// Audio format code (1 = PCM integer, 3 = IEEE 754 float).
	uint16_t	AudioFormat		= 1;

	/// Number of audio channels.
	uint16_t	NumChannels		= 1;

	/// Sample rate in hertz (e.g., 44100 Hz).
	uint32_t	SampleRate		= 44100;

	/// Bytes per second (SampleRate * BytesPerBlock).
	uint32_t	BytesPerSec		= 0;

	/// Number of bytes per audio block (NumChannels * (BitsPerSample / 8)).
	uint16_t	BytesPerBlock	= 0;

	/// Number of bits per sample (e.g., 16, 24, 32).
	uint16_t	BitsPerSample	= 0;
};

/// \brief Represents the RIFF header of a WAV file.
/// \details Contains file type and overall file size information.
struct RIFF_Header
{
	/// File type identifier, always "RIFF".
	char			FileTypeID[4];	// 'R', 'I', 'F', 'F'

	/// Overall file size in bytes, not including FileTypeID and FileSize fields (8 bytes).
	uint32_t		FileSize;

	/// Format identifier, always "WAVE".
	char			FileFormatID[4]; // 'W', 'A', 'V', 'E'
};

/// \brief Represents a generic RIFF sub-chunk header.
/// \details Can be used for any sub-chunk like 'fmt ' or 'data'.
struct RIFF_SubChunk
{
	/// Sub-chunk ID (e.g., "fmt ", "data").
	char			SubChunkID[4];

	/// Size of the sub-chunk data in bytes.
	uint32_t		SubChunkSize;
};
#pragma pack(pop)

#endif // CLIENT_N3BASE_WAVEFORMAT_H
