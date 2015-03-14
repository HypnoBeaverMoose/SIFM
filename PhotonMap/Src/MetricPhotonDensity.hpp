#pragma once
#include "IMetric.hpp"


class MetricPhotonDensity: public IMetric  {	
public:
	double distance(Tile* const left, Tile* const right, Mat4f& transform) const;

protected:
	Vec3f calculateDensity(Tile* const tile, const Vec3f& normal, float area) const;

};