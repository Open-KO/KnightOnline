#include <gtest/gtest.h>
#include <FileIO/FileWriter.h>
#include <atomic>
#include <cstdio>
#include <ctime>

class FileWriterSetupTest : public ::testing::Test
{
protected:
	std::filesystem::path _testFilePath;
	FileWriter _file;

	void SetUp() override
	{
		static std::atomic<uint32_t> s_testCounter = 0;
		static const time_t s_time                 = time(nullptr);

		std::string filename = "FileWriterSetupTest_" + std::to_string(s_time) + "_"
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

TEST_F(FileWriterSetupTest, IsOpen_DefaultIsFalse)
{
	EXPECT_FALSE(_file.IsOpen());
}

TEST_F(FileWriterSetupTest, Size_DefaultIsZero)
{
	EXPECT_EQ(_file.Size(), 0);
}

TEST_F(FileWriterSetupTest, SizeOnDisk_DefaultIsZero)
{
	EXPECT_EQ(_file.SizeOnDisk(), 0);
}

TEST_F(FileWriterSetupTest, Offset_DefaultIsZero)
{
	EXPECT_EQ(_file.Offset(), 0);
}

TEST_F(FileWriterSetupTest, OpenExisting_FailsOnMissingFile)
{
	EXPECT_FALSE(_file.IsOpen());
	EXPECT_FALSE(_file.OpenExisting("MISSING FILE.BIN"));
	EXPECT_FALSE(_file.IsOpen());
}

TEST_F(FileWriterSetupTest, OpenExisting_SucceedsOnExistingFile)
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
