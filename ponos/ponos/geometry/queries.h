#ifndef PONOS_GEOMETRY_INTERSECTIONS_H
#define PONOS_GEOMETRY_INTERSECTIONS_H

#include <ponos/geometry/bbox.h>
#include <ponos/geometry/interval.h>
#include <ponos/geometry/line.h>
#include <ponos/geometry/plane.h>
#include <ponos/geometry/polygon.h>
#include <ponos/geometry/ray.h>
#include <ponos/geometry/segment.h>
#include <ponos/geometry/sphere.h>
#include <ponos/geometry/transform.h>
#include <ponos/geometry/utils.h>
#include <ponos/geometry/vector.h>
#include <optional>

namespace ponos {

class GeometricQueries {
public:
  ///
  /// \param box
  /// \param p
  /// \return
  static point3 closestPoint(const bbox3 &box, const point3 &p);
};

class GeometricPredicates {
public:
  ///
  /// \param bounds
  /// \param ray
  /// \param inv_dir
  /// \param dir_is_neg
  /// \param max_t
  /// \return
  static std::optional<real_t> intersect(const ponos::bbox3 &bounds,
                                         const ray3 &ray,
                                         const ponos::vec3 &inv_dir,
                                         const i32 dir_is_neg[3],
                                         real_t max_t = Constants::real_infinity);
  ///
  /// \param bounds
  /// \param ray
  /// \param second_hit
  /// \return
  static std::optional<real_t> intersect(const ponos::bbox3 &bounds,
                                         const ray3 &ray, real_t *second_hit = nullptr);

/// BBox / Ray3 intersection test.
/// **b1** and **b2**, if not null, receive the barycentric coordinates of the
/// intersection point.
/// \param p1 **[in]** first triangle's vertex
/// \param p2 **[in]** second triangle's vertex
/// \param p3 **[in]** third triangle's vertex
/// \param ray **[in]**
/// \param tHit **[out]** intersection (parametric coordinate)
/// \param b1 **[out]** barycentric coordinate
/// \param b2 **[out]** barycentric coordinate
/// return **true** if intersection exists
  static std::optional<real_t> intersect(const point3 &p1, const point3 &p2,
                                         const point3 &p3, const Ray3 &ray,
                                         real_t *b1 = nullptr, real_t *b2 = nullptr);
};

/** \brief  intersection test
 * \param a **[in]**
 * \param b **[in]**
 * /return
 */
bool bbox_bbox_intersection(const bbox2 &a, const bbox2 &b);
/** \brief  intersection test
 * \param a **[in]**
 * \param b **[in]**
 * /return
 */
bool bbox_bbox_intersection(const bbox3 &a, const bbox3 &b);
/** \brief  intersection test
 * \param a **[in]**
 * \param b **[in]**
 * /return
 */
template<typename T>
bool interval_interval_intersection(const Interval<T> &a,
                                    const Interval<T> &b) {
  return (a.low <= b.low && a.high >= b.low) ||
      (b.low <= a.low && b.high >= a.low);
}

/** \brief  intersection test
 * \param r **[in]** ray
 * \param s **[in]** segment
 * \param t **[out]** intersection point (ray coordinate)
 * /return **true** if intersection exists
 */
bool ray_segment_intersection(const Ray2 &r, const Segment2 &s,
                              real_t *t = nullptr);

/** \brief  intersection test
 * \param r **[in]** ray
 * \param s **[in]** segment
 * \param t **[out]** intersection point (ray coordinate)
 * /return **true** if intersection exists
 */
bool ray_segment_intersection(const Ray3 &r, const Segment3 &s,
                              real_t *t = nullptr);
/** \brief  intersection test
 * \param pl **[in]** plane
 * \param l **[in]** line
 * \param p **[out]** intersection point
 *
 * Plane / Line intersection test
 *
 * /return **true** if intersection exists
 */
bool plane_line_intersection(const Plane &pl, const Line &l, point3 &p);
/** \brief  intersection test
 * \param s **[in]** sphere
 * \param l **[in]** line
 * \param p1 **[out]** first intersection
 * \param p2 **[out]** second intersection
 *
 * Sphere / Line intersection test
 *
 * **p1** = **p2** if line is tangent to sphere
 *
 * /return **true** if intersection exists
 */
bool sphere_line_intersection(const Sphere s, const Line l, point3 &p1,
                              point3 &p2);
/** \brief  intersection test
 * \param s **[in]** sphere
 * \param r **[in]** ray
 * \param t1 **[out | optional]** closest intersection (parametric coordinate)
 * \param t2 **[out | optional]** second intersection (parametric coordinate)
 *
 * /return **true** if intersection exists
 */
bool sphere_ray_intersection(const Sphere &s, const Ray3 &r,
                             real_t *t1 = nullptr, real_t *t2 = nullptr);
/** \brief  intersection test
 * \param box **[in]**
 * \param ray **[in]**
 * \param hit1 **[out]** first intersection
 * \param hit2 **[out]** second intersection
 * \param normal **[out | optional]** collision normal
 *
 * bbox2D / Ray intersection test.
 *
 * **hit1** and **hit2** are in the ray's parametric coordinate.
 *
 * **hit1** = **hit2** if a single point is found.
 *
 * /return **true** if intersectiton exists
 */
bool bbox_ray_intersection(const bbox2 &box, const Ray2 &ray, real_t &hit1,
                           real_t &hit2, real_t *normal = nullptr);

/** \brief  intersection test
 * \param box **[in]**
 * \param ray **[in]**
 * \param hit1 **[out]** first intersection
 * \param hit2 **[out]** second intersection
 *
 * BBox / Ray3 intersection test.
 *
 * **hit1** and **hit2** are in the ray's parametric coordinate.
 *
 * **hit1** = **hit2** if a single point is found.
 *
 * /return **true** if intersectiton exists
 */
[[deprecated]]
bool bbox_ray_intersection(const bbox3 &box, const Ray3 &ray, real_t &hit1,
                           real_t &hit2);
/** \brief  intersection test
 * \param box **[in]**
 * \param ray **[in]**
 * \param hit1 **[out]** closest intersection
 *
 * BBox / Ray3 intersection test. Computes the closest intersection from the
 *ray's origin.
 *
 * **hit1** in in the ray's parametric coordinate.
 *
 * /return **true** if intersection exists
 */
bool bbox_ray_intersection(const bbox3 &box, const Ray3 &ray, real_t &hit1);
void triangle_barycentric_coordinates(const point2 &p, const point2 &a,
                                      const point2 &b, const point2 &c,
                                      real_t &u, real_t &v, real_t &w);
bool triangle_point_intersection(const point2 &p, const point2 *vertices);
/// BBox / Ray3 intersection test.
/// **b1** and **b2**, if not null, receive the barycentric coordinates of the
/// intersection point.
/// \param p1 **[in]** first triangle's vertex
/// \param p2 **[in]** second triangle's vertex
/// \param p3 **[in]** third triangle's vertex
/// \param ray **[in]**
/// \param tHit **[out]** intersection (parametric coordinate)
/// \param b1 **[out]** barycentric coordinate
/// \param b2 **[out]** barycentric coordinate
/// return **true** if intersection exists
bool triangle_ray_intersection(const point3 &p1, const point3 &p2,
                               const point3 &p3, const Ray3 &ray,
                               real_t *tHit = nullptr, real_t *b1 = nullptr,
                               real_t *b2 = nullptr);
///
/// \param p **[in]** source point
/// \param p1 **[in]** first triangle's vertex
/// \param p2 **[in]** second triangle's vertex
/// \param p3 **[in]** third triangle's vertex
/// \return closest point on triangle **(p1,p2,p3)** to point **p**
point3 closest_point_triangle(const point3 &p, const point3 &p1,
                              const point3 &p2, const point3 &p3);
/** \brief  closest point
 * \param p **[in]** point
 * \param pl **[in]** plane
 *
 * /return closest point on **pl** from **p**
 */
point3 closest_point_plane(const point3 &p, const Plane &pl);
/** \brief
 * \param s **[in]** segment
 * \param p **[in]** point
 * \param t **[in]** receives
 *
 * If the projected **p** on **s** lies outside **s**, **t** is clamped to
 *[0,1].
 *
 * /return closest point to **p** that lies on **s**
 */
template<typename T>
T closest_point_segment(const Segment<T> &s, const T &p, real_t *t = nullptr) {
  real_t t_ = dot(p - s.a, s.b - s.a) / (s.b - s.a).length2();
  if (t_ < 0.f)
    t_ = 0.f;
  if (t_ > 1.f)
    t_ = 1.f;
  if (t != nullptr)
    *t = t_;
  return s.a + t_ * (s.b - s.a);
}
/** \brief  closest point
 * \param p **[in]**
 * \param pl **[in]**
 *
 * Assumes **pl**'s normal is normalized.
 *
 * /return closest point on **pl** from **p**
 */
point3 closest_point_n_plane(const point3 &p, const Plane &pl);
/** \brief  closest point
 * \param p **[in]** point
 * \param b **[in]** bbox
 *
 * /return closest point on **b** from **p**
 */
point3 closest_point_bbox(const point3 &p, const bbox3 &b);
/// distance from point to line
/// \param p **[in]** point
/// \param l **[in]** line
/// \return distance from **p** to **l**
real_t distance_point_line(const point3 &p, const Line &l);
/// \brief  distance
/// \param p **[in]** point
/// \param pl **[in]** plane
/// \return signed distance of **p** to **pl**
real_t distance_point_plane(const point3 &p, const Plane &pl);
/** \brief  distance
 * \param p **[in]** point
 * \param pl **[in]** plane
 *
 * Assumes **pl**'s normal is normalized.
 *
 * /return signed distance of **p** to **pl**
 */
real_t distance_point_n_plane(const point3 &p, const Plane &pl);
/** \brief  distance
 * \param p **[in]** point
 * \param s **[in]** segment
 *
 * /return the squared distance between **p** and **s**
 */
template<typename T>
real_t distance2_point_segment(const T &p, const Segment<T> &s);
/** \brief  distance
 * \param p **[in]** point
 * \param b **[in]** bbox
 *
 * /return squared distance between **p** and **b**
 */
real_t distance2_point_bbox(const point3 &p, const bbox3 &b);

inline bbox2 compute_bbox(const Shape &po, const Transform2 *t = nullptr) {
  bbox2 b;
  if (po.type == ShapeType::POLYGON)
    b = compute_bbox(static_cast<const Polygon &>(po), t);
  else if (po.type == ShapeType::SPHERE)
    b = compute_bbox(static_cast<const Circle &>(po), t);
  return b;
}
} // namespace ponos

#endif
