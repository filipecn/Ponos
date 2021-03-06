#include "structures/vdb_particle_grid.h"

//#include <openvdb_points/tools/PointAttribute.h>
//#include <openvdb_points/tools/PointConversion.h>
//#include <openvdb_points/tools/PointCount.h>

using namespace openvdb;
using namespace openvdb::tools;

namespace poseidon {

	VDBParticleGrid::VDBParticleGrid(const ponos::ivec3& d, const float& s, const ponos::vec3& o)
		: dimensions(d), updated(false) {
			// initialize libraries
			openvdb::initialize();
			openvdb::points::initialize();
			// Introduce a typedef for our position attribute (note no compression codec here)
			typedef TypedAttributeArray<Vec3f> PositionAttribute;
			typedef TypedAttributeArray<Vec3f> VelocityAttribute;
			typedef TypedAttributeArray<Int32> IdAttribute;
			// Create a list of names and attribute types (in this case, just position)
			AttributeSet::Descriptor::NameAndTypeVec attributes;
			attributes.push_back(AttributeSet::Descriptor::NameAndType("P", PositionAttribute::attributeType()));
			attributes.push_back(AttributeSet::Descriptor::NameAndType("V", VelocityAttribute::attributeType()));
			attributes.push_back(AttributeSet::Descriptor::NameAndType("I", IdAttribute::attributeType()));
			// Create an AttributeSet Descriptor for this list
			descriptor = AttributeSet::Descriptor::create(attributes);

			transform = openvdb::math::Transform::createLinearTransform(s);
			toWorld = ponos::scale(s, s, s);
			toGrid = ponos::inverse(toWorld);
			voxelSize = s;
			pl = nullptr;
		}

	void VDBParticleGrid::init() {
		if(updated)
			return;
		// Create a PointPartitioner-compatible point list using the std::vector wrapper provided
		const PointAttributeVector<openvdb::Vec3f> pointList(positions);
		pointIndexGrid = createPointIndexGrid<PointIndexGrid>(pointList, *transform);
		// Create the PointDataGrid, position attribute is mandatory
		pointDataGrid = createPointDataGrid<PointDataGrid>(*pointIndexGrid, pointList,
				TypedAttributeArray<openvdb::Vec3f>::attributeType(), *transform);

		// Add a new velocity attribute
		AttributeSet::Util::NameAndType velocityAttribute("V", TypedAttributeArray<openvdb::Vec3f>::attributeType());
		appendAttribute(pointDataGrid->tree(), velocityAttribute);
		const PointAttributeVector<Vec3f> vList(velocities);
		populateAttribute(pointDataGrid->tree(), pointIndexGrid->tree(), "V", vList);

		// Add a new id attribute
		AttributeSet::Util::NameAndType idAttribute("id", TypedAttributeArray<openvdb::Int32>::attributeType());
		appendAttribute(pointDataGrid->tree(), idAttribute);
		const PointAttributeVector<Int32> idList(ids);
		populateAttribute(pointDataGrid->tree(), pointIndexGrid->tree(), "id", idList);

		// Add a new id attribute
		AttributeSet::Util::NameAndType densityAttribute("D", TypedAttributeArray<float>::attributeType());
		appendAttribute(pointDataGrid->tree(), densityAttribute);
		const PointAttributeVector<float> densityList(densities);
		populateAttribute(pointDataGrid->tree(), pointIndexGrid->tree(), "D", densityList);

		pl = new PointList(particles);
		pointGridPtr = openvdb::tools::createPointIndexGrid<PointIndexGrid>(*pl, *transform);
		filter = new openvdb::tools::PointIndexFilter<PointList>(*pl, pointGridPtr->tree(), pointGridPtr->transform());
		updated = true;
	}

	void VDBParticleGrid::addParticle(Particle *p) {
		ponos::ivec3 i = worldToIndex(p->position);
		if(!(i >= ponos::ivec3()) || !(i < dimensions))
			return;
		ids.emplace_back(positions.size());
		positions.emplace_back(Vec3f(p->position.x, p->position.y, p->position.z));
		velocities.emplace_back(Vec3f(p->velocity.x, p->velocity.y, p->velocity.z));
		densities.emplace_back(p->density);
		particles.emplace_back(p);
		updated = false;
	}

	void VDBParticleGrid::addParticle(const ponos::Point3& p, const ponos::vec3& v) {
		ponos::ivec3 i = worldToIndex(p);
		if(!(i >= ponos::ivec3()) || !(i < dimensions))
			return;
		ids.emplace_back(positions.size());
		positions.emplace_back(Vec3f(p.x, p.y, p.z));
		velocities.emplace_back(Vec3f(v.x, v.y, v.z));
		densities.emplace_back(0.f);
		updated = false;
	}

	void VDBParticleGrid::addParticle(const ponos::ivec3& c, const ponos::Point3& p, const ponos::vec3& v) {
		ponos::Point3 tp = p + ponos::vec3(c[0], c[1], c[2]);
		addParticle(toWorld(tp), v);
	}

	void VDBParticleGrid::setParticle(int id, const ponos::Point3& p, const ponos::vec3& v) {
		/*AttributeWriteHandle<Vec3f>::Ptr attributeWriteHandle =
			AttributeWriteHandle<Vec3f>::create(leaf->attributeArray("P"));
		attributeWriteHandle->set(id, openvdb::Vec3f(p.x, p.y, p.z));
		attributeWriteHandle =
			AttributeWriteHandle<Vec3f>::create(leaf->attributeArray("V"));
		attributeWriteHandle->set(id, Vec3f(v.x, v.y, v.z));*/
		positions[id] = Vec3f(p.x, p.y, p.z);
		velocities[id] = Vec3f(v.x, v.y, v.z);
		updated = false;
	}

	int VDBParticleGrid::particleCount(const ponos::ivec3& ijk) {
		const openvdb::Coord _ijk(ijk[0], ijk[1], ijk[2]);
		openvdb::tools::PointDataTree::LeafNodeType* leaf = pointDataGrid->tree().touchLeaf(_ijk);
		IndexIter indexIter = leaf->beginIndex(_ijk);
		return iterCount(indexIter);
	}

	void VDBParticleGrid::iterateNeighbours(ponos::BBox bbox, std::function<void(const Particle& p)> f) {}

	void VDBParticleGrid::iterateNeighbours(ponos::Point3 center, float radius, std::function<void(const Particle& p)> f) {}

	void VDBParticleGrid::iterateCellNeighbours(const ponos::ivec3& c, const ponos::ivec3& d, std::function<void(const size_t& id)> f) {
		ponos::ivec3 pMin(0, 0, 0);
		ponos::ivec3 coord;
		int& x = coord[0], &y = coord[1], &z = coord[2];
		for(x = c[0] - d[0]; x <= c[0] + d[0]; x++)
			for(y = c[1] - d[1]; y <= c[1] + d[1]; y++)
				for(z = c[2] - d[2]; z <= c[2] + d[2]; z++) {
					if(!(coord < dimensions) || !(coord >= pMin))
						continue;
					// iterate cell particles
					iterateCell(coord, f);
				}
	}

	void VDBParticleGrid::iterateCell(const ponos::ivec3& c, const std::function<void(const size_t& id)>& f) {
		// Create a co-ordinate to perform the look-up and voxel position in index space
		const openvdb::Coord ijk(c[0], c[1], c[2]);
		openvdb::tools::PointDataTree::LeafNodeType* leaf = pointDataGrid->tree().touchLeaf(ijk);

		// Retrieve a read-only attribute handle for position
		AttributeHandle<openvdb::Int32>::Ptr attributeHandle =
			AttributeHandle<openvdb::Int32>::create(leaf->attributeArray("id"));

		// Create an IndexIter for accessing the co-ordinate
		IndexIter indexIter = leaf->beginIndex(ijk);

		// Iterate over all the points in the voxel
		for (; indexIter; ++indexIter) {
			f(attributeHandle->get(*indexIter));
		}
	}

	int VDBParticleGrid::particleCount() {
		if(pointDataGrid == nullptr)
			return 0;
		return pointCount(pointDataGrid->tree());
	}

	ponos::ivec3 VDBParticleGrid::worldToIndex(const ponos::Point3& wp) {
		ponos::Point3 gp = toGrid(wp) + ponos::vec3(0.5f, 0.5f, 0.5f);
		return ponos::ivec3(gp.x, gp.y, gp.z);
	}

	ponos::ivec3 VDBParticleGrid::gridToIndex(const ponos::Point3& gp) {
		ponos::Point3 p = gp + ponos::vec3(0.5f, 0.5f, 0.5f);
		return ponos::ivec3(p.x, p.y, p.z);
	}

	ponos::Point3 VDBParticleGrid::worldToVoxel(const ponos::Point3& wp) {
		ponos::Point3 gp = toGrid(wp);
		ponos::ivec3 ip = gridToIndex(gp);
		return ponos::Point3(gp - ponos::vec3(ip[0], ip[1], ip[2]));
	}

	ponos::Point3 VDBParticleGrid::gridToVoxel(const ponos::Point3& gp) {
		ponos::ivec3 ip = gridToIndex(gp);
		return ponos::Point3(gp - ponos::vec3(ip[0], ip[1], ip[2]));
	}

	ponos::Point3 VDBParticleGrid::indexToWorld(const ponos::ivec3& i) {
		return toWorld(ponos::Point3(i[0], i[1], i[2]));
	}

	void VDBParticleGrid::cellVertices(const ponos::ivec3& ijk, std::vector<ponos::Point3>& v) {
		v.clear();
		ponos::Point3 wp = indexToWorld(ijk);
		for(float x = -1.f; x <= 1.f; x += 1.f)
			for(float y = -1.f; y <= 1.f; y += 1.f)
				for(float z = -1.f; z <= 1.f; z += 1.f)
					v.push_back(wp + toWorld(ponos::vec3(x, y, z) * 0.5f));
	}

	float VDBParticleGrid::computeDensity(float d, float md) {
		const double radius = 1.5 * voxelSize;

		WeightedAverageAccumulator<double> accumulator(particles, radius, ParticleAttribute::DENSITY);

		double sum = 0.f;

		for(size_t n = 0, N = particles.size(); n < N; n++) {
			continue;
			accumulator.reset();
			openvdb::Vec3d p(particles[n]->position.x, particles[n]->position.y, particles[n]->position.z);
			filter->searchAndApply(p, radius, accumulator);
			sum += accumulator.result();
		}

		float maxd = dimensions.max();
		ponos::parallel_for(0, positions.size(), [this, maxd, d, md](size_t f, size_t l) {
				for(size_t id = f; id <= l; id++) {
				ponos::ivec3 cell = worldToIndex(ponos::Point3(positions[id].asPointer()));
				float sum = 0.f;
				iterateCellNeighbours(cell, ponos::ivec3(1, 1, 1), [this, &sum, d, id, maxd](const size_t& i) {
						float distance = ponos::distance2(
								ponos::Point3(positions[id].asPointer()),
								ponos::Point3(positions[i].asPointer()));
						float weight = particleMass * ponos::smooth(distance, 4.f * d / maxd);
						sum += weight;
						});
				densities[id] = sum / md;
				}
				});
		size_t _md = ponos::parallel_max(0, densities.size(), &densities[0]);
		return densities[_md];
	}

	const std::vector<Particle*>& VDBParticleGrid::getParticles() const {
		return particles;
	}

	float VDBParticleGrid::gather(ParticleAttribute a, const ponos::Point3& p, float r) const {
		float radius = r < 0.f ? 1.5f * voxelSize : r;
		WeightedAverageAccumulator<double> accumulator(particles, radius, a);
		openvdb::Vec3d vdbp(p.x, p.y, p.z);
		filter->searchAndApply(vdbp, radius, accumulator);
		return accumulator.result();
	}

	float VDBParticleGrid::cellSDF(const ponos::ivec3& ijk, float d, ParticleType t) {
		float acc = 0.0f;
		bool solid = false;
		iterateCell(ijk, [this, &acc, &solid, t] (const size_t& id) {
				if(particles[id]->type == t)
					acc += particles[id]->density;
					else solid = true;
				});
		if(solid)
			return 1.f;
		float n0 = 1.0f / (d * d * d);
		return 0.2f * n0 - acc;
	}

} // poseidon namespace
