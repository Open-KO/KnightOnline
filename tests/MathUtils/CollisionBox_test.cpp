#include <gtest/gtest.h>
#include "MathHelpers.h"

namespace
{
	const __Vector3 Min = { 0.0f, 0.0f, 0.0f };
	const __Vector3 Max = { 1.0f, 1.0f, 1.0f };
}

TEST(CheckCollisionByBox, HitsFrontFace)
{
	const __Vector3 orig = { 0.5f, 0.5f, -1.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsFrontFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsBackFace)
{
	const __Vector3 orig = { 0.5f, 0.5f, 2.0f };
	const __Vector3 dir = { 0.0f, 0.0f, -1.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsBackFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsTopFace)
{
	const __Vector3 orig = { 0.5f, 2.0f, 0.5f };
	const __Vector3 dir = { 0.0f, -1.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsTopFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsBottomFace)
{
	const __Vector3 orig = { 0.5f, -1.0f, 0.5f };
	const __Vector3 dir = { 0.0f, 1.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsBottomFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsLeftFace)
{
	const __Vector3 orig = { -1.0f, 0.5f, 0.5f };
	const __Vector3 dir = { 1.0f, 0.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsLeftFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsRightFace)
{
	const __Vector3 orig = { 2.0f, 0.5f, 0.5f };
	const __Vector3 dir = { -1.0f, 0.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsRightFace");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsEdge)
{
	const __Vector3 orig = { -1.0f, 0.0f, 0.0f };
	const __Vector3 dir = { 1.0f, 0.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsEdge");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, HitsVertex)
{
	const __Vector3 orig = { -1.0f, -1.0f, -1.0f };
	const __Vector3 dir = { 1.0f, 1.0f, 1.0f };

	SCOPED_TRACE("CheckCollisionByBox::HitsVertex");

	EXPECT_TRUE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayMissesBox)
{
	const __Vector3 orig = { -1.0f, -1.0f, -1.0f };
	const __Vector3 dir = { -1.0f, -1.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::RayMissesBox");

	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayStartsInsideBox)
{
	const __Vector3 orig = { 0.5f, 0.5f, 0.5f };
	const __Vector3 dir = { 1.0f, 0.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::RayStartsInsideBox");

	// NOTE: _CheckCollisionByBox() does not detect rays starting inside,
	// so this *should* fail.
	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}

TEST(CheckCollisionByBox, RayParallelToFacesMisses)
{
	const __Vector3 orig = { 2.0f, 2.0f, 2.0f };
	const __Vector3 dir = { 0.0f, 1.0f, 0.0f };

	SCOPED_TRACE("CheckCollisionByBox::RayParallelToFacesMisses");

	EXPECT_FALSE(_CheckCollisionByBox(orig, dir, Min, Max));
}
