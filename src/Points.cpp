#include "./Points.h"

Rect2D::Rect2D(long lX, long lY, long lWidth, long lHeight)
    : x(lX), y(lY), width(lWidth), height(lHeight) {}

RECT *Rect2D::get() {
  rect.left = x;
  rect.top = y;
  rect.right = x + width;
  rect.bottom = y + height;
  return &rect;
}

std::ostream &operator<<(std::ostream &o, const Vector2 &vec) {
  o << vec.x << "," << vec.y;
  return o;
}

Vector2::Vector2() {}

Vector2::Vector2(double a, double b) : x(a), y(b) {}

Vector2::Vector2(POINT p) {
  x = p.x;
  y = p.y;
}

bool Vector2::operator=(POINT p) {
  x = p.x;
  y = p.y;
  return true;
}

Vector2 Vector2::operator+(Vector2 v) { return {x + v.x, y + v.y}; }

void Vector2::clear() {
  x = 0;
  y = 0;
}

bool Vector2::operator+=(Vector2 v) {
  x += v.x;
  y += v.y;
  return true;
}

Vector2::operator POINT() {
  POINT point;
  point.x = (LONG)x;
  point.y = (LONG)y;
  return point;
}