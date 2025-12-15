#include <gtest/gtest.h>
#include <N3Base/My_3DStruct.h>

namespace
{
	constexpr float Epsilon					= std::numeric_limits<float>::epsilon();
}

class Vector2Test : public ::testing::Test
{
protected:
	void ExpectVector2Near(const __Vector2& a, const __Vector2& b, float epsilon = Epsilon)
	{
		EXPECT_NEAR(a.x, b.x, epsilon);
		EXPECT_NEAR(a.y, b.y, epsilon);
	}
};

TEST_F(Vector2Test, DefaultConstructor_RespectsDefaultInitializer)
{
	SCOPED_TRACE("__Vector2::__Vector2() = default");

	__Vector2 vec = { 1.0f, 2.0f };
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
}

TEST_F(Vector2Test, ConstructFromFloats_MatchesReference)
{
	SCOPED_TRACE("__Vector2::__Vector2(float, float)");

	__Vector2 vec(1.0f, 2.0f);
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
}

TEST_F(Vector2Test, ConstructFromVector2_MatchesReference)
{
	const __Vector2 initialisedVector(1.0f, 2.0f);

	SCOPED_TRACE("__Vector2::__Vector2(const __Vector2&)");

	__Vector2 vec(initialisedVector);
	ExpectVector2Near(vec, initialisedVector);
}

TEST_F(Vector2Test, Zero_ClearsAllComponents)
{
	SCOPED_TRACE("__Vector2::Zero()");

	__Vector2 vec = { 64.0f, 128.0f };
	vec.Zero();

	EXPECT_FLOAT_EQ(vec.x, 0.0f);
	EXPECT_FLOAT_EQ(vec.y, 0.0f);
}

TEST_F(Vector2Test, Set_Floats_MatchesReference)
{
	const __Vector2 expectedVector = { 64.0f, 128.0f };

	SCOPED_TRACE("__Vector2::Set(float, float)");

	__Vector2 vec;
	vec.Set(64.0f, 128.0f);
	ExpectVector2Near(vec, expectedVector);
}

TEST_F(Vector2Test, AddAssign_Vector2_MatchesReference)
{
	const __Vector2 expectedVec = { 80.0f, 160.0f };
	const __Vector2 lhsVec = { 64.0f, 128.0f };
	const __Vector2 rhsVec = { 16.0f, 32.0f };

	SCOPED_TRACE("__Vector2::operator+=(const __Vector2&)");

	__Vector2 vec(lhsVec);
	vec += rhsVec;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, SubtractAssign_Vector2_MatchesReference)
{
	const __Vector2 expectedVec = { 48.0f, 96.0f };
	const __Vector2 lhsVec = { 64.0f, 128.0f };
	const __Vector2 rhsVec = { 16.0f, 32.0f };

	SCOPED_TRACE("__Vector2::operator-=(const __Vector2&)");

	__Vector2 vec(lhsVec);
	vec -= rhsVec;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, MultiplyAssign_Delta_MatchesReference)
{
	const __Vector2 expectedVec = { 8192.0f, 16384.0f };
	const __Vector2 lhsVec = { 64.0f, 128.0f };

	SCOPED_TRACE("__Vector2::operator*(float)");

	__Vector2 vec(lhsVec);
	vec *= 128.0f;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, DivideAssign_Float_MatchesReference)
{
	constexpr float Delta = 128.0f;

	const __Vector2 expectedVec = { 64.0f, 256.0f };
	const __Vector2 lhsVec = { 8192.0f, 32768.0f };

	SCOPED_TRACE("__Vector3::operator/=(float)");

	__Vector2 vec(lhsVec);
	vec /= Delta;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, Add_Vector2_MatchesReference)
{
	const __Vector2 expectedVec = { 80.0f, 160.0f };
	const __Vector2 lhsVec = { 64.0f, 128.0f };
	const __Vector2 rhsVec = { 16.0f, 32.0f };

	SCOPED_TRACE("__Vector2::operator+(const __Vector2&)");

	__Vector2 vec = lhsVec + rhsVec;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, Subtract_Vector2_MatchesReference)
{
	const __Vector2 expectedVec = { 48.0f, 96.0f };
	const __Vector2 lhsVec = { 64.0f, 128.0f };
	const __Vector2 rhsVec = { 16.0f, 32.0f };

	SCOPED_TRACE("__Vector2::operator-(const __Vector2&)");

	__Vector2 vec = lhsVec - rhsVec;
	ExpectVector2Near(vec, expectedVec);
}

TEST_F(Vector2Test, Multiply_Float_MatchesReference)
{
	constexpr float Delta = 128.0f;

	const __Vector2 expectedVec = { 8192.0f, 32768.0f };
	const __Vector2 lhsVec = { 64.0f, 256.0f };

	SCOPED_TRACE("__Vector2::operator*(float)");

	__Vector2 vec = lhsVec * Delta;
	ExpectVector2Near(vec, expectedVec);
}
