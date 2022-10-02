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
	Rect2D(long lX, long lY, long lWidth, long lHeight);
	RECT* get();
};

struct Vector2 {
	double x = 0;
	double y = 0;
	bool operator = (POINT point);
	bool operator += (Vector2 vector);
	Vector2 operator + (Vector2 vector);
	operator POINT ();
	void clear();
	Vector2(POINT p);
	Vector2(double x, double y);
	Vector2();
};

std::ostream& operator << (std::ostream& o, const Vector2& vec);

#define RECT_ARGS(r) r.left, r.top, r.right - r.left, r.bottom - r.top