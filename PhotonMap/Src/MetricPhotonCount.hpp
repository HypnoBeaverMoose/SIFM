#pragma once
#include"IMetric.hpp"

#include "Tile.hpp"
class MetricPhotonCount: public IMetric {
public:
	MetricPhotonCount(size_t maxPhotonCount) : m_maxPhotons(maxPhotonCount) {};

	double distance(Tile* const left, Tile* const right, Mat4f& transform) const {
		transform = Mat4f(1);
		return glm::abs((double)(left->photonMap()->photonCount()) - (double)(right->photonMap()->photonCount())); 
	}
private:
	size_t m_maxPhotons;
};