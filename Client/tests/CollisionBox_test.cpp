#include <gtest/gtest.h>
#include "MathHelpers.h"

namespace
{
	const __Vector3 Min = { 0.0f, 0.0f, 0.0f };
	const __Vector3 Max = { 1.0f, 1.0f, 1.0f };
}

TEST(CheckCollisionByBox, HitsFrontFace)
{
	__Vector3 orig = { 0.5f, 0.5f, -1.0f };
	__Vector3 dir = { 0.0f, 0.0f, 1.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsBackFace)
{
	__Vector3 orig = { 0.5f, 0.5f, 2.0f };
	__Vector3 dir = { 0.0f, 0.0f, -1.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsTopFace)
{
	__Vector3 orig = { 0.5f, 2.0f, 0.5f };
	__Vector3 dir = { 0.0f, -1.0f, 0.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsBottomFace)
{
	__Vector3 orig = { 0.5f, -1.0f, 0.5f };
	__Vector3 dir = { 0.0f, 1.0f, 0.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsLeftFace)
{
	__Vector3 orig = { -1.0f, 0.5f, 0.5f };
	__Vector3 dir = { 1.0f, 0.0f, 0.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsRightFace)
{
	__Vector3 orig = { 2.0f, 0.5f, 0.5f };
	__Vector3 dir = { -1.0f, 0.0f, 0.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsEdge)
{
	__Vector3 orig = { -1.0f, 0.0f, 0.0f };
	__Vector3 dir = { 1.0f, 0.0f, 0.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsVertex)
{
	__Vector3 orig = { -1.0f, -1.0f, -1.0f };
	__Vector3 dir = { 1.0f, 1.0f, 1.0f };
	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayMissesBox)
{
	__Vector3 orig = { -1.0f, -1.0f, -1.0f };
	__Vector3 dir = { -1.0f, -1.0f, 0.0f };
	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayStartsInsideBox)
{
	__Vector3 orig = { 0.5f, 0.5f, 0.5f };
	__Vector3 dir = { 1.0f, 0.0f, 0.0f };

	// NOTE: _CheckCollisionByBox() does not detect rays starting inside,
	// so this *should* fail.
	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayParallelToFacesMisses)
{
	__Vector3 orig = { 2.0f, 2.0f, 2.0f };
	__Vector3 dir = { 0.0f, 1.0f, 0.0f };
	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}
