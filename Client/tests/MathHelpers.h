#ifndef CLIENT_TESTS_MATHHELPERS_H
#define CLIENT_TESTS_MATHHELPERS_H

#include <N3Base/My_3DStruct.h>

namespace test
{
	static constexpr float Epsilon				= std::numeric_limits<float>::epsilon();
	static constexpr float EpsilonWithTolerance	= 1e-3f;

	inline static void ExpectVector2Near(const __Vector2& a, const __Vector2& b, float epsilon = Epsilon)
	{
		EXPECT_NEAR(a.x, b.x, epsilon);
		EXPECT_NEAR(a.y, b.y, epsilon);
	}

	inline static void ExpectVector3Near(const __Vector3& a, const __Vector3& b, float epsilon = Epsilon)
	{
		EXPECT_NEAR(a.x, b.x, epsilon);
		EXPECT_NEAR(a.y, b.y, epsilon);
		EXPECT_NEAR(a.z, b.z, epsilon);
	}

	inline static void ExpectVector4Near(const __Vector4& a, const __Vector4& b, float epsilon = Epsilon)
	{
		EXPECT_NEAR(a.x, b.x, epsilon);
		EXPECT_NEAR(a.y, b.y, epsilon);
		EXPECT_NEAR(a.z, b.z, epsilon);
		EXPECT_NEAR(a.w, b.w, epsilon);
	}

	inline static void ExpectMatrixNear(const __Matrix44& a, const __Matrix44& b, float epsilon = Epsilon)
	{
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
				EXPECT_NEAR(a.m[row][col], b.m[row][col], epsilon);
		}
	}
}

#endif // CLIENT_TESTS_MATHHELPERS_H
