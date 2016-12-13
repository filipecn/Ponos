#include "structures/mesh.h"

namespace ponos {

	Mesh::Mesh(const ponos::RawMesh *m, const ponos::Transform &t) {
		mesh.reset(m);
		transform = t;
		bbox = t(m->bbox);
	}

	bool Mesh::intersect(const ponos::Point3 &p) {
		ponos::Transform inv = ponos::inverse(transform);
		ponos::Point3 pp = inv(p);
		ponos::Vector<3, float> P(pp.x, pp.y, pp.z);
		//ponos::Ray3 r(inv(p), ponos::vec3(0, 1, 0));
		int hitCount = 0;
		if(P >= ponos::Vector<3, float>(-0.5f) &&
			 P <= ponos::Vector<3, float>(0.5f))
			hitCount = 1;
		return hitCount % 2;
	}

	const ponos::BBox& Mesh::getBBox() const {
		return bbox;
	}

	const ponos::RawMesh* Mesh::getMesh() const {
		return mesh.get();
	}

	const ponos::Transform& Mesh::getTransform() const {
		return transform;
	}

	Mesh2D::Mesh2D(const ponos::RawMesh *m, const ponos::Transform2D &t) {
		mesh.reset(m);
		transform = t;
		bbox = t(BBox2D(m->bbox.pMin.xy(), m->bbox.pMax.xy()));
	}

	bool Mesh2D::intersect(const ponos::Point2 &p) {
		ponos::Transform2D inv = ponos::inverse(transform);
		ponos::Point2 pp = inv(p);
		ponos::Vector<2, float> P(pp.x, pp.y);
		//ponos::Ray3 r(inv(p), ponos::vec3(0, 1, 0));
		int hitCount = 0;
		if(P >= ponos::Vector<2, float>(-0.5f) &&
			 P <= ponos::Vector<2, float>(0.5f))
			hitCount = 1;
		return hitCount % 2;
	}

	const ponos::BBox2D& Mesh2D::getBBox() const {
		return bbox;
	}

	const ponos::RawMesh* Mesh2D::getMesh() const {
		return mesh.get();
	}

	const ponos::Transform2D& Mesh2D::getTransform() const {
		return transform;
	}


} // aergia namespace