#include"Definitions.hpp"
#include"Math.hpp"

#include"MetricDescriptor.hpp"
#include"Photon.hpp"
#include"Tile.hpp"

#include "DensityDescriptor.hpp"

std::map<Tile*, DensityDescriptor*> MetricDescriptor::s_descriptorCache;

Mat3f MetricDescriptor::Rotations[RSize] = { Mat3f(1), Mat3f(0,-1,0, -1,0,0, 0,0,1), Mat3f(-1,0,0, 0,-1,0, 0,0,1), Mat3f(0,1,0, -1,0,0, 0,0,1)};
Mat3f MetricDescriptor::Mirros[MirrorSize] = { Mat3f(1), Mat3f(-1,0,0, 0,1,0, 0,0,1), Mat3f(1,0,0, 0,-1,0, 0,0,1)};


double MetricDescriptor::distance(Tile* const left, Tile* const right, Mat4f& transform) const {

	DensityDescriptor* left_desc = getDescriptor(left);
	DensityDescriptor* right_desc = getDescriptor(right);

	transform = Mat4f(1);
	double distance = DBL_MAX;
	//for(int r = 0; r < RSize; r++) {
	//	for(int m = 0; m < MirrorSize; m++) {
	//		
	//		double d = 0;
	//		float* values = 0;
	//		right_desc->value(Rotations[r] * Mirros[m], &values);
	//		for(size_t x = 0; x < m_resolution * m_resolution; x++) {
	//				d += (left_desc->value()[x] - values[x]) * (left_desc->value()[x] -  values[x]);
	//		}
	//		if( d < distance) {
	//			distance = d;
	//			transform = Mat4f(Rotations[r] * Mirros[m]);
	//		}
	//	}
	//}
	return glm::sqrt(distance);
}

std::vector<Vec3f> MetricDescriptor::getTexture(Tile* tile) {
	std::vector<Vec3f> tex;
	DensityDescriptor* desc = s_descriptorCache[tile];

	//for(size_t i = 0 ; i < desc->value().size(); i++) {
	//	tex.push_back(Vec3f(desc->value()[i], desc->value()[i], desc->value()[i]) / 500.0f);
	//}

	return tex;
}

DensityDescriptor* MetricDescriptor::getDescriptor(Tile* const tile) const {

	DensityDescriptor* desc = s_descriptorCache[tile];
	if(desc == 0) {
		desc =  new DensityDescriptor(m_resolution, m_photonMap);
		desc->rebuildDescriptor(tile);
		s_descriptorCache[tile] = desc;
		
	}
	return desc;
}

void MetricDescriptor::clearDescriptorCache() {
	s_descriptorCache.clear();
}