#include "StdAfxBase.h"
#include "AudioAsset.h"
#include "al_wrapper.h"

#include <FileIO/FileReader.h>
#include <shared/StringUtils.h>

bool ParseWAV(FileReader& file, ALenum* format, ALsizei* sampleRate, size_t* pcmChunkSize,
	const uint8_t** pcmDataBuffer, ALsizei* pcmDataSize);

BufferedAudioAsset::BufferedAudioAsset()
{
	Type		= AUDIO_ASSET_BUFFERED;
	BufferId	= INVALID_AUDIO_BUFFER_ID;
}

BufferedAudioAsset::~BufferedAudioAsset()
{
	if (BufferId != INVALID_AUDIO_BUFFER_ID)
	{
		alDeleteBuffers(1, &BufferId);
		AL_CHECK_ERROR();
	}

	BufferId = INVALID_AUDIO_BUFFER_ID;
}

bool BufferedAudioAsset::LoadFromFile(const std::string& filename)
{
	// Expect ".wav", 4 characters long.
	constexpr size_t ExtensionLength = 4;

	if (filename.length() < ExtensionLength)
		return false;

	const char* extension = filename.data() + filename.length() - ExtensionLength;
	if (strnicmp(extension, ".wav", ExtensionLength) != 0)
		return false;

	FileReader file;
	if (!file.OpenExisting(filename))
		return false;

	ALenum alFormat;
	ALsizei sampleRate, pcmDataSize;
	size_t pcmChunkSize;
	const uint8_t* pcmDataBuffer = nullptr;
	if (!ParseWAV(file, &alFormat, &sampleRate, &pcmChunkSize, &pcmDataBuffer, &pcmDataSize))
		return false;

	alGenBuffers(1, &BufferId);
	if (AL_CHECK_ERROR())
		return false;

	alBufferData(BufferId, alFormat, pcmDataBuffer, pcmDataSize, sampleRate);
	if (AL_CHECK_ERROR())
		return false;

	Filename	= filename;
	AlFormat	= alFormat;
	SampleRate	= static_cast<int32_t>(sampleRate);

	return true;
}

StreamedAudioAsset::StreamedAudioAsset()
{
	Type			= AUDIO_ASSET_STREAMED;
	PcmDataSize		= 0;
	PcmChunkSize	= 0;
	PcmDataBuffer	= nullptr;
}

bool StreamedAudioAsset::LoadFromFile(const std::string& filename)
{
	// Expect ".mp3" or ".wav", both 4 characters long.
	constexpr size_t ExtensionLength = 4;

	if (filename.length() < ExtensionLength)
		return false;

	const char* extension = filename.data() + filename.length() - ExtensionLength;

	if (strnicmp(extension, ".mp3", ExtensionLength) == 0)
		return load_from_file_mp3(filename);

	if (strnicmp(extension, ".wav", ExtensionLength) == 0)
		return load_from_file_wav(filename);

	// Unsupported extension
	return false;
}

bool StreamedAudioAsset::load_from_file_mp3(const std::string& filename)
{
	auto file = std::make_unique<FileReader>();
	if (file == nullptr)
		return false;

	if (!file->OpenExisting(filename))
		return false;

	DecoderType	= AUDIO_DECODER_MP3;
	File		= std::move(file);
	Filename	= filename;
	AlFormat	= AL_FORMAT_STEREO16;
	SampleRate	= 44100;

	return true;
}

bool StreamedAudioAsset::load_from_file_wav(const std::string& filename)
{
	auto file = std::make_unique<FileReader>();
	if (file == nullptr)
		return false;

	if (!file->OpenExisting(filename))
		return false;

	ALenum alFormat = -1;
	ALsizei sampleRate = 0, pcmDataSize = 0;
	size_t pcmChunkSize = 0;
	const uint8_t* pcmDataBuffer = nullptr;

	if (!ParseWAV(*file, &alFormat, &sampleRate, &pcmChunkSize, &pcmDataBuffer, &pcmDataSize))
		return false;

	DecoderType		= AUDIO_DECODER_PCM;
	File			= std::move(file);
	Filename		= filename;
	AlFormat		= alFormat;
	SampleRate		= static_cast<int32_t>(sampleRate);

	PcmChunkSize	= pcmChunkSize;
	PcmDataSize		= static_cast<size_t>(pcmDataSize);
	PcmDataBuffer	= pcmDataBuffer;

	return true;
}

StreamedAudioAsset::~StreamedAudioAsset()
{
}
