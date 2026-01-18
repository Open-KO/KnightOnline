#include "FileWriter.h"

#include <cassert>
#include <cstdio> // SEEK_SET, SEEK_CUR, SEEK_END

namespace llfio = LLFIO_V2_NAMESPACE;

FileWriter::FileWriter()
{
}

bool FileWriter::OpenExisting(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	// Open the given file for writing.
	// If it doesn't exist, create it.
	// If it already exists, simply load it.
	// NOTE: As we have no 'append' flag, we will leave the offset set to 0.
	// If the user wishes to append, they must seek to the end of the file themselves.
	// This matches WinAPI & general C file I/O API behaviour (where the "a" mode isn't used).
	auto handleResult = llfio::file({}, path.native(), llfio::handle::mode::write,
		llfio::handle::creation::open_existing, llfio::handle::caching::all,
		llfio::handle::flag::none);
	if (!handleResult)
		return false;

	_fileHandle = std::move(std::move(handleResult).value());
	_path       = path;
	_open       = true;
	_size       = 0;

	// We don't initialize this for performance; we purposefully only request the size component.
	// NOLINTNEXTLINE(*.cplusplus.UninitializedObject)
	llfio::stat_t stat;

	auto statResult = stat.fill(_fileHandle, llfio::stat_t::want::size);
	if (statResult)
		_size = static_cast<uint64_t>(stat.st_size);

	_sizeOnDisk = _size;
	return true;
}

bool FileWriter::Create(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	// Open the given file for writing.
	// If it doesn't exist, create it.
	// If it already exists, truncate it.
	auto handleResult = llfio::file({}, path.native(), llfio::handle::mode::write,
		llfio::handle::creation::always_new, llfio::handle::caching::all,
		llfio::handle::flag::none);
	if (!handleResult)
		return false;

	_fileHandle = std::move(std::move(handleResult).value());
	_path       = path;
	_open       = true;
	_size       = 0;
	_sizeOnDisk = 0;
	return true;
}

bool FileWriter::Read(void* /*buffer*/, size_t /*bytesToRead*/, size_t* /*bytesRead = nullptr*/)
{
	// Read operations are not supported in a FileWriter.
	return false;
}

bool FileWriter::Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten /*= nullptr*/)
{
	// Ensure bytesWritten is always initialised, regardless of whether this succeeds or fails.
	if (bytesWritten != nullptr)
		*bytesWritten = 0;

	// Fail on invalid buffers.
	if (buffer == nullptr)
		return false;

	// Nothing to write, so just skip ahead and consider this successful.
	if (bytesToWrite == 0)
		return true;

	assert(_fileHandle.is_valid());

	// Write the given bytes out to file.
	auto writeResult = _fileHandle.write(
		_offset, { { static_cast<const std::byte*>(buffer), bytesToWrite } });
	if (!writeResult)
		return false;

	// Advance the current file's offset by the number of bytes actually written.
	size_t effectiveBytesWritten  = writeResult.value();
	_offset                      += effectiveBytesWritten;

	// Update the caller with the number of bytes actually written.
	if (bytesWritten != nullptr)
		*bytesWritten = effectiveBytesWritten;

	// Expand the size of the file if we've written past the end of it.
	if (_offset > _size)
	{
		_size       = _offset;
		_sizeOnDisk = _size;
	}

	// Succeed if we wrote all of the expected bytes.
	return effectiveBytesWritten == bytesToWrite;
}

bool FileWriter::Seek(int64_t offset, int origin)
{
	// File not opened, we cannot seek.
	if (!IsOpen())
		return false;

	int64_t newOffset = offset;

	switch (origin)
	{
		// Explicitly set to the given offset
		case SEEK_SET:
			break;

		// Set is relative to the current offset
		case SEEK_CUR:
			newOffset += static_cast<int64_t>(_offset);
			break;

		// Set is relative to the end offset (i.e. the size)
		case SEEK_END:
			newOffset += static_cast<int64_t>(_size);
			break;

		default:
			return false;
	}

	if (newOffset < 0)
		return false;

	_offset = static_cast<uint64_t>(newOffset);

	// Expand the size of the file if we're seeking past it.
	// NOTE: As this doesn't make a physical change to the file,
	// we don't yet update _sizeOnDisk.
	// This will be deferred until either a manually triggered Flush(),
	// or the file is closed.
	// At that time, the file on disk will be expanded to match our expected
	// size.
	if (_offset > _size)
		_size = _offset;

	return true;
}

void FileWriter::Flush()
{
	if (!_fileHandle.is_valid())
		return;

	// Our size in memory is larger than the physical size on disk.
	// This can happen when we seek beyond the file; this doesn't
	// expand the file right way, it waits until a flush (now).
	// Write a byte to the last expected offset (_size - 1) and
	// the operating system will fill in the blanks.
	if (_size > _sizeOnDisk)
	{
		std::byte dummy {};

		auto writeResult = _fileHandle.write(_size - 1, { { &dummy, 1 } });
		if (writeResult)
			_sizeOnDisk = _size;
	}
}

bool FileWriter::Close()
{
	if (!_fileHandle.is_valid())
		return false;

	// Ensure we flush any changes before closing.
	Flush();

	(void) _fileHandle.close();

	_offset = 0;
	_open   = false;

	return true;
}

FileWriter::~FileWriter()
{
}
