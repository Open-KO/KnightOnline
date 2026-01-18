#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;
using test::ExpectVector4Near;

class Vector4Test : public ::testing::Test
{
protected:
	static constexpr float Projection[4][4] = {
		{ 1.07111096f, 0.000000000f, 0.000000000f, 0.000000000f },  //
		{ 0.000000000f, 1.42814803f, 0.000000000f, 0.000000000f },  //
		{ 0.000000000f, 0.000000000f, 1.00108361f, 1.00000000f },   //
		{ 0.000000000f, 0.000000000f, -0.500541806f, 0.000000000f } //
	};

	__Matrix44 mtxProjection;

	void SetUp() override
	{
		mtxProjection = Projection;
	}
};

TEST_F(Vector4Test, DefaultConstructor_RespectsDefaultInitializer)
{
	SCOPED_TRACE("__Vector4::__Vector4() = default");

	__Vector4 vec = { 1.0f, 2.0f, 3.0f, 4.0f };
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
	EXPECT_FLOAT_EQ(vec.z, 3.0f);
	EXPECT_FLOAT_EQ(vec.w, 4.0f);
}

TEST_F(Vector4Test, ConstructFromFloats_MatchesReference)
{
	SCOPED_TRACE("__Vector4::__Vector4(float, float, float, float)");

	__Vector4 vec(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_FLOAT_EQ(vec.x, 1.0f);
	EXPECT_FLOAT_EQ(vec.y, 2.0f);
	EXPECT_FLOAT_EQ(vec.z, 3.0f);
	EXPECT_FLOAT_EQ(vec.w, 4.0f);
}

TEST_F(Vector4Test, ConstructFromVector4_MatchesReference)
{
	const __Vector4 initialisedVector(1.0f, 2.0f, 3.0f, 4.0f);

	SCOPED_TRACE("__Vector4::__Vector3(const __Vector4&)");

	__Vector4 vec(initialisedVector);
	ExpectVector4Near(vec, initialisedVector);
}

TEST_F(Vector4Test, Zero_ClearsAllComponents)
{
	SCOPED_TRACE("__Vector4::Zero()");

	__Vector4 vec = { 64.0f, 128.0f, 256.0f, 2048.0f };
	vec.Zero();

	EXPECT_FLOAT_EQ(vec.x, 0.0f);
	EXPECT_FLOAT_EQ(vec.y, 0.0f);
	EXPECT_FLOAT_EQ(vec.z, 0.0f);
	EXPECT_FLOAT_EQ(vec.w, 0.0f);
}

TEST_F(Vector4Test, Set_Floats_MatchesReference)
{
	const __Vector4 expectedVector = { 64.0f, 128.0f, 256.0f, 2048.0f };

	SCOPED_TRACE("__Vector4::Set(float, float, float, float)");

	__Vector4 vec;
	vec.Set(64.0f, 128.0f, 256.0f, 2048.0f);
	ExpectVector4Near(vec, expectedVector);
}

TEST_F(Vector4Test, Transform_MatchesReferenceWithinTolerance)
{
	const __Vector4 expectedVector = { 68.5511017f, 182.802948f, 255.776855f, 256.0f };
	const __Vector3 inputVec       = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Vector4::Transform(const __Vector3&, const __Matrix44&)");

	__Vector4 vec;
	vec.Transform(inputVec, mtxProjection);
	ExpectVector4Near(vec, expectedVector, EpsilonWithTolerance);
}

TEST_F(Vector4Test, AddAssign_Vector4_MatchesReference)
{
	const __Vector4 expectedVec = { 80.0f, 160.0f, 320.0f, 640.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 rhsVec      = { 16.0f, 32.0f, 64.0f, 128.0f };

	SCOPED_TRACE("__Vector4::operator+=(const __Vector4&)");

	__Vector4 vec(lhsVec);
	vec += rhsVec;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, SubtractAssign_Vector4_MatchesReference)
{
	const __Vector4 expectedVec = { 48.0f, 96.0f, 192.0f, 384.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 rhsVec      = { 16.0f, 32.0f, 64.0f, 128.0f };

	SCOPED_TRACE("__Vector4::operator-=(const __Vector4&)");

	__Vector4 vec(lhsVec);
	vec -= rhsVec;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, MultiplyAssign_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector4 expectedVec = { 8192.0f, 16384.0f, 32768.0f, 65536.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Vector4::operator*=(float)");

	__Vector4 vec(lhsVec);
	vec *= Delta;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, DivideAssign_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector4 expectedVec = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 lhsVec      = { 8192.0f, 16384.0f, 32768.0f, 65536.0f };

	SCOPED_TRACE("__Vector4::operator/=(float)");

	__Vector4 vec(lhsVec);
	vec /= Delta;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, Add_Vector4_MatchesReference)
{
	const __Vector4 expectedVec = { 80.0f, 160.0f, 320.0f, 640.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 rhsVec      = { 16.0f, 32.0f, 64.0f, 128.0f };

	SCOPED_TRACE("__Vector4::operator+(const __Vector4&)");

	__Vector4 vec = lhsVec + rhsVec;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, Subtract_Vector4_MatchesReference)
{
	const __Vector4 expectedVec = { 48.0f, 96.0f, 192.0f, 384.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 rhsVec      = { 16.0f, 32.0f, 64.0f, 128.0f };

	SCOPED_TRACE("__Vector4::operator+(const __Vector4&)");

	__Vector4 vec = lhsVec - rhsVec;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, Multiply_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector4 expectedVec = { 8192.0f, 16384.0f, 32768.0f, 65536.0f };
	const __Vector4 lhsVec      = { 64.0f, 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Vector4::operator*(float)");

	__Vector4 vec = lhsVec * Delta;
	ExpectVector4Near(vec, expectedVec);
}

TEST_F(Vector4Test, Divide_Float_MatchesReference)
{
	constexpr float Delta       = 128.0f;

	const __Vector4 expectedVec = { 64.0f, 128.0f, 256.0f, 512.0f };
	const __Vector4 lhsVec      = { 8192.0f, 16384.0f, 32768.0f, 65536.0f };

	SCOPED_TRACE("__Vector4::operator/(float)");

	__Vector4 vec = lhsVec / Delta;
	ExpectVector4Near(vec, expectedVec);
}
