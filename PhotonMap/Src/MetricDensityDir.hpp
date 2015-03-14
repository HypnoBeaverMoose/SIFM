#pragma once

#include "MetricPhotonDensity.hpp" 

class MetricDensityDir : public MetricPhotonDensity {

public:
	MetricDensityDir(double maxDensity) : m_maxDensity(maxDensity) {};
	 double distance(Tile* const left, Tile* const right, Mat4f& transform) const;

protected:
	Vec3f averageNormal(PhotonMap* const map) const;
private:
	double m_maxDensity;
};