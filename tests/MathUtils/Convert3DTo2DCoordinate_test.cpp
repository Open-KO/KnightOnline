#include <gtest/gtest.h>
#include "MathHelpers.h"

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

TEST(Convert3DTo2DCoordinate, ReturnedPoint_MatchesReference)
{
	const __Matrix44 viewMatrix(View), projectionMatrix(Projection);
	const __Vector3 pos = { 311.0f, 1.0f, 351.0f };

	SCOPED_TRACE("Convert3DTo2DCoordinate::ReturnedPoint_MatchesReference");

	_POINT pt = _Convert3D_To_2DCoordinate(pos, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight);

	EXPECT_EQ(pt.x, 130);
	EXPECT_EQ(pt.y, 1633);
}

TEST(Convert3DTo2DCoordinate, PointInFrontOfCamera_MapsInsideViewport)
{
	const __Vector3 pos = { 0.0f, 0.0f, 1.0f };

	__Matrix44 viewMatrix, projectionMatrix;
	viewMatrix.Identity();
	projectionMatrix.Identity();

	SCOPED_TRACE("Convert3DTo2DCoordinate::PointInFrontOfCamera_MapsInsideViewport");

	_POINT pt = _Convert3D_To_2DCoordinate(pos, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight);

	EXPECT_GE(pt.x, 0);
	EXPECT_LT(pt.x, static_cast<int>(ViewportWidth));
	EXPECT_GE(pt.y, 0);
	EXPECT_LT(pt.y, static_cast<int>(ViewportHeight));
}

TEST(Convert3DTo2DCoordinate, PointBehindCamera_IsInvalid)
{
	const __Vector3 pos = { 0.0f, 0.0f, -1.0f };

	__Matrix44 viewMatrix, projectionMatrix;
	viewMatrix.Identity();
	projectionMatrix.Identity();

	SCOPED_TRACE("Convert3DTo2DCoordinate::PointBehindCamera_IsInvalid");

	_POINT pt = _Convert3D_To_2DCoordinate(pos, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight);

	EXPECT_EQ(pt.x, -1);
	EXPECT_EQ(pt.y, -1);
}

TEST(Convert3DTo2DCoordinate, PointOutsideFarPlane_IsInvalid)
{
	const __Vector3 pos = { 0.0f, 0.0f, 2.0f };

	__Matrix44 viewMatrix, projectionMatrix;
	viewMatrix.Identity();
	projectionMatrix.Identity();

	SCOPED_TRACE("Convert3DTo2DCoordinate::PointOutsideFarPlane_IsInvalid");

	_POINT pt = _Convert3D_To_2DCoordinate(pos, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight);

	EXPECT_EQ(pt.x, -1);
	EXPECT_EQ(pt.y, -1);
}

TEST(Convert3DTo2DCoordinate, WorldOrigin_MapsToMiddleOfScreen)
{
	const __Vector3 pos = { 0.0f, 0.0f, 0.5f };

	__Matrix44 viewMatrix, projectionMatrix;
	viewMatrix.Identity();
	projectionMatrix.Identity();

	SCOPED_TRACE("Convert3DTo2DCoordinate::WorldOrigin_MapsToMiddleOfScreen");

	_POINT pt = _Convert3D_To_2DCoordinate(pos, viewMatrix, projectionMatrix, ViewportWidth, ViewportHeight);

	EXPECT_EQ(pt.x, ViewportWidth / 2);
	EXPECT_EQ(pt.y, ViewportHeight / 2);
}
