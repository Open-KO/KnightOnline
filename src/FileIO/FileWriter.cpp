#include "FileWriter.h"

#include <cassert>
#include <cstdio> // SEEK_SET, SEEK_CUR, SEEK_END

FileWriter::FileWriter()
{
}

bool FileWriter::OpenExisting(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	FILE* fileHandle = nullptr;

	// Open the given file for writing.
	// This must already exist so that we can simply load it for writing.
	// NOTE: As we have no 'append' flag, we will leave the offset set to 0.
	// If the user wishes to append, they must seek to the end of the file themselves.
	// This matches WinAPI & general C file I/O API behaviour (where the "a" mode isn't used).
#if defined(_MSC_VER) || defined(__MINGW32__)
	_wfopen_s(&fileHandle, path.native().c_str(), L"rb+");
#else
	fileHandle = fopen(path.native().c_str(), "rb+");
#endif
	if (fileHandle == nullptr)
		return false;

	_fileHandle = fileHandle;
	_path       = path;
	_open       = true;
	_size       = 0;

	if (fseek(fileHandle, 0, SEEK_END) == 0)
	{
		long fileSize = ftell(fileHandle);
		if (fileSize >= 0)
			_size = static_cast<uint64_t>(fileSize);

		// Reset offset to start
		(void) fseek(fileHandle, 0, SEEK_SET);
	}

	_sizeOnDisk = _size;

	return true;
}

bool FileWriter::Create(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	FILE* fileHandle = nullptr;

	// Open the given file for writing.
	// If it doesn't exist, create it.
	// If it already exists, truncate it.
#if defined(_MSC_VER) || defined(__MINGW32__)
	_wfopen_s(&fileHandle, path.native().c_str(), L"wb");
#else
	fileHandle = fopen(path.native().c_str(), "wb");
#endif

	if (fileHandle == nullptr)
		return false;

	_fileHandle = fileHandle;
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

	assert(_fileHandle != nullptr);

	// Write the given bytes out to file.
	if (fwrite(buffer, bytesToWrite, 1, static_cast<FILE*>(_fileHandle)) != 1)
		return false;

	// Advance the current file's offset by the number of bytes written.
	_offset += bytesToWrite;

	// Update the caller with the number of bytes actually written.
	if (bytesWritten != nullptr)
		*bytesWritten = bytesToWrite;

	// Expand the size of the file if we've written past the end of it.
	if (_offset > _size)
	{
		_size       = _offset;
		_sizeOnDisk = _size;
	}

	return true;
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

	fseek(static_cast<FILE*>(_fileHandle), static_cast<long>(offset), origin);

	return true;
}

void FileWriter::Flush()
{
	FlushImpl();
}

void FileWriter::FlushImpl()
{
	if (_fileHandle == nullptr)
		return;

	if (fflush(static_cast<FILE*>(_fileHandle)) != 0)
		return;

	// Our size in memory is larger than the physical size on disk.
	// This can happen when we seek beyond the file; this doesn't
	// expand the file right way, it waits until a flush (now).
	// Write a byte to the last expected offset (_size - 1) and
	// the operating system will fill in the blanks.
	if (_size > _sizeOnDisk)
	{
		uint8_t dummy = 0;
		fseek(static_cast<FILE*>(_fileHandle), static_cast<long>(_size - 1), SEEK_SET);

		if (fwrite(&dummy, sizeof(uint8_t), 1, static_cast<FILE*>(_fileHandle)) == 1)
			_sizeOnDisk = _size;

		fseek(static_cast<FILE*>(_fileHandle), static_cast<long>(_offset), SEEK_SET);
	}
}

bool FileWriter::Close()
{
	if (_fileHandle == nullptr)
		return false;

	// Ensure we flush any changes before closing.
	Flush();

	fclose(static_cast<FILE*>(_fileHandle));
	_fileHandle = nullptr;

	_offset     = 0;
	_open       = false;

	return true;
}

FileWriter::~FileWriter()
{
	if (_fileHandle != nullptr)
	{
		FlushImpl();

		fclose(static_cast<FILE*>(_fileHandle));
		_fileHandle = nullptr;
	}
}
