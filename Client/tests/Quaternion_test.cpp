#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;
using test::ExpectQuaternionNear;
using test::ExpectVector3Near;

class QuaternionTest : public ::testing::Test
{
protected:
	static constexpr float View[4][4] =
	{
		{ -0.342704505f,   0.397026330f,  0.851424575f, 0.000000000f },
		{  0.000000000f,   0.906307697f, -0.422618717f, 0.000000000f },
		{ -0.939443290f,  -0.144833341f, -0.310595691f, 0.000000000f },
		{   432.886841f,   -84.8252563f,  -150.401611f,  1.00000000f }
	};

	__Matrix44 mtxView;

	void SetUp() override
	{
		mtxView = View;
	}
};

TEST_F(QuaternionTest, DefaultConstructor_RespectsDefaultInitializer)
{
	SCOPED_TRACE("__Quaternion::__Quaternion() = default");

	__Quaternion quat = { 1.0f, 2.0f, 3.0f, 4.0f };
	EXPECT_FLOAT_EQ(quat.x, 1.0f);
	EXPECT_FLOAT_EQ(quat.y, 2.0f);
	EXPECT_FLOAT_EQ(quat.z, 3.0f);
	EXPECT_FLOAT_EQ(quat.w, 4.0f);
}

TEST_F(QuaternionTest, ConstructFromMatrix_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { -0.124080211f, -0.799938679f, 0.177342355f, 0.559689105f };

	SCOPED_TRACE("__Quaternion::__Quaternion(const __Matri44&)");

	__Quaternion quat(mtxView);
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, ConstructFromQuaternion_MatchesReference)
{
	const __Quaternion expectedQuat = { 1.0f, 2.0f, 3.0f, 4.0f };

	SCOPED_TRACE("__Quaternion::__Quaternion(const __Quaternion)");

	__Quaternion quat(expectedQuat);
	EXPECT_FLOAT_EQ(quat.x, expectedQuat.x);
	EXPECT_FLOAT_EQ(quat.y, expectedQuat.y);
	EXPECT_FLOAT_EQ(quat.z, expectedQuat.z);
	EXPECT_FLOAT_EQ(quat.w, expectedQuat.w);
}

TEST_F(QuaternionTest, ConstructFromFloats_MatchesReference)
{
	SCOPED_TRACE("__Quaternion::__Quaternion(float, float, float, float)");

	__Quaternion quat(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_FLOAT_EQ(quat.x, 1.0f);
	EXPECT_FLOAT_EQ(quat.y, 2.0f);
	EXPECT_FLOAT_EQ(quat.z, 3.0f);
	EXPECT_FLOAT_EQ(quat.w, 4.0f);
}

TEST_F(QuaternionTest, Identity_MatchesReference)
{
	const __Quaternion expectedQuat = { 0.0f, 0.0f, 0.0f, 1.0f };

	SCOPED_TRACE("__Quaternion::Identity()");

	__Quaternion quat;
	quat.Identity();

	EXPECT_FLOAT_EQ(quat.x, expectedQuat.x);
	EXPECT_FLOAT_EQ(quat.y, expectedQuat.y);
	EXPECT_FLOAT_EQ(quat.z, expectedQuat.z);
	EXPECT_FLOAT_EQ(quat.w, expectedQuat.w);
}

TEST_F(QuaternionTest, Set_MatchesReference)
{
	SCOPED_TRACE("__Quaternion::Set(float, float, float, float)");

	__Quaternion quat;
	quat.Set(1.0f, 2.0f, 3.0f, 4.0f);

	EXPECT_FLOAT_EQ(quat.x, 1.0f);
	EXPECT_FLOAT_EQ(quat.y, 2.0f);
	EXPECT_FLOAT_EQ(quat.z, 3.0f);
	EXPECT_FLOAT_EQ(quat.w, 4.0f);
}

TEST_F(QuaternionTest, RotationAxis_Vector3_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { -0.137427822f, -0.219884515f, -0.247370064f, 0.933580399f };
	const __Vector3 rot = { 0.5f, 0.8f, 0.9f };

	SCOPED_TRACE("__Quaternion::RotationAxis(const __Vector3&, float)");

	__Quaternion quat = { -0.124080211f, -0.799938679f, 0.177342355f, 0.559689105f };
	quat.RotationAxis(rot, DegreesToRadians(-42.0f));

	// This originally used D3DXQuaternionRotationAxis() so slight inaccuracy is expected.
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, RotationAxis_Floats_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { -0.137427822f, -0.219884515f, -0.247370064f, 0.933580399f };
	const __Vector3 rot = { 0.5f, 0.8f, 0.9f };

	SCOPED_TRACE("__Quaternion::RotationAxis(float, float, float, float)");

	__Quaternion quat = { -0.124080211f, -0.799938679f, 0.177342355f, 0.559689105f };
	quat.RotationAxis(rot.x, rot.y, rot.z, DegreesToRadians(-42.0f));

	// This originally used D3DXQuaternionRotationAxis() so slight inaccuracy is expected.
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, Assign_Matrix_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { -0.124080211f, -0.799938679f, 0.177342355f, 0.559689105f };

	SCOPED_TRACE("__Quaternion::operator=(const __Matrix44&)");

	__Quaternion quat = mtxView;

	// This originally used D3DXQuaternionRotationMatrix() so slight inaccuracy is expected.
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, AxisAngle_MatchesReferenceWithinTolerance)
{
	const __Vector3 expectedAxis(-0.124080211f, -0.799938679f, 0.177342355f);
	const float expectedYaw = 1.95357144f;

	SCOPED_TRACE("__Quaternion::AxisAngle(__Vector3&, float&)");

	__Quaternion quat = { -0.124080211f, -0.799938679f, 0.177342355f, 0.559689105f };
	__Vector3 axis;
	float yaw = 0.0f;
	quat.AxisAngle(axis, yaw);

	// This originally used D3DXQuaternionToAxisAngle() so slight inaccuracy is expected.
	EXPECT_NEAR(yaw, expectedYaw, EpsilonWithTolerance);
	ExpectVector3Near(axis, expectedAxis, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, Slerp_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { 0.150000006f, 0.799938679f, -0.177342355f, 0.559689105f };
	const __Quaternion lhsQuat = { 0.15f, 0.799938679f, -0.177342355f, 0.559689105f };
	const __Quaternion rhsQuat = { -0.15f, -0.799938679f, 0.177342355f, -0.559689105f };

	SCOPED_TRACE("__Quaternion::Slerp(const __Quaternion&, const __Quaternion&, float)");

	__Quaternion quat;
	quat.Slerp(lhsQuat, rhsQuat, 0.5f);

	// This originally used D3DXQuaternionSlerp() so slight inaccuracy is expected.
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, RotationYawPitchRoll_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { 0.438867152f, 0.0410707891f, 0.301422805f, 0.845489919f };

	SCOPED_TRACE("__Quaternion::RotationYawPitchRoll(float, float, float)");

	__Quaternion quat;
	quat.RotationYawPitchRoll(0.5f, 0.8f, 0.9f);

	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, Multiply_Quaternion_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { 0.465000004f, 0.295000017f, 0.310000002f, -0.242500007f };
	const __Quaternion lhsQuat = { 0.5f, 0.8f, 0.9f, 0.75f };
	const __Quaternion rhsQuat = { 0.2f, 0.3f, 0.1f, 0.25f };

	SCOPED_TRACE("__Quaternion::operator*(const __Quaternion&)");

	__Quaternion quat = lhsQuat * rhsQuat;
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}

TEST_F(QuaternionTest, MultiplyAssign_Quaternion_MatchesReferenceWithinTolerance)
{
	const __Quaternion expectedQuat = { 0.465000004f, 0.295000017f, 0.310000002f, -0.242500007f };
	const __Quaternion lhsQuat = { 0.5f, 0.8f, 0.9f, 0.75f };
	const __Quaternion rhsQuat = { 0.2f, 0.3f, 0.1f, 0.25f };

	SCOPED_TRACE("__Quaternion::operator*=(const __Quaternion&)");

	__Quaternion quat = lhsQuat;
	quat *= rhsQuat;
	ExpectQuaternionNear(quat, expectedQuat, EpsilonWithTolerance);
}
