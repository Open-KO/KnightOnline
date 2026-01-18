#ifndef MATHUTILS_GEOMETRICSTRUCTS_H
#define MATHUTILS_GEOMETRICSTRUCTS_H

#pragma once

// replacement for CSize
struct _SIZE
{
	long cx;
	long cy;
};

// replacement for CPoint
struct _POINT
{
	long x;
	long y;
};

// replacement for CRect
struct _RECT
{
	long left;
	long top;
	long right;
	long bottom;
};

inline bool IsPointInRect(const _POINT p, const _RECT r)
{
	return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
}

#endif // MATHUTILS_GEOMETRICSTRUCTS_H
