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

#ifndef CIRCE_SCENE_SCENE_H
#define CIRCE_SCENE_SCENE_H

#include <circe/scene/scene_object.h>
#include <circe/utils/open_gl.h>

#include <ponos/ponos.h>

#include <memory>

namespace circe {

/** \brief The scene stores the list of objects to be rendered.
 *
 * It is possible to define how these objects are arranged in memory by setting
 * a **StructureType**. The default organization is a flat array with no
 * acceleration schemes.
 */
template <template <typename> class StructureType = ponos::Array> class Scene {
public:
  Scene() {}
  virtual ~Scene() {}

  /** \brief  add
   * \param o pointer to the object
   */
  template <typename T = SceneObject> T *add(SceneObject *o) {
    s.add(o);
    return static_cast<T *>(o);
  }

  void render(CameraInterface *camera) {
    s.iterate([&](SceneObject *o) {
      if (o->visible)
        o->draw(camera, transform);
    });
  }

  /** \brief intersects ray with objects
   * \param ray
   * \param t [optional] stores the parametric coordinates of the ray's
   *intersection point
   * \returns the closest object intersected
   */
  SceneObject *intersect(const ponos::Ray3 &ray, float *t = nullptr) {
    ponos::Transform tr = ponos::inverse(transform);
    ponos::point3 ta = tr(ray.o);
    ponos::point3 tb = tr(ray(1.f));
    return s.intersect(ponos::Ray3(ta, tb - ta), t);
  }

  /** \brief  iterate all objects
   * \param f function called to each object
   */
  void iterateObjects(std::function<void(const SceneObject *o)> f) const {
    s.iterate(f);
  }

  ponos::Transform
      transform; //!< scene transform (applied to all objects on draw)

private:
  StructureType<SceneObject> s;
};

} // namespace circe

#endif // CIRCE_SCENE_SCENE_H
