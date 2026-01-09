#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;
using test::ExpectVector3Near;

class Vector3Test : public ::testing::Test
{
protected:
	static constexpr float Projection[4][4] = {
		{ 1.07111096f, 0.000000000f, 0.000000000f, 0.000000000f },  //
		{ 0.000000000f, 1.42814803f, 0.000000000f, 0.000000000f },  //
		{ 0.000000000f, 0.000000000f, 1.00108361f, 1.00000000f },   //
		{ 0.000000000f, 0.000000000f, -0.500541806f, 0.000000000f } //
	};

	__Matrix44 mtxProjection {};

	void SetUp() override
	{
		mtxProjection = Projection;
	}
};

TEST_F(Vector3Test, DefaultConstructor_RespectsDefaultInitializer)
{
	SCOPED_TRACE("__Vector3::__Vector3() = default");

	__Vector3 vec = { 1.0f, 2.0f, 3.0f };
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
	EXPECT_FLOAT_EQ(vec.z, 3.0f);
}

TEST_F(Vector3Test, ConstructFromFloats_MatchesReference)
{
	SCOPED_TRACE("__Vector3::__Vector3(float, float, float)");

	__Vector3 vec(1.0f, 2.0f, 3.0f);
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
	EXPECT_FLOAT_EQ(vec.z, 3.0f);
}

TEST_F(Vector3Test, ConstructFromVector3_MatchesReference)
{
	const __Vector3 expectedVec(1.0f, 2.0f, 3.0f);

	SCOPED_TRACE("__Vector3::__Vector3(const __Vector3&)");

	__Vector3 vec(expectedVec);
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Normalize_Self_MatchesReference)
{
	const __Vector3 expectedVec = { 0.218217880f, 0.436435759f, 0.872871518f };

	SCOPED_TRACE("__Vector3::Normalize()");

	__Vector3 vec = { 64.0f, 128.0f, 256.0f };
	vec.Normalize();
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Normalize_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 0.218217880f, 0.436435759f, 0.872871518f };

	SCOPED_TRACE("__Vector3::Normalize(const __Vector3&)");

	__Vector3 inputVec = { 64.0f, 128.0f, 256.0f };
	__Vector3 vec {};
	vec.Normalize(inputVec);
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Magnitude_MatchesReference)
{
	constexpr float ExpectedResult = 293.284851f;

	SCOPED_TRACE("__Vector3::Magnitude()");

	__Vector3 vec   = { 64.0f, 128.0f, 256.0f };
	float magnitude = vec.Magnitude();
	EXPECT_FLOAT_EQ(magnitude, ExpectedResult);
}

TEST_F(Vector3Test, Dot_MatchesReference)
{
	constexpr float ExpectedResult = 52224.0000f;

	const __Vector3 vec1           = { 64.0f, 128.0f, 256.0f };
	const __Vector3 vec2           = { 48.0f, 96.0f, 144.0f };

	SCOPED_TRACE("__Vector3::Dot(const __Vector3&)");

	float dotProduct = vec1.Dot(vec2);
	EXPECT_FLOAT_EQ(dotProduct, ExpectedResult);
}

TEST_F(Vector3Test, Cross_MatchesReference)
{
	const __Vector3 expectedVec = { -6144.00000f, 3072.00000f, 0.000000000f };
	const __Vector3 vec1        = { 64.0f, 128.0f, 256.0f };
	const __Vector3 vec2        = { 48.0f, 96.0f, 144.0f };

	SCOPED_TRACE("__Vector3::Cross(const __Vector3&, const __Vector3&)");

	__Vector3 vec {};
	vec.Cross(vec1, vec2);
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Absolute_MatchesReference)
{
	const __Vector3 expectedVec = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::Absolute() (x, z)");

	__Vector3 vec = { -64.0f, 128.0f, -256.0f };
	vec.Absolute();
	ExpectVector3Near(vec, expectedVec);

	SCOPED_TRACE("__Vector3::Absolute() (y)");

	vec = { 64.0f, -128.0f, 256.0f };
	vec.Absolute();
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Zero_ClearsAllComponents)
{
	SCOPED_TRACE("__Vector3::Zero()");

	__Vector3 vec = { 64.0f, 128.0f, 256.0f };
	vec.Zero();

	EXPECT_FLOAT_EQ(vec.x, 0.0f);
	EXPECT_FLOAT_EQ(vec.y, 0.0f);
	EXPECT_FLOAT_EQ(vec.z, 0.0f);
}

TEST_F(Vector3Test, Set_Floats_MatchesReference)
{
	const __Vector3 expectedVec = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::Set(float, float, float)");

	__Vector3 vec {};
	vec.Set(64.0f, 128.0f, 256.0f);
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Assign_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator=(const __Vector3&)");

	__Vector3 vec;
	vec = expectedVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Multiply_Matrix_MatchesReferenceWithinTolerance)
{
	const __Vector3 expectedVec = { 68.5511017f, 182.802948f, 255.776855f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator*(const __Matrix44&)");

	__Vector3 vec = lhsVec * mtxProjection;
	ExpectVector3Near(vec, expectedVec, EpsilonWithTolerance);
}

TEST_F(Vector3Test, MultiplyAssign_Delta_MatchesReference)
{
	const __Vector3 expectedVec = { 8192.0f, 16384.0f, 32768.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator*=(float)");

	__Vector3 vec(lhsVec);
	vec *= 128.0f;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, MultiplyAssign_Matrix_MatchesReferenceWithinTolerance)
{
	const __Vector3 expectedVec = { 68.5511017f, 182.802948f, 255.776855f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator*=(const __Matrix44&)");

	__Vector3 vec  = lhsVec;
	vec           *= mtxProjection;
	ExpectVector3Near(vec, expectedVec, EpsilonWithTolerance);
}

TEST_F(Vector3Test, Add_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 80.0f, 160.0f, 320.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator+(const __Vector3&)");

	__Vector3 vec = lhsVec + rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Subtract_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 48.0f, 96.0f, 192.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator-(const __Vector3&)");

	__Vector3 vec = lhsVec - rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Multiply_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 1024.0f, 4096.0f, 16384.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator*(const __Vector3&)");

	__Vector3 vec = lhsVec * rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Divide_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 4.0f, 4.0f, 4.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator/(const __Vector3&)");

	__Vector3 vec = lhsVec / rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, AddAssign_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 80.0f, 160.0f, 320.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator+=(const __Vector3&)");

	__Vector3 vec(lhsVec);
	vec += rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, SubtractAssign_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 48.0f, 96.0f, 192.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator-=(const __Vector3&)");

	__Vector3 vec(lhsVec);
	vec -= rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, MultiplyAssign_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 1024.0f, 4096.0f, 16384.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator*=(const __Vector3&)");

	__Vector3 vec(lhsVec);
	vec *= rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, DivideAssign_Vector3_MatchesReference)
{
	const __Vector3 expectedVec = { 4.0f, 4.0f, 4.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };
	const __Vector3 rhsVec      = { 16.0f, 32.0f, 64.0f };

	SCOPED_TRACE("__Vector3::operator/=(const __Vector3&)");

	__Vector3 vec(lhsVec);
	vec /= rhsVec;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Add_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector3 expectedVec = { 192.0f, 256.0f, 384.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator+(float)");

	__Vector3 vec = lhsVec + Delta;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Subtract_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector3 expectedVec = { -64.0f, 0.0f, 128.0f };
	const __Vector3 lhsVec      = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector3::operator-(float)");

	__Vector3 vec = lhsVec - Delta;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Multiply_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector3 expectedVec = { 8192.0f, 32768.0f, 49152.0f };
	const __Vector3 lhsVec      = { 64.0f, 256.0f, 384.0f };

	SCOPED_TRACE("__Vector3::operator*(float)");

	__Vector3 vec = lhsVec * Delta;
	ExpectVector3Near(vec, expectedVec);
}

TEST_F(Vector3Test, Divide_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector3 expectedVec = { 64.0f, 256.0f, 384.0f };
	const __Vector3 lhsVec      = { 8192.0f, 32768.0f, 49152.0f };

	SCOPED_TRACE("__Vector3::operator/(float)");

	__Vector3 vec = lhsVec / Delta;
	ExpectVector3Near(vec, expectedVec);
}
