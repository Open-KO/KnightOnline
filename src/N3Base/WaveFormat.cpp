#include "StdAfxBase.h"
#include "WaveFormat.h"
#include "al_wrapper.h"

#include <FileIO/FileReader.h>

#include <cstring> // std::memcpy()

// We follow Frictional Games' implementation for scanning through RIFF chunks
// https://github.com/FrictionalGames/OALWrapper/blob/973659d5700720a87063789a591b811ce5af7dbe/sources/OAL_WAVSample.cpp

template <typename T>
static const uint8_t* find_chunk(
	const uint8_t* start, const uint8_t* end, const char* chunkID, const T** outStruct)
{
	constexpr size_t RequiredStructSize = std::max(sizeof(T), sizeof(RIFF_SubChunk));
	const uint8_t* ptr                  = start;
	while (ptr < (end - RequiredStructSize))
	{
		const RIFF_SubChunk* chunk = reinterpret_cast<const RIFF_SubChunk*>(ptr);

		if (chunk->SubChunkID[0] == chunkID[0] && chunk->SubChunkID[1] == chunkID[1]
			&& chunk->SubChunkID[2] == chunkID[2] && chunk->SubChunkID[3] == chunkID[3])
		{
			*outStruct = reinterpret_cast<const T*>(ptr);
			return ptr;
		}

		ptr += sizeof(RIFF_SubChunk);
		ptr += chunk->SubChunkSize;
	}

	return nullptr;
}

bool ParseWAV(FileReader& file, ALenum* format, ALsizei* sampleRate, size_t* pcmChunkSize,
	const uint8_t** pcmDataBuffer, ALsizei* pcmDataSize)
{
	const size_t fileSize = static_cast<size_t>(file.Size());
	if (fileSize < sizeof(RIFF_Header))
		return false;

	const uint8_t* start          = static_cast<const uint8_t*>(file.Memory());
	const uint8_t* end            = start + fileSize;
	const uint8_t* ptr            = start;

	const RIFF_Header* riffHeader = reinterpret_cast<const RIFF_Header*>(ptr);
	const WAVE_Format* waveFormat = nullptr;
	const WAVE_Data* waveData     = nullptr;

	if (riffHeader->FileTypeID[0] != 'R' || riffHeader->FileTypeID[1] != 'I'
		|| riffHeader->FileTypeID[2] != 'F' || riffHeader->FileTypeID[3] != 'F'
		|| riffHeader->FileFormatID[0] != 'W' || riffHeader->FileFormatID[1] != 'A'
		|| riffHeader->FileFormatID[2] != 'V' || riffHeader->FileFormatID[3] != 'E')
		return false;

	ptr += sizeof(RIFF_Header);

	ptr  = find_chunk<WAVE_Format>(ptr, end, "fmt ", &waveFormat);
	if (ptr == nullptr)
		return false;

	// We expect PCM
	if (waveFormat->AudioFormat != 1)
		return false;

	ptr += sizeof(RIFF_SubChunk);
	ptr += waveFormat->Size;

	ptr  = find_chunk<WAVE_Data>(ptr, end, "data", &waveData);
	if (ptr == nullptr)
		return false;

	ptr                           += sizeof(RIFF_SubChunk);

	const uint8_t* pcmDataBuffer_  = ptr;
	const uint32_t pcmDataSize_    = waveData->Size;

	if (pcmDataSize_ > static_cast<uintptr_t>(end - pcmDataBuffer_))
		return false;

	ALenum format_ = AL_FORMAT_STEREO16;

	if (waveFormat->NumChannels == 2)
	{
		if (waveFormat->BitsPerSample == 8)
			format_ = AL_FORMAT_STEREO8;
		else if (waveFormat->BitsPerSample == 16)
			format_ = AL_FORMAT_STEREO16;
		else
			return false;
	}
	else if (waveFormat->NumChannels == 1)
	{
		if (waveFormat->BitsPerSample == 8)
			format_ = AL_FORMAT_MONO8;
		else if (waveFormat->BitsPerSample == 16)
			format_ = AL_FORMAT_MONO16;
		else
			return false;
	}
	else
	{
		return false;
	}

	constexpr double DurationSec = 0.418; // roughly the same duration as the MP3 chunks

	ALsizei bytesPerFrame        = static_cast<ALsizei>(
        waveFormat->NumChannels * (waveFormat->BitsPerSample / 8));
	ALsizei chunkFrames   = static_cast<ALsizei>(waveFormat->SampleRate * DurationSec);
	ALsizei pcmChunkSize_ = chunkFrames * bytesPerFrame;

	if (format != nullptr)
		*format = format_;

	if (sampleRate != nullptr)
		*sampleRate = static_cast<ALsizei>(waveFormat->SampleRate);

	if (pcmDataBuffer != nullptr)
		*pcmDataBuffer = pcmDataBuffer_;

	if (pcmDataSize != nullptr)
		*pcmDataSize = pcmDataSize_;

	if (pcmChunkSize != nullptr)
		*pcmChunkSize = pcmChunkSize_;

	return true;
}
