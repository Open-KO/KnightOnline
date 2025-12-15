#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::ExpectVector3Near;

TEST(IntersectTriangle, HitsTriangleFrontFace)
{
	const __Vector3 orig = { 0.0f, 0.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle::HitsTriangleFrontFace");

	EXPECT_TRUE(_IntersectTriangle(orig, dir, v0, v1, v2));
}

TEST(IntersectTriangle, MissesTriangle)
{
	const __Vector3 orig = { 5.0f, 5.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 1.0f, -1.0f, 0.0f };
	const __Vector3 v2 = { 0.0f,  1.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle::MissesTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2));
}

TEST(IntersectTriangle, RayParallelToTriangle)
{
	const __Vector3 orig = { 0.0f, 0.0f, 1.0f };
	const __Vector3 dir = { 1.0f, 0.0f, 0.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 1.0f, -1.0f, 0.0f };
	const __Vector3 v2 = { 0.0f,  1.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle::RayParallelToTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2));
}

TEST(IntersectTriangle, RayBehindTriangle)
{
	const __Vector3 orig = { 0.0f, 0.0f, 5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 1.0f, -1.0f, 0.0f };
	const __Vector3 v2 = { 0.0f,  1.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle::RayBehindTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2));
}

TEST(IntersectTriangle, HitsTriangleAtVertex)
{
	const __Vector3 orig = { -1.0f, -1.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle::HitsTriangleAtVertex");

	EXPECT_TRUE(_IntersectTriangle(orig, dir, v0, v1, v2));
}

TEST(IntersectTriangle_WithColInfo, HitsTriangleFrontFace)
{
	const __Vector3 orig = { 0.0f, 0.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t, u, v;
	__Vector3 col;

	SCOPED_TRACE("IntersectTriangle_WithColInfo::HitsTriangleFrontFace");

	ASSERT_TRUE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
	ExpectVector3Near(col, { 0.0f, 0.0f, 0.0f });

	EXPECT_GT(t, 0.0f);
	EXPECT_GE(u, 0.0f);
	EXPECT_GE(v, 0.0f);
	EXPECT_LE(u + v, 1.0f);
}

TEST(IntersectTriangle_WithColInfo, MissesTriangle)
{
	const __Vector3 orig = { 5.0f, 5.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t = -FLT_MIN, u = -FLT_MIN, v = -FLT_MIN;
	__Vector3 col = { -FLT_MIN, -FLT_MIN, -FLT_MIN };

	SCOPED_TRACE("IntersectTriangle_WithColInfo::MissesTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
	ExpectVector3Near(col, { -FLT_MIN, -FLT_MIN, -FLT_MIN });
}

TEST(IntersectTriangle_WithColInfo, RayParallelToTriangle)
{
	const __Vector3 orig = { 0.0f, 0.0f, 1.0f };
	const __Vector3 dir = { 1.0f, 0.0f, 0.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t, u, v;
	__Vector3 col = { 0.0f, 0.0f, 0.0f };

	SCOPED_TRACE("IntersectTriangle_WithColInfo::RayParallelToTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
}

TEST(IntersectTriangle_WithColInfo, RayBehindTriangle)
{
	const __Vector3 orig = { 0.0f, 0.0f, 5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t = -FLT_MIN, u = -FLT_MIN, v = -FLT_MIN;
	__Vector3 col = { -FLT_MIN, -FLT_MIN, -FLT_MIN };

	SCOPED_TRACE("IntersectTriangle_WithColInfo::RayBehindTriangle");

	EXPECT_FALSE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
	ExpectVector3Near(col, { -FLT_MIN, -FLT_MIN, -FLT_MIN });
}

TEST(IntersectTriangle_WithColInfo, HitsTriangleAtVertex)
{
	const __Vector3 orig = { -1.0f, -1.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t, u, v;
	__Vector3 col;

	SCOPED_TRACE("IntersectTriangle_WithColInfo::HitsTriangleAtVertex");

	ASSERT_TRUE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
	ExpectVector3Near(col, { -1.0f, -1.0f, 0.0f });

	EXPECT_GT(t, 0.0f);
	EXPECT_GE(u, 0.0f);
	EXPECT_GE(v, 0.0f);
	EXPECT_LE(u + v, 1.0f);
}

TEST(IntersectTriangle_WithColInfo, RayHitsTriangleEdge)
{
	const __Vector3 orig = { 0.0f, -1.0f, -5.0f };
	const __Vector3 dir = { 0.0f, 0.0f, 1.0f };

	const __Vector3 v0 = { -1.0f, -1.0f, 0.0f };
	const __Vector3 v1 = { 0.0f,  1.0f, 0.0f };
	const __Vector3 v2 = { 1.0f, -1.0f, 0.0f };

	float t, u, v;
	__Vector3 col;

	SCOPED_TRACE("IntersectTriangle_WithColInfo::RayHitsTriangleEdge");

	ASSERT_TRUE(_IntersectTriangle(orig, dir, v0, v1, v2, t, u, v, &col));
	ExpectVector3Near(col, { 0.0f, -1.0f, 0.0f });
}
