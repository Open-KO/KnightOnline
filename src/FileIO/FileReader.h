#ifndef FILEIO_FILEREADER_H
#define FILEIO_FILEREADER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

/// \brief FileReader implements a read-only file interface.
/// 
/// Provides memory-mapped reading of a file and tracks the current read offset.
/// Writing operations are not supported.
class FileReader : public File
{
public:
	/// \brief Gets the underlying mapped file handle. Intended only for testing.
	/// \returns Reference to the mapped_file_handle used internally.
	LLFIO_V2_NAMESPACE::mapped_file_handle& MappedFileHandle()
	{
		return _mappedFileHandle;
	}

	/// \brief Gets the pointer to the mapped memory for the loaded file.
	/// \returns the pointer to the mapped memory for the loaded file.
	const void* Memory() const
	{
		return _address;
	}

	/// \brief Constructs a new FileReader object.
	FileReader();

	/// \brief Opens an existing file for reading.
	/// \param path Path to the file to open.
	/// \returns true if the file was successfully opened, false otherwise.
	bool OpenExisting(const std::filesystem::path& path) override;

	/// \brief Not supported for FileReader. Always returns false.
	/// \param path Path to the file to create.
	/// \returns false
	bool Create(const std::filesystem::path& path) override;

	/// \brief Reads data from the file from the current offset.
	/// \param buffer Destination buffer for data.
	/// \param bytesToRead Number of bytes to read.
	/// \param bytesRead Optional output for number of bytes actually read.
	/// \returns true if at least one byte was read successfully, false otherwise.
	/// \note Reading past EOF fails and bytesRead is set to 0.
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;

	/// \brief Writing is not supported for FileReader.
	/// \param buffer Data to write.
	/// \param bytesToWrite Number of bytes to write.
	/// \param bytesWritten Optional output for number of bytes written.
	/// \returns false always.
	bool Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten = nullptr) override;

	/// \brief Changes the current read offset.
	/// \param offset Offset value.
	/// \param origin One of SEEK_SET, SEEK_CUR, SEEK_END.
	/// \returns true if the new offset is valid, false otherwise.
	/// \note Seeking past the file start or EOF is invalid and returns false.
	bool Seek(int64_t offset, int origin) override;

	/// \brief No-op for FileReader, as it does not write any buffered data.
	void Flush() override;

	/// \brief Closes the file and resets internal state.
	/// \returns true if the file was open and closed successfully, false if already closed.
	bool Close() override;

	/// \brief Destroys the FileReader object.
	~FileReader() override;

protected:
	LLFIO_V2_NAMESPACE::mapped_file_handle _mappedFileHandle;	///< Memory-mapped file handle for read access.
	void* _address;												///< Pointer to the memory-mapped data.
};

#endif // FILEIO_FILEREADER_H
