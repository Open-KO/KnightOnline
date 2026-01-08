#include <gtest/gtest.h>
#include <FileIO/FileWriter.h>
#include <atomic>
#include <cstdio>
#include <ctime>

class FileWriterTest : public ::testing::Test
{
protected:
	static constexpr int TEST_FILE_SIZE = 16'384;

	std::filesystem::path _testFilePath;
	FileWriter _file;

	void SetUp() override
	{
		static std::atomic<uint32_t> s_testCounter = 0;
		static const time_t s_time                 = time(nullptr);

		std::string filename = "FileWriterTest_" + std::to_string(s_time) + "_"
							   + std::to_string(s_testCounter++) + ".tmp";

		_testFilePath = std::filesystem::temp_directory_path() / filename;

		ASSERT_TRUE(_file.Create(_testFilePath));
	}

	void TearDown() override
	{
		(void) _file.Close();

		std::error_code ec;
		std::filesystem::remove(_testFilePath, ec);
	}
};

TEST_F(FileWriterTest, IsOpen_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	ASSERT_FALSE(_file.IsOpen());
}

TEST_F(FileWriterTest, Size_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	ASSERT_EQ(_file.Size(), 0);
}

TEST_F(FileWriterTest, Offset_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	ASSERT_EQ(_file.Offset(), 0);
}

TEST_F(FileWriterTest, Seek_Set_SucceedsOnlyWithValidOffsets)
{
	// SEEK_SET (absolute position within the file)
	EXPECT_EQ(_file.Offset(), 0);

	// Start of the file
	EXPECT_TRUE(_file.Seek(0, SEEK_SET));
	EXPECT_EQ(_file.Offset(), 0);

	// Before the start of the file
	EXPECT_FALSE(_file.Seek(-1, SEEK_SET));
	EXPECT_EQ(_file.Offset(), 0);

	// Expand the file
	EXPECT_TRUE(_file.Seek(TEST_FILE_SIZE, SEEK_SET));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);
}

TEST_F(FileWriterTest, Seek_Cur_SucceedsOnlyWithValidOffsets)
{
	// SEEK_CUR (relative position from current offset)
	EXPECT_EQ(_file.Offset(), 0);

	// Start of the file
	EXPECT_TRUE(_file.Seek(0, SEEK_CUR));
	EXPECT_EQ(_file.Offset(), 0);

	// Before the start of the file
	EXPECT_FALSE(_file.Seek(-1, SEEK_CUR));
	EXPECT_EQ(_file.Offset(), 0);

	// 10 bytes ahead of where we are (from 0 -> 10)
	EXPECT_TRUE(_file.Seek(10, SEEK_CUR));
	EXPECT_EQ(_file.Offset(), 10);

	// Expand the file
	EXPECT_TRUE(_file.Seek(TEST_FILE_SIZE, SEEK_CUR));
	EXPECT_EQ(_file.Offset(), 10 + TEST_FILE_SIZE);
}

TEST_F(FileWriterTest, Seek_End_SucceedsOnlyWithValidOffsets)
{
	// SEEK_END (relative position from end offset)
	EXPECT_EQ(_file.Offset(), 0);

	// Expand the file
	EXPECT_TRUE(_file.Seek(TEST_FILE_SIZE, SEEK_SET));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// Reset us back to the start of the file
	// That is, from the end, go all the way back to the start:
	EXPECT_TRUE(_file.Seek(-TEST_FILE_SIZE, SEEK_END));
	EXPECT_EQ(_file.Offset(), 0);

	// End of the file
	EXPECT_TRUE(_file.Seek(0, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// After the end of the file (will expand)
	EXPECT_TRUE(_file.Seek(1, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE + 1);
}

TEST_F(FileWriterTest, Read_FailsOnWriterCall)
{
	uint32_t test = 0;
	ASSERT_FALSE(_file.Read(&test, sizeof(test)));
}

TEST_F(FileWriterTest, Write_FailsWhenBufferIsNullptr)
{
	size_t bytesWritten = 100;
	EXPECT_FALSE(_file.Write(nullptr, 0, &bytesWritten));

	// bytesWritten should be reset
	EXPECT_EQ(bytesWritten, 0);
}

TEST_F(FileWriterTest, Write_SucceedsOnWriteOfZero)
{
	uint8_t test;
	EXPECT_TRUE(_file.Write(&test, 0));
}

TEST_F(FileWriterTest, Write_IsValidFromStart)
{
	uint8_t output[4]   = { 0, 1, 2, 3 };
	uint8_t input[4]    = {};
	size_t bytesWritten = 0;

	EXPECT_EQ(_file.Size(), 0);
	EXPECT_EQ(_file.SizeOnDisk(), 0);

	// Write considered successful
	ASSERT_TRUE(_file.Write(output, 4, &bytesWritten));

	// Correct number of bytes written
	EXPECT_EQ(bytesWritten, 4);

	// Verify size was updated
	EXPECT_EQ(_file.Size(), 4);
	EXPECT_EQ(_file.SizeOnDisk(), 4);

	// Offset moved
	EXPECT_EQ(_file.Offset(), 4);

	// Verify the data we wrote to file is still valid.
	EXPECT_TRUE(_file.Close());

	// File should exist.
	ASSERT_TRUE(std::filesystem::exists(_testFilePath));

	FILE* fp = nullptr;
#ifdef _MSC_VER
	fopen_s(&fp, _testFilePath.string().c_str(), "rb");
#else
	fp = fopen(_testFilePath.string().c_str(), "rb");
#endif

	// Should be accessible...
	EXPECT_TRUE(fp != nullptr);

	if (fp != nullptr)
	{
		long fileSize;

		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Wrote 4 bytes
		EXPECT_EQ(fileSize, 4);

		fread(input, 4, 1, fp);
		fclose(fp);

		// Bytes in the file match what we wrote
		EXPECT_EQ(input[0], output[0]);
		EXPECT_EQ(input[1], output[1]);
		EXPECT_EQ(input[2], output[2]);
		EXPECT_EQ(input[3], output[3]);
	}
}

TEST_F(FileWriterTest, Write_SeekingExpandsFile)
{
	// Seek 10 bytes into the file
	ASSERT_TRUE(_file.Seek(10, SEEK_SET));

	// File should be expanded to 10 bytes
	EXPECT_EQ(_file.Size(), 10);

	// Size on disk isn't expanded yet; not until it's flushed
	EXPECT_EQ(_file.SizeOnDisk(), 0);

	// Manually invoke flush to write any padding to file now
	_file.Flush();

	// Size on disk is updated as the file's flushed
	EXPECT_EQ(_file.SizeOnDisk(), 10);

	EXPECT_TRUE(_file.Close());

	std::error_code ec;
	auto fileSize = std::filesystem::file_size(_testFilePath, ec);

	// Ideally shouldn't have errored fetching the filesize
	ASSERT_FALSE(ec);

	// Verify the actual filesize on disk is correct
	EXPECT_EQ(fileSize, 10);
}

TEST_F(FileWriterTest, Write_CloseInvokesFlush)
{
	// Seek 10 bytes into the file
	ASSERT_TRUE(_file.Seek(10, SEEK_SET));

	// File should be expanded to 10 bytes
	EXPECT_EQ(_file.Size(), 10);

	// Size on disk isn't expanded yet; not until it's flushed
	EXPECT_EQ(_file.SizeOnDisk(), 0);

	// No explicit Flush(), we're just closing the file.
	EXPECT_TRUE(_file.Close());

	std::error_code ec;
	auto fileSize = std::filesystem::file_size(_testFilePath, ec);

	// Ideally shouldn't have errored fetching the filesize
	ASSERT_FALSE(ec);

	// Verify the actual filesize on disk is correct
	EXPECT_EQ(fileSize, 10);
}

TEST_F(FileWriterTest, Create_TruncatesFile)
{
	uint32_t testData = 0;

	EXPECT_EQ(_file.Size(), 0);
	EXPECT_EQ(_file.SizeOnDisk(), 0);

	// Write some data to the file we've already created
	ASSERT_TRUE(_file.Write(&testData, sizeof(uint32_t)));

	EXPECT_EQ(_file.Size(), 4);
	EXPECT_EQ(_file.SizeOnDisk(), 4);

	// Close so that we can check it
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.IsOpen());

	// Verify that we do actually have bytes written in the file
	std::error_code ec;
	auto fileSize = std::filesystem::file_size(_testFilePath, ec);
	EXPECT_EQ(fileSize, 4);

	// Reopen the file
	ASSERT_TRUE(_file.Create(_testFilePath));
	ASSERT_TRUE(_file.IsOpen());

	// Should be empty.
	EXPECT_EQ(_file.Size(), 0);

	// Close it again so we can check the new filesize
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.IsOpen());

	// File should now be truncated
	fileSize = std::filesystem::file_size(_testFilePath, ec);
	EXPECT_EQ(fileSize, 0);
}

TEST_F(FileWriterTest, OpenExisting_AppendsAreOptIn)
{
	uint32_t testData = 1;

	EXPECT_EQ(_file.Size(), 0);
	EXPECT_EQ(_file.SizeOnDisk(), 0);

	// Write some data to the file we've already created
	ASSERT_TRUE(_file.Write(&testData, sizeof(uint32_t)));

	EXPECT_EQ(_file.Size(), 4);
	EXPECT_EQ(_file.SizeOnDisk(), 4);

	// Close so that we can check it
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.IsOpen());

	// Verify that we do actually have bytes written in the file
	std::error_code ec;
	auto fileSize = std::filesystem::file_size(_testFilePath, ec);
	EXPECT_EQ(fileSize, 4);

	// Reopen the file, but this time as existing.
	ASSERT_TRUE(_file.OpenExisting(_testFilePath));
	ASSERT_TRUE(_file.IsOpen());

	// Should have data in it.
	EXPECT_EQ(_file.Size(), 4);

	// As we don't explicitly have a mode for appending,
	// as with WinAPI & the C file I/O API, the offset should still be at the start.
	// The onus is on the caller to seek to the appropriate place.
	EXPECT_EQ(_file.Offset(), 0);

	// Close it again so we can check the new filesize
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.IsOpen());

	// File should still be 4 bytes.
	fileSize = std::filesystem::file_size(_testFilePath, ec);
	EXPECT_EQ(fileSize, 4);
}

TEST_F(FileWriterTest, Close_SucceedsWhenOpen)
{
	EXPECT_TRUE(_file.Close());
}

TEST_F(FileWriterTest, Close_FailsWhenAlreadyClosed)
{
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.Close());
}
