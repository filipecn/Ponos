#ifndef PONOS_STRUCTURES_BAH_H
#define PONOS_STRUCTURES_BAH_H

#include <ponos/geometry/point.h>
#include <ponos/geometry/polygon.h>
#include <ponos/geometry/ray.h>
#include <ponos/geometry/transform.h>
#include <ponos/structures/mesh.h>

#include <memory>

namespace ponos {

/* hierarchical structure
 * Bounding Area Hierarchie.
 */
class BAH {
public:
  /* Constructor.
   * @m **[in]**
   */
  BAH(Mesh2D *p);
  virtual ~BAH() {}

  std::shared_ptr<Mesh2D> mesh;

  int intersect(const Ray2 &ray, float *t = nullptr);
  bool isInside(const point2 &p);

private:
  struct BAHElement {
    BAHElement(size_t i, const ponos::bbox3 &b)
        : ind(i), bounds(ponos::bbox2(b.lower.xy(), b.upper.xy())) {
      centroid = bounds.centroid();
    }
    size_t ind;
    ponos::bbox2 bounds;
    ponos::point2 centroid;
  };
  struct BAHNode {
    BAHNode() { children[0] = children[1] = nullptr; }
    void initLeaf(uint32_t first, uint32_t n, const ponos::bbox2 &b) {
      firstElementOffset = first;
      nElements = n;
      bounds = b;
    }
    void initInterior(uint32_t axis, BAHNode *c0, BAHNode *c1) {
      children[0] = c0;
      children[1] = c1;
      bounds = ponos::make_union(c0->bounds, c1->bounds);
      splitAxis = axis;
      nElements = 0;
    }
    ponos::bbox2 bounds;
    BAHNode *children[2];
    uint32_t splitAxis, firstElementOffset, nElements;
  };
  struct LinearBAHNode {
    ponos::bbox2 bounds;
    union {
      uint32_t elementsOffset;
      uint32_t secondChildOffset;
    };
    uint8_t nElements;
    uint8_t axis;
    uint8_t pad[2];
  };
  struct ComparePoints {
    ComparePoints(int d) { dim = d; }
    int dim;
    bool operator()(const BAHElement &a, const BAHElement &b) const {
      return a.centroid[dim] < b.centroid[dim];
    }
  };
  std::vector<uint32_t> orderedElements;
  std::vector<LinearBAHNode> nodes;
  BAHNode *root;
  BAHNode *recursiveBuild(std::vector<BAHElement> &buildData, uint32_t start,
                          uint32_t end, uint32_t *totalNodes,
                          std::vector<uint32_t> &orderedElements);
  uint32_t flattenBAHTree(BAHNode *node, uint32_t *offset);
  bool intersect(const ponos::bbox2 &bounds, const ponos::Ray2 &ray,
                 const ponos::vec2 &invDir, const uint32_t dirIsNeg[2]) const;
};

} // namespace ponos

#endif // AERGIA_SCENE_MESH_H
