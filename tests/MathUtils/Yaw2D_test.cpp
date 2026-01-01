#include <gtest/gtest.h>
#include "MathHelpers.h"

using test::EpsilonWithTolerance;

TEST(Yaw2D, MatchesReferenceWithinTolerance)
{
	float yaw;

	SCOPED_TRACE("Yaw2D::MatchesReferenceWithinTolerance");

	yaw = _Yaw2D(-1.0f, -1.0f);
	EXPECT_NEAR(yaw, 4.71238899f, EpsilonWithTolerance);

	yaw = _Yaw2D(-1.0f, 0.0f);
	EXPECT_NEAR(yaw, 4.71238899f, EpsilonWithTolerance);

	yaw = _Yaw2D(-1.0f, 1.0f);
	EXPECT_NEAR(yaw, 4.71238899f, EpsilonWithTolerance);

	yaw = _Yaw2D(0.0f, -1.0f);
	EXPECT_NEAR(yaw, 3.14159274f, EpsilonWithTolerance);

	yaw = _Yaw2D(0.0f, 0.0f);
	EXPECT_NEAR(yaw, 0.0f, EpsilonWithTolerance);

	yaw = _Yaw2D(0.0f, 1.0f);
	EXPECT_NEAR(yaw, 0.0f, EpsilonWithTolerance);

	yaw = _Yaw2D(1.0f, -1.0f);
	EXPECT_NEAR(yaw, 1.57079637f, EpsilonWithTolerance);

	yaw = _Yaw2D(1.0f, 0.0f);
	EXPECT_NEAR(yaw, 1.57079637f, EpsilonWithTolerance);

	yaw = _Yaw2D(1.0f, 1.0f);
	EXPECT_NEAR(yaw, 1.57079637f, EpsilonWithTolerance);
}
