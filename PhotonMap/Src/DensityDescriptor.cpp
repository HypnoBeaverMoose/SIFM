#include"Definitions.hpp"
#include"Math.hpp"
#include"PhotonMap.hpp"
#include"Tile.hpp"
#include"DensityDescriptor.hpp"

DensityDescriptor::DensityDescriptor(size_t resolution, PhotonMap* const map): m_resolution(resolution), m_photonMap(map) {

}

void DensityDescriptor::rebuildDescriptor(Tile* tile,  size_t de_photons) {

	m_values.clear();

	float tileSize_obj = glm::compMax(glm::abs(tile->position(0) - tile->position(1)));
	float tileSize_world = glm::length(tile->worldPosition(0) - tile->worldPosition(1));
	float pixelSize = tileSize_obj / m_resolution;
	double densityRadius;
	Vec4f point(0,0,0,1);
	size_t index = 0;
	Vec3f variance;
	for(float y = pixelSize / 2 - tileSize_obj / 2; y < tileSize_obj / 2; y+=pixelSize) {	
		point.y = y;
		for(float x = pixelSize / 2 - tileSize_obj / 2; x < tileSize_obj / 2; x+=pixelSize) {
			point.x = x;			
			
			Vec3f density = m_photonMap->photonDensity(Vec3f(tile->transform() * point),
				tile->worldNormal(),de_photons,densityRadius,Vec3f(1),false, variance);

			density /=  (densityRadius * glm::pi<float>() * PhotonMap::globalPhotonCount);
			m_values.push_back(density.x);	
			m_values.push_back(density.y);	
			m_values.push_back(density.z);
			m_variance.push_back(variance.x);
			m_variance.push_back(variance.y);
			m_variance.push_back(variance.z);
		}
	}
}

bool DensityDescriptor::empty() const {
	
	return m_values.empty();
}

void DensityDescriptor::value(std::vector<float>& value) const {
	
	value.assign(m_values.begin(), m_values.end());
}

void DensityDescriptor::value(const Mat3f& transform, std::vector<float>& transformed_values) const {

	transformed_values.assign(m_values.size(), -1);
	rotate_descriptor(transform,m_values.data(), transformed_values.data(), m_resolution);

	//for(size_t i = 0; i < m_values.size(); i++) {
	//	assert(transformed_values[i] >= 0);
	//}
}


void DensityDescriptor::rotate_descriptor(const Mat3f& transform, const float* input,  const float* output, size_t resolution) {

	size_t size = resolution * resolution;
	Vec3f center_point((resolution - 1) / 2.0f, (resolution - 1) / 2.0f, 0);
	for(size_t i = 0; i < size; i++) {
		Vec3f point1(i % resolution, i / resolution, 1);
		Vec3f point2 =point1 - center_point;
		Vec3f point3 = transform * point2;
		Vec3f point = point3 + center_point;
		assert(point.x >= 0 && point.x < resolution && point.y >= 0 && point.y < resolution);
		size_t new_index = (size_t)(point.y * resolution + point.x);
		memcpy((void*)(&(output[new_index * 3])), &input[i * 3], sizeof(Vec3f));
	}

}


void DensityDescriptor::variance(std::vector<float>& variance) const {

	variance.assign(m_variance.begin(), m_variance.end());
}

void DensityDescriptor::variance(const Mat3f& transform, std::vector<float>& transformed_variance) const {

//	transformed_variance.assign(m_values.size(), -1);
	rotate_descriptor(transform,m_values.data(), transformed_variance.data(), m_resolution);

	//for(size_t i = 0; i < m_values.size(); i++) {
	//	assert(transformed_values[i] >= 0);
	//}
}