#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;
using test::ExpectVector3Near;

namespace
{
	constexpr uint32_t ViewportWidth = 1024;
	constexpr uint32_t ViewportHeight = 768;

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
}

TEST(Convert2DTo3DCoordinate, ReturnedPoint_MatchesReferenceWithinTolerance)
{
	const __Vector3 expectedPos = { 310.085724f, 13.3152275f, 347.672943f };
	const __Vector3 expectedDir = { 0.185912609f, -2.48673177f, 0.673640966f };

	const __Matrix44 viewMatrix(View), projectionMatrix(Projection);
	const _POINT pt = { 130, 1633 };

	__Vector3 posResult = { -FLT_MIN, -FLT_MIN, -FLT_MIN };
	__Vector3 dirResult = { -FLT_MIN, -FLT_MIN, -FLT_MIN };

	SCOPED_TRACE("Convert2DTo3DCoordinate::ReturnedPoint_MatchesReferenceWithinTolerance");

	_Convert2D_To_3DCoordinate(pt.x, pt.y, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight, posResult, dirResult);

	ExpectVector3Near(posResult, expectedPos, EpsilonWithTolerance);
	ExpectVector3Near(dirResult, expectedDir, EpsilonWithTolerance);
}
