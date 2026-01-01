#include "StdAfxBase.h"
#include "AudioHandle.h"
#include "N3Base.h"
#include "N3SndMgr.h"
#include "AudioAsset.h"
#include "al_wrapper.h"
#include "mpg123_reader_io.h"

#include <FileIO/FileReader.h>

AudioHandle::AudioHandle()
{
	HandleType		= AUDIO_HANDLE_UNKNOWN;
	IsManaged		= false;
	StartedPlaying	= false;
	FinishedPlaying	= false;
	SourceId		= INVALID_AUDIO_SOURCE_ID;

	State			= SNDSTATE_INITIAL;

	FadeInTime		= 0.0f;
	FadeOutTime		= 0.0f;
	StartDelayTime	= 0.0f;
	Timer			= 0.0f;
}

AudioHandle::~AudioHandle()
{
}

std::shared_ptr<BufferedAudioHandle> BufferedAudioHandle::Create(std::shared_ptr<AudioAsset> asset)
{
	if (asset == nullptr)
		return nullptr;

	uint32_t sourceId;
	if (!CN3Base::s_SndMgr.PullBufferedSourceIdFromPool(&sourceId))
		return nullptr;

	auto handle = std::make_shared<BufferedAudioHandle>();
	if (handle == nullptr)
	{
		CN3Base::s_SndMgr.RestoreBufferedSourceIdToPool(&sourceId);
		return nullptr;
	}

	handle->SourceId = sourceId;
	handle->Asset = std::move(asset);
	return handle;
}

std::shared_ptr<StreamedAudioHandle> StreamedAudioHandle::Create(std::shared_ptr<AudioAsset> asset)
{
	assert(asset != nullptr);

	if (asset == nullptr)
		return nullptr;

	assert(asset->Type == AUDIO_ASSET_STREAMED);

	if (asset->Type != AUDIO_ASSET_STREAMED)
		return nullptr;

	StreamedAudioAsset* streamedAudioAsset = static_cast<StreamedAudioAsset*>(asset.get());

	uint32_t sourceId;
	if (!CN3Base::s_SndMgr.PullStreamedSourceIdFromPool(&sourceId))
		return nullptr;

	auto handle = std::make_shared<StreamedAudioHandle>();
	if (handle == nullptr)
	{
		CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
		return nullptr;
	}

	if (handle->Mp3Handle != nullptr)
	{
		mpg123_delete(handle->Mp3Handle);
		handle->Mp3Handle = nullptr;
	}

	if (asset->DecoderType == AUDIO_DECODER_MP3)
	{
		handle->Mp3Handle = mpg123_new(nullptr, nullptr);
		if (handle->Mp3Handle == nullptr)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		int err = mpg123_format_none(handle->Mp3Handle);
		assert(err == MPG123_OK);

		if (err != MPG123_OK)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		err = mpg123_format(handle->Mp3Handle, asset->SampleRate, MPG123_STEREO, MPG123_ENC_SIGNED_16);
		assert(err == MPG123_OK);

		if (err != MPG123_OK)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		err = mpg123_replace_reader_handle(handle->Mp3Handle, mpg123_filereader_read, mpg123_filereader_seek, mpg123_filereader_cleanup);
		assert(err == MPG123_OK);

		if (err != MPG123_OK)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		// Position the reader handle at the start of the file.
		handle->FileReaderHandle.File	= streamedAudioAsset->File.get();
		handle->FileReaderHandle.Offset	= 0;
	
		err = mpg123_open_handle(handle->Mp3Handle, &handle->FileReaderHandle);
		assert(err == MPG123_OK);

		if (err != MPG123_OK)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		if (streamedAudioAsset->PcmChunkSize == 0)
			streamedAudioAsset->PcmChunkSize = mpg123_outblock(handle->Mp3Handle);
	}
	else if (asset->DecoderType == AUDIO_DECODER_PCM)
	{
		if (streamedAudioAsset->PcmDataBuffer == nullptr)
		{
			CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
			return nullptr;
		}

		// Position the reader handle at the beginning of the data buffer.
		handle->FileReaderHandle.File	= streamedAudioAsset->File.get();
		handle->FileReaderHandle.Offset	= streamedAudioAsset->PcmDataBuffer
			- static_cast<const uint8_t*>(streamedAudioAsset->File->Memory());
	}
	else
	{
		CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&sourceId);
		return nullptr;
	}

	handle->SourceId		= sourceId;
	handle->Asset			= std::move(asset);

	return handle;
}

BufferedAudioHandle::BufferedAudioHandle()
{
	HandleType = AUDIO_HANDLE_BUFFERED;
}

BufferedAudioHandle::~BufferedAudioHandle()
{
	if (SourceId != INVALID_AUDIO_SOURCE_ID)
	{
		alSourceStop(SourceId);
		AL_CLEAR_ERROR_STATE();

		alSourceRewind(SourceId);
		AL_CLEAR_ERROR_STATE();

		alSourcei(SourceId, AL_BUFFER, 0);
		AL_CLEAR_ERROR_STATE();

		CN3Base::s_SndMgr.RestoreBufferedSourceIdToPool(&SourceId);
	}
}

StreamedAudioHandle::StreamedAudioHandle()
{
	HandleType			= AUDIO_HANDLE_STREAMED;
	Mp3Handle			= nullptr;
	BuffersAllocated	= false;
	FinishedDecoding	= false;

	BufferIds.reserve(MAX_AUDIO_STREAM_BUFFER_COUNT);
}

void StreamedAudioHandle::RewindFrame()
{
	StreamedAudioAsset* asset = static_cast<StreamedAudioAsset*>(Asset.get());

	if (asset->DecoderType == AUDIO_DECODER_MP3)
	{
		if (Mp3Handle != nullptr)
			mpg123_seek_frame(Mp3Handle, 0, SEEK_SET);
		else
			FileReaderHandle.Offset = 0;
	}
	else if (asset->DecoderType == AUDIO_DECODER_PCM)
	{
		FileReaderHandle.Offset = asset->PcmDataBuffer - static_cast<const uint8_t*>(asset->File->Memory());
	}
	else
	{
		assert(!"StreamedAudioHandle::RewindFrame: Unsupported asset decoder type");
	}
}

StreamedAudioHandle::~StreamedAudioHandle()
{
	if (SourceId != INVALID_AUDIO_SOURCE_ID)
	{
		alSourceStop(SourceId);
		AL_CHECK_ERROR();

		alSourceRewind(SourceId);
		AL_CHECK_ERROR();

		alSourcei(SourceId, AL_BUFFER, 0);
		AL_CHECK_ERROR();

		for (uint32_t bufferId : BufferIds)
		{
			alDeleteBuffers(1, &bufferId);
			AL_CHECK_ERROR();
		}

		CN3Base::s_SndMgr.RestoreStreamedSourceIdToPool(&SourceId);
	}

	if (Mp3Handle != nullptr)
	{
		mpg123_delete(Mp3Handle);
		Mp3Handle = nullptr;
	}
}
