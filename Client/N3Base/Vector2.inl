#ifndef CLIENT_N3BASE_VECTOR2_INL
#define CLIENT_N3BASE_VECTOR2_INL

#pragma once

#include "My_3DStruct.h"

__Vector2::__Vector2(float fx, float fy)
{
	x = fx;
	y = fy;
}

void __Vector2::Zero()
{
	x = y = 0;
}

void __Vector2::Set(float fx, float fy)
{
	x = fx;
	y = fy;
}

#endif // CLIENT_N3BASE_VECTOR2_INL
