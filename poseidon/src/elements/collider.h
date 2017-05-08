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

#ifndef POSEIDON_ELEMENTS_COLLIDER_H
#define POSEIDON_ELEMENTS_COLLIDER_H

#include <ponos.h>

namespace poseidon {

struct Collision {
  float distance;
  ponos::Point3 point;
  ponos::Normal normal;
  ponos::vec3 velocity;
};

class Collider {
public:
  Collider() {}
  virtual ~Collider() {}
  void collide(const ponos::Point3 &p, const ponos::vec3 &v, float r, float rc,
               ponos::Point3 *np, ponos::vec3 *nv);
  void closestPoint(const ponos::SurfaceInterface &s, const ponos::Point3 &p,
                    ColliderQueryResult *r) const {
    r->distance = s->closestDistance(p);
    r->point = s->closestPoint(p);
    r->normal = s->closestNormal(p);
    r->velocity = getVelocity(p);
  }
  bool isPenetrating(const Collision &c, const ponos::Point3 &p, float r) {
    return ponos::dot(p - c.point, c.normal) < 0.f || c.distance < r;
  }
};

} // ponos namespace

#endif // POSEIDON_ELEMENTS_COLLIDER_H
