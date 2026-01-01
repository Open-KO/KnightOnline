#include <gtest/gtest.h>
#include <FileIO/FileReader.h>
#include <atomic>
#include <cstdio>
#include <time.h>

class FileReaderSetupTest : public ::testing::Test
{
protected:
	std::filesystem::path _testFilePath;
	FileReader _file;

	void SetUp() override
	{
		static std::atomic<uint32_t> s_testCounter = 0;
		static const time_t s_time = time(nullptr);

		std::string filename = "FileReaderSetupTest_"
			+ std::to_string(s_time)
			+ "_"
			+ std::to_string(s_testCounter++) + ".tmp";

		_testFilePath = std::filesystem::temp_directory_path() / filename;
	}

	void TearDown() override
	{
		(void) _file.Close();

		std::error_code ec;
		std::filesystem::remove(_testFilePath, ec);
	}
};

TEST_F(FileReaderSetupTest, IsOpen_DefaultIsFalse)
{
	EXPECT_FALSE(_file.IsOpen());
}

TEST_F(FileReaderSetupTest, Size_DefaultIsZero)
{
	EXPECT_EQ(_file.Size(), 0);
}

TEST_F(FileReaderSetupTest, Offset_DefaultIsZero)
{
	EXPECT_EQ(_file.Offset(), 0);
}

TEST_F(FileReaderSetupTest, OpenExisting_FailsOnMissingFile)
{
	EXPECT_FALSE(_file.IsOpen());
	EXPECT_FALSE(_file.OpenExisting("MISSING FILE.BIN"));
	EXPECT_FALSE(_file.IsOpen());
}

TEST_F(FileReaderSetupTest, OpenExisting_SucceedsOnExistingFile)
{
	FILE* fp = nullptr;
#ifdef _MSC_VER
	fopen_s(&fp, _testFilePath.string().c_str(), "wb");
#else
	fp = fopen(_testFilePath.string().c_str(), "wb");
#endif

	ASSERT_TRUE(fp != nullptr);

	if (fp != nullptr)
		fclose(fp);

	EXPECT_FALSE(_file.IsOpen());
	ASSERT_TRUE(_file.OpenExisting(_testFilePath));
	ASSERT_TRUE(_file.IsOpen());
}

TEST_F(FileReaderSetupTest, Create_FailsOnReaderCall)
{
	EXPECT_FALSE(_file.IsOpen());
	ASSERT_FALSE(_file.Create(_testFilePath));
	ASSERT_FALSE(_file.IsOpen());
}
