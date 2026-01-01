#include <gtest/gtest.h>
#include <FileIO/FileReader.h>
#include <atomic>
#include <cstdio>
#include <time.h>

class FileReaderTest : public ::testing::Test
{
protected:
	static constexpr int TEST_FILE_SIZE = 16'384;

	std::filesystem::path _testFilePath;
	FileReader _file;

	void SetUp() override
	{
		static std::atomic<uint32_t> s_testCounter = 0;
		static const time_t s_time = time(nullptr);

		std::string filename = "FileReaderTest_"
			+ std::to_string(s_time)
			+ "_"
			+ std::to_string(s_testCounter++) + ".tmp";

		_testFilePath = std::filesystem::temp_directory_path() / filename;

		FILE* fp = nullptr;
#ifdef _MSC_VER
		fopen_s(&fp, _testFilePath.string().c_str(), "wb");
#else
		fp = fopen(_testFilePath.string().c_str(), "wb");
#endif
		if (fp != nullptr)
		{
			std::vector<uint8_t> buffer;
			buffer.resize(TEST_FILE_SIZE);

			for (size_t i = 0; i < TEST_FILE_SIZE; i++)
				buffer[i] = i % 256;

			fwrite(&buffer[0], buffer.size(), 1, fp);
			fclose(fp);
		}

		ASSERT_TRUE(_file.OpenExisting(_testFilePath));
	}

	void TearDown() override
	{
		(void) _file.Close();

		std::error_code ec;
		std::filesystem::remove(_testFilePath, ec);
	}
};

TEST_F(FileReaderTest, IsOpen_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.IsOpen());
}

TEST_F(FileReaderTest, Size_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	EXPECT_EQ(_file.Size(), 0);
}

TEST_F(FileReaderTest, Offset_IsResetAfterClose)
{
	EXPECT_TRUE(_file.Close());
	EXPECT_EQ(_file.Offset(), 0);
}

TEST_F(FileReaderTest, OpenExisting_LoadedCapacityMatchesUnderlyingFileSize)
{
	const auto& mappedFileHandle = _file.MappedFileHandle();
	EXPECT_EQ(mappedFileHandle.capacity(), TEST_FILE_SIZE);
	EXPECT_EQ(mappedFileHandle.capacity(), mappedFileHandle.underlying_file_maximum_extent().value());
}

TEST_F(FileReaderTest, Seek_Set_SucceedsOnlyWithValidOffsets)
{
	// SEEK_SET (absolute position within the file)
	EXPECT_EQ(_file.Offset(), 0);

	// Start of the file
	EXPECT_TRUE(_file.Seek(0, SEEK_SET));
	EXPECT_EQ(_file.Offset(), 0);

	// Before the start of the file
	EXPECT_FALSE(_file.Seek(-1, SEEK_SET));
	EXPECT_EQ(_file.Offset(), 0);

	// End of the file
	EXPECT_TRUE(_file.Seek(TEST_FILE_SIZE, SEEK_SET));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// After the end of the file
	EXPECT_FALSE(_file.Seek(TEST_FILE_SIZE + 1, SEEK_SET));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);
}

TEST_F(FileReaderTest, Seek_Cur_SucceedsOnlyWithValidOffsets)
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

	// After the end of the file
	EXPECT_FALSE(_file.Seek(TEST_FILE_SIZE, SEEK_CUR));
	EXPECT_EQ(_file.Offset(), 10);
}

TEST_F(FileReaderTest, Seek_End_SucceedsOnlyWithValidOffsets)
{
	// SEEK_END (relative position from end offset)
	EXPECT_EQ(_file.Offset(), 0);

	// End of the file
	EXPECT_TRUE(_file.Seek(0, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// After the end of the file
	EXPECT_FALSE(_file.Seek(1, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// 10 bytes from the end of the file
	EXPECT_TRUE(_file.Seek(-10, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE - 10);

	// After the start of the file
	EXPECT_FALSE(_file.Seek(TEST_FILE_SIZE + 10, SEEK_END));
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE - 10);
}

TEST_F(FileReaderTest, Write_FailsOnReaderCall)
{
	char test[] = "test";
	EXPECT_FALSE(_file.Write(test, sizeof(test)));
}

TEST_F(FileReaderTest, Read_FailsWhenBufferIsNullptr)
{
	size_t bytesRead = 100;
	EXPECT_FALSE(_file.Read(nullptr, 0, &bytesRead));

	// bytesRead should be reset
	EXPECT_EQ(bytesRead, 0);
}

TEST_F(FileReaderTest, Read_SucceedsOnReadOfZero)
{
	uint8_t test;
	EXPECT_TRUE(_file.Read(&test, 0));
}

TEST_F(FileReaderTest, Read_IsValidReadFromStart)
{
	uint8_t input[4] = {};
	size_t bytesRead = 0;

	// Read considered successful
	ASSERT_TRUE(_file.Read(input, 4, &bytesRead));

	// Correct number of bytes read
	EXPECT_EQ(bytesRead, 4);

	// Offset moved
	EXPECT_EQ(_file.Offset(), 4);

	// Correct data
	EXPECT_EQ(input[0], 0);
	EXPECT_EQ(input[1], 1);
	EXPECT_EQ(input[2], 2);
	EXPECT_EQ(input[3], 3);
}

TEST_F(FileReaderTest, Read_HasFullDataAfterSeek)
{
	uint8_t input[4] = {};
	size_t bytesRead = 0;

	// Seek 10 bytes into the file
	ASSERT_TRUE(_file.Seek(10, SEEK_SET));

	// Read considered successful
	ASSERT_TRUE(_file.Read(input, 4, &bytesRead));

	// Correct number of bytes read
	EXPECT_EQ(bytesRead, 4);

	// Offset moved
	EXPECT_EQ(_file.Offset(), 14);

	// Correct data
	EXPECT_EQ(input[0], 10);
	EXPECT_EQ(input[1], 11);
	EXPECT_EQ(input[2], 12);
	EXPECT_EQ(input[3], 13);
}

TEST_F(FileReaderTest, Read_HasPartialDataAfterSeek)
{
	uint8_t input[4] = {};
	size_t bytesRead = 0;

	// Seek to the end of the file
	ASSERT_TRUE(_file.Seek(TEST_FILE_SIZE - 2, SEEK_SET));

	// Read considered successful
	ASSERT_TRUE(_file.Read(input, 4, &bytesRead));

	// Correct number of bytes read
	EXPECT_EQ(bytesRead, 2);

	// Offset moved
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// Correct data
	EXPECT_EQ(input[0], (TEST_FILE_SIZE - 2) % 256);
	EXPECT_EQ(input[1], (TEST_FILE_SIZE - 1) % 256);
	EXPECT_EQ(input[2], 0);
	EXPECT_EQ(input[3], 0);
}

TEST_F(FileReaderTest, Read_FailsAtEndOfFile)
{
	uint8_t input[4] = {};
	size_t bytesRead = 0;

	// Seek to the end of the file
	ASSERT_TRUE(_file.Seek(TEST_FILE_SIZE, SEEK_SET));

	// Read considered a failure
	ASSERT_FALSE(_file.Read(input, 4, &bytesRead));

	// Correct number of bytes read
	EXPECT_EQ(bytesRead, 0);

	// Offset unchanged
	EXPECT_EQ(_file.Offset(), TEST_FILE_SIZE);

	// Correct data
	EXPECT_EQ(input[0], 0);
	EXPECT_EQ(input[1], 0);
	EXPECT_EQ(input[2], 0);
	EXPECT_EQ(input[3], 0);
}

TEST_F(FileReaderTest, Close_SucceedsWhenOpen)
{
	EXPECT_TRUE(_file.Close());
}

TEST_F(FileReaderTest, Close_FailsWhenAlreadyClosed)
{
	EXPECT_TRUE(_file.Close());
	EXPECT_FALSE(_file.Close());
}
