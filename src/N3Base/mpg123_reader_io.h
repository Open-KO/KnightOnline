#pragma once

#include <mpg123.h>

/// Reads data from a memory-mapped file for mpg123 decoding.
///
/// This function serves as the read callback for mpg123 when using
/// memory-mapped or file-backed audio data via \ref FileReaderHandle.
///
/// \param userData Pointer to a \ref FileReaderHandle containing the file
///                 and current offset.
/// \param dst Destination buffer to fill with audio data.
/// \param bytes Maximum number of bytes to read.
/// \returns The number of bytes actually read, or 0 if EOF is reached.
mpg123_ssize_t mpg123_filereader_read(void* userData, void* dst, size_t bytes);

/// Seeks to a new position in a memory-mapped file for mpg123 decoding.
///
/// This function serves as the seek callback for mpg123. It adjusts
/// the offset stored in the \ref FileReaderHandle according to \p whence
/// and \p offset.
///
/// \param userData Pointer to a \ref FileReaderHandle containing the file
///                 and current offset.
/// \param offset Byte offset to seek to, relative to \p whence.
/// \param whence SEEK_SET, SEEK_CUR, or SEEK_END.
/// \returns The new absolute position in the file, or -1 on error.
off_t mpg123_filereader_seek(void* userData, off_t offset, int whence);

/// Cleanup callback for mpg123 when using a memory-mapped file.
///
/// This function is provided to satisfy mpg123's callback interface. No
/// cleanup is performed here because the memory and file lifetime is
/// managed externally by \ref StreamedAudioAsset and \ref FileReaderHandle.
///
/// \param userData Pointer to the user data (ignored).
void mpg123_filereader_cleanup(void* userData);
