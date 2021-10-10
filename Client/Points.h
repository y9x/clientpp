#pragma once
#include <Windows.h>
#include <ostream>

// manage RECT and retrieve unique pointer on stack
class Rect2D {
private:
	RECT rect{};
public:
	long x = 0;
	long y = 0;
	long width = 0;
	long height = 0;
	Rect2D() {}
	Rect2D(long lX, long lY, long lWidth, long lHeight) : x(lX), y(lY), width(lWidth), height(lHeight) {}
	RECT* get() {
		rect.left = x;
		rect.top = y;
		rect.right = x + width;
		rect.bottom = y + height;
		return &rect;
	}
};

struct Vector2 {
	double x = 0;
	double y = 0;
	bool operator = (POINT p) {
		x = p.x;
		y = p.y;
		return true;
	}
	operator POINT () {
		POINT point;
		point.x = (LONG)x;
		point.y = (LONG)y;
		return point;
	}
};

#define RECT_ARGS(r) r.left, r.top, r.right - r.left, r.bottom - r.top