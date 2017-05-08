/*
 * Copyright (c) 2017 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
*/

#ifndef PONOS_GEOMETRY_SPHERE_H
#define PONOS_GEOMETRY_SPHERE_H

#include "geometry/surface.h"
#include "geometry/shape.h"
#include "geometry/transform.h"

namespace ponos {

class Circle : public Shape {
public:
  Circle() { r = 0.f; }
  Circle(Point2 center, float radius) : c(center), r(radius) {
    this->type = ShapeType::SPHERE;
  }
  virtual ~Circle() {}

  Point2 c;
  float r;
};

class Sphere : public SurfaceInterface {
public:
  Sphere() : r(0.f) {}

  Sphere(Point3 center, float radius) : c(center), r(radius) {}
  virtual ~Sphere() {}

  Point3 closestPoint(const Point3 &p) const override {
    return c + r * vec3(this->closestNomal(p));
  }
  Normal closestNomal(const Point3 &p) const override {
    if (c == p)
      return Normal(1, 0, 0);
    vec3 n = normalize(c - p);
    return Normal(n.x, n.y, n.z);
  }
  BBox boundingBox() const override { return BBox(c - r, c + r); }
  void closestIntersection(const Ray3 &r,
                           SurfaceRayIntersection *i) const override {}
  Point3 c;
  float r;
};

class ImplicitSphere final : public ImplicitSurfaceInterface {
  ImplicitSphere() : r(0.f) {}

  ImplicitSphere(Point3 center, float radius) : c(center), r(radius) {}
  ~ImplicitSphere() {}

  Point3 closestPoint(const Point3 &p) const override {
    return c + r * vec3(this->closestNomal(p));
  }
  Normal closestNomal(const Point3 &p) const override {
    if (c == p)
      return Normal(1, 0, 0);
    vec3 n = normalize(c - p);
    return Normal(n.x, n.y, n.z);
  }
  BBox boundingBox() const override { return BBox(c - r, c + r); }
  void closestIntersection(const Ray3 &r,
                           SurfaceRayIntersection *i) const override {}
  double signedDistance(const Point3 &p) const override {
    return distance(c, p) - r;
  }

  Point3 c;
  float r;
};

inline BBox2D compute_bbox(const Circle &po, const Transform2D *t = nullptr) {
  BBox2D b;
  ponos::Point2 center = po.c;
  if (t != nullptr) {
    b = make_union(b, (*t)(center + ponos::vec2(po.r, 0)));
    b = make_union(b, (*t)(center + ponos::vec2(-po.r, 0)));
    b = make_union(b, (*t)(center + ponos::vec2(0, po.r)));
    b = make_union(b, (*t)(center + ponos::vec2(0, -po.r)));
  } else {
    b = make_union(b, center + ponos::vec2(po.r, 0));
    b = make_union(b, center + ponos::vec2(-po.r, 0));
    b = make_union(b, center + ponos::vec2(0, po.r));
    b = make_union(b, center + ponos::vec2(0, -po.r));
  }
  return b;
}

} // ponos namespace

#endif
