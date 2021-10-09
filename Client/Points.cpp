#include "./Points.h"

std::ostream& operator << (std::ostream& o, const Vector2& vec) {
	o << vec.x << "," << vec.y;
	return o;
}