#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;
using test::ExpectMatrixNear;

namespace
{
	constexpr float Identity[4][4] =
	{
		{ 1.000000000f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   1.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  1.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  0.000000000f, 1.000000000f }
	};

	constexpr float Projection[4][4] =
	{
		{  1.07111096f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,    1.42814803f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,   1.00108361f,  1.00000000f },
		{ 0.000000000f,   0.000000000f, -0.500541806f, 0.000000000f }
	};

	constexpr float View[4][4] =
	{
		{ -0.342704505f,  0.397026330f,  0.851424575f, 0.000000000f },
		{  0.000000000f,  0.906307697f, -0.422618717f, 0.000000000f },
		{ -0.939443290f, -0.144833341f, -0.310595691f, 0.000000000f },
		{   432.886841f,  -84.8252563f,  -150.401611f,  1.00000000f }
	};

	constexpr float ViewInverse[4][4] =
	{
		{ -0.342704475f,  0.000000000f, -0.939443111f, 0.000000000f },
		{  0.397026211f,  0.906307399f, -0.144833297f, 0.000000000f },
		{  0.851424456f, -0.422618628f, -0.310595632f, 0.000000000f },
		{   310.085724f,   13.3152275f,   347.672943f, 0.999999821f }
	};

	constexpr float ZeroInitialized[4][4] =
	{
		{ 0.000000000f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  0.000000000f, 0.000000000f }
	};
}

class Matrix44Test : public ::testing::Test
{
protected:
	__Matrix44 mtxZeroInitialized;
	__Matrix44 mtxIdentity;
	__Matrix44 mtxView;
	__Matrix44 mtxViewInverse;
	__Matrix44 mtxProjection;

	void SetUp() override
	{
		mtxZeroInitialized = ZeroInitialized;
		mtxIdentity = Identity;
		mtxView = View;
		mtxViewInverse = ViewInverse;
		mtxProjection = Projection;
	}
};

TEST_F(Matrix44Test, DefaultConstructor_InitializesToZero)
{
	SCOPED_TRACE("__Matrix44::__Matrix44() = default");

	__Matrix44 mtx = {};
	ExpectMatrixNear(mtx, mtxZeroInitialized);
}

TEST_F(Matrix44Test, ConstructFromMatrix_CopiesValues)
{
	SCOPED_TRACE("__Matrix44::__Matrix44(const __Matrix44&)");

	__Matrix44 mtx(mtxIdentity);
	ExpectMatrixNear(mtx, mtxIdentity);
}

TEST_F(Matrix44Test, ConstructFromQuaternion_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -25.0000000f,  28.0000000f,   -10.0000000f,  0.00000000f },
		{ -20.0000000f, -19.0000000f,    20.0000000f,  0.00000000f },
		{  22.0000000f,  4.00000000f,   -9.00000000f,  0.00000000f },
		{  0.00000000f,  0.00000000f,    0.00000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Quaternion quat = { 1.0f, 2.0f, 3.0f, 4.0f };

	SCOPED_TRACE("__Matrix44::__Matrix44(const __Quaternion&)");

	__Matrix44 mtx(quat);
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, Zero_ClearsAllElements)
{
	SCOPED_TRACE("__Matrix44::Zero()");

	__Matrix44 mtx(mtxIdentity);
	mtx.Zero();
	ExpectMatrixNear(mtx, mtxZeroInitialized);
}

TEST_F(Matrix44Test, Identity_SetsIdentityMatrix)
{
	SCOPED_TRACE("__Matrix44::Identity()");

	__Matrix44 mtx(mtxZeroInitialized);
	mtx.Identity();
	ExpectMatrixNear(mtx, mtxIdentity);
}

TEST_F(Matrix44Test, Inverse_MatchesReferenceWithinTolerance)
{
	SCOPED_TRACE("__Matrix44::Inverse()");

	__Matrix44 mtx(mtxView.Inverse());

	// NOTE: The original calls D3DXMatrixInverse(), which can cause some slight inaccuracy.
	ExpectMatrixNear(mtx, mtxViewInverse, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, Pos_ReturnsTranslationComponent)
{
	SCOPED_TRACE("__Matrix44::Pos()");

	__Vector3 pos = mtxViewInverse.Pos();
	EXPECT_FLOAT_EQ(pos.x, 310.085724f);
	EXPECT_FLOAT_EQ(pos.y, 13.3152275f);
	EXPECT_FLOAT_EQ(pos.z, 347.672943f);
}

TEST_F(Matrix44Test, PosSet_Vector3_SetsTranslationComponent)
{
	SCOPED_TRACE("__Matrix44::PosSet(const __Vector3&)");

	__Matrix44 mtx(mtxIdentity);
	mtx.PosSet(mtxViewInverse.Pos());

	__Vector3 pos = mtx.Pos();
	EXPECT_FLOAT_EQ(pos.x, 310.085724f);
	EXPECT_FLOAT_EQ(pos.y, 13.3152275f);
	EXPECT_FLOAT_EQ(pos.z, 347.672943f);
}

TEST_F(Matrix44Test, PosSet_Floats_SetsTranslationComponent)
{
	const __Vector3 expectedPos = mtxViewInverse.Pos();

	SCOPED_TRACE("__Matrix44::PosSet(float, float, float)");

	__Matrix44 mtx(mtxIdentity);
	mtx.PosSet(expectedPos.x, expectedPos.y, expectedPos.z);

	__Vector3 pos = mtx.Pos();
	EXPECT_FLOAT_EQ(pos.x, 310.085724f);
	EXPECT_FLOAT_EQ(pos.y, 13.3152275f);
	EXPECT_FLOAT_EQ(pos.z, 347.672943f);
}

TEST_F(Matrix44Test, RotationX_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ 1.00000000f,   0.000000000f,  0.000000000f, 0.000000000f },
		{ 0.000000000f, -0.692895830f,  0.721037686f, 0.000000000f },
		{ 0.000000000f, -0.721037686f, -0.692895830f, 0.000000000f },
		{ 0.000000000f,  0.000000000f,  0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::RotationX(float)");

	__Matrix44 mtx(mtxProjection);
	mtx.RotationX(128.0f);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, RotationY_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -0.692895830f, 0.000000000f, -0.721037686f, 0.000000000f },
		{ 0.000000000f,   1.00000000f,  0.000000000f, 0.000000000f },
		{ 0.721037686f,  0.000000000f, -0.692895830f, 0.000000000f },
		{ 0.000000000f,  0.000000000f,  0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::RotationY(float)");

	__Matrix44 mtx(mtxProjection);
	mtx.RotationY(128.0f);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, RotationZ_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -0.692895830f,  0.721037686f, 0.000000000f, 0.000000000f },
		{ -0.721037686f, -0.692895830f, 0.000000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f,  1.00000000f, 0.000000000f },
		{ 0.000000000f,   0.000000000f, 0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::RotationZ(float)");

	__Matrix44 mtx(mtxProjection);
	mtx.RotationZ(128.0f);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Rotation_Floats_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ 0.0396647602f, -0.00316410139f,   0.999208033f, 0.000000000f },
		{  0.773283303f,    0.633411288f, -0.0286906380f, 0.000000000f },
		{ -0.632818818f,    0.773808837f,  0.0275708530f, 0.000000000f },
		{  0.000000000f,    0.000000000f,   0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 rotation = { 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Matrix44::Rotation(float, float, float)");

	__Matrix44 mtx(mtxProjection);
	mtx.Rotation(rotation.x, rotation.y, rotation.z);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Rotation_Vector3_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ 0.0396647602f, -0.00316410139f,   0.999208033f, 0.000000000f },
		{  0.773283303f,    0.633411288f, -0.0286906380f, 0.000000000f },
		{ -0.632818818f,    0.773808837f,  0.0275708530f, 0.000000000f },
		{  0.000000000f,    0.000000000f,   0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 rotation = { 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Matrix44::Rotation(const __Vector3&)");

	__Matrix44 mtx(mtxProjection);
	mtx.Rotation(rotation);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Scale_Floats_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{  1.00000000f, 0.000000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f,  2.00000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f,  3.00000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f, 0.000000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 scale = { 1.0f, 2.0f, 3.0f };

	SCOPED_TRACE("__Matrix44::Scale(float, float, float)");

	__Matrix44 mtx(mtxProjection);
	mtx.Scale(scale.x, scale.y, scale.z);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Scale_Vector3_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{  1.00000000f, 0.000000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f,  2.00000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f,  3.00000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f, 0.000000000f,  1.00000000f }
	};

	SCOPED_TRACE("__Matrix44::Scale(const __Vector3&)");

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 vecScale = { 1.0f, 2.0f, 3.0f };

	__Matrix44 mtx(mtxProjection);
	mtx.Scale(vecScale);
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Direction_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{  0.970142603f, 0.000000000f, -0.242535651f,  0.000000000f },
		{ -0.105851233f, 0.899735451f, -0.423404932f,  0.000000000f },
		{  0.218217924f, 0.436435848f,  0.872871697f, -0.000000000f },
		{  0.000000000f, -0.000000000f, 0.000000000f,  1.00000012f  }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 dir = { 64.0f, 128.0f, 256.0f };

	SCOPED_TRACE("__Matrix44::Direction(const __Vector3&)");

	__Matrix44 mtx(mtxProjection);
	mtx.Direction(dir);
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, LookAtLH_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{  0.00000000f,  0.00000000f,  0.218217880f,  0.00000000f },
		{  0.00000000f,  0.00000000f,  0.436435759f,  0.00000000f },
		{  0.00000000f,  0.00000000f,  0.872871518f,  0.00000000f },
		{ -0.00000000f, -0.00000000f,  -293.284821f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 vecEye = {  64.0f, 128.0f,  256.0f };
	const __Vector3 vecAt  = { 128.0f, 256.0f,  512.0f };
	const __Vector3 vecUp  = { 256.0f, 512.0f, 1024.0f };

	SCOPED_TRACE("__Matrix44::LookAtLH(const __Vector3&, const __Vector3&, const __Vector3&)");

	__Matrix44 mtx(mtxProjection);
	mtx.LookAtLH(vecEye, vecAt, vecUp);
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, OrthoLH_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ 0.00195312500f,    0.00000000f,      0.00000000f,  0.00000000f },
		{ 0.00000000f,    0.00260416674f,      0.00000000f,  0.00000000f },
		{ 0.00000000f,       0.00000000f,   0.00223214296f,  0.00000000f },
		{ 0.00000000f,       0.00000000f,    -0.142857149f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::OrthoLH(float, float, float, float)");

	__Matrix44 mtx(mtxProjection);
	mtx.OrthoLH(1024.0f, 768.0f, 64.0f, 512.0f);
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, PerspectiveFovLH_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -0.0163227450f,  0.00000000f,   0.00000000f,  0.00000000f },
		{  0.00000000f,   -12.5358677f,   0.00000000f,  0.00000000f },
		{  0.00000000f,    0.00000000f,   1.14285719f,  1.00000000f },
		{  0.00000000f,    0.00000000f,  -73.1428604f,  0.00000000f } 
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::PerspectiveFovLH(float, float, float, float)");

	__Matrix44 mtx(mtxProjection);
	mtx.PerspectiveFovLH(1024.0f, 768.0f, 64.0f, 512.0f);
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, Multiply_Matrix_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -0.367074549f,  0.567012370f,  0.852347195f,  0.851424575f },
		{  0.000000000f,   1.29434156f, -0.423076659f, -0.422618717f },
		{  -1.00624800f, -0.206843451f, -0.310932249f, -0.310595691f },
		{   463.669830f,  -121.143021f,  -151.065140f,  -150.401611f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::operator*(const __Matrix44&)");

	__Matrix44 mtx = mtxView * mtxProjection;
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, MultiplyAssign_Matrix_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -0.367074549f,  0.567012370f,  0.852347195f,  0.851424575f },
		{  0.000000000f,   1.29434156f, -0.423076659f, -0.422618717f },
		{  -1.00624800f, -0.206843451f, -0.310932249f, -0.310595691f },
		{   463.669830f,  -121.143021f,  -151.065140f,  -150.401611f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);

	SCOPED_TRACE("__Matrix44::operator*=(const __Matrix44&)");

	__Matrix44 mtx(mtxView);
	mtx *= mtxProjection;
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, AddAssign_Vector3_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ 1.07111096f,  0.000000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f,  1.42814803f, 0.000000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f,  1.00108361f,  1.00000000f },
		{ 128.000000f,   256.000000f,  511.499451f, 0.000000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 delta(128.0f, 256.0f, 512.0f);

	SCOPED_TRACE("__Matrix44::operator+=(const __Vector3&)");

	__Matrix44 mtx(mtxProjection);
	mtx += delta;
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, SubtractAssign_Vector3_MatchesReference)
{
	constexpr float ExpectedResult[4][4] =
	{
		{  1.07111096f, 0.000000000f, 0.000000000f, 0.000000000f },
		{ 0.000000000f,  1.42814803f, 0.000000000f, 0.000000000f },
		{ 0.000000000f, 0.000000000f,  1.00108361f,  1.00000000f },
		{ -128.000000f, -256.000000f, -512.500549f, 0.000000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Vector3 delta(128.0f, 256.0f, 512.0f);

	SCOPED_TRACE("__Matrix44::operator-=(const __Vector3&)");

	__Matrix44 mtx(mtxProjection);
	mtx -= delta;
	ExpectMatrixNear(mtx, expectedMatrix);
}

TEST_F(Matrix44Test, Multiply_Quaternion_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -175489.750f,  298334.406f, -105294.492f, 0.000000000f },
		{ -350981.656f, -198888.172f,  187190.219f, 0.000000000f },
		{  164017.531f, 0.000000000f, -41003.3828f,  1.00000000f },
		{ -82008.7656f, 0.000000000f,  20501.6914f, 0.000000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Quaternion quat = { 64.0f, 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Matrix44::operator*(const __Quaternion&)");

	__Matrix44 mtx = mtxProjection * quat;
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, MultiplyAssign_Quaternion_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -175489.750f,  298334.406f, -105294.492f, 0.000000000f },
		{ -350981.656f, -198888.172f,  187190.219f, 0.000000000f },
		{  164017.531f, 0.000000000f, -41003.3828f,  1.00000000f },
		{ -82008.7656f, 0.000000000f,  20501.6914f, 0.000000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Quaternion quat = { 64.0f, 128.0f, 256.0f, 512.0f };

	SCOPED_TRACE("__Matrix44::operator*=(const __Quaternion&)");

	__Matrix44 mtx(mtxProjection);
	mtx *= quat;
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}

TEST_F(Matrix44Test, Assign_Quaternion_MatchesReferenceWithinTolerance)
{
	constexpr float ExpectedResult[4][4] =
	{
		{ -25.0000000f,  28.0000000f,   -10.0000000f,  0.00000000f },
		{ -20.0000000f, -19.0000000f,    20.0000000f,  0.00000000f },
		{  22.0000000f,  4.00000000f,   -9.00000000f,  0.00000000f },
		{  0.00000000f,  0.00000000f,    0.00000000f,  1.00000000f }
	};

	const __Matrix44 expectedMatrix(ExpectedResult);
	const __Quaternion quat = { 1.0f, 2.0f, 3.0f, 4.0f };

	SCOPED_TRACE("__Matrix44::operator=(const __Quaternion&)");

	__Matrix44 mtx;
	mtx = quat;
	ExpectMatrixNear(mtx, expectedMatrix, EpsilonWithTolerance);
}
