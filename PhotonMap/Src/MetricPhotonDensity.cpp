#include "Definitions.hpp"
#include "Math.hpp"
#include "Tile.hpp"
#include "MetricPhotonDensity.hpp"


double MetricPhotonDensity::distance(Tile* const left, Tile* const right, Mat4f& transform) const {

	transform = Mat4f(1);
	Vec3f leftDensity = calculateDensity(left, left->worldNormal(), 1.0f);	
	Vec3f rightDensity = calculateDensity(right, right->worldNormal(), 1.0f);

	return glm::distance(leftDensity, rightDensity);
}


Vec3f MetricPhotonDensity::calculateDensity(Tile* const tile, const Vec3f& normal, float area) const {

	Vec3f density(0,0,0);
	const Photon* photons = tile->photons();
	for(size_t i = 0 ; i < tile->photonCount(); i++) {
		const Photon& p = photons[i];
		density  += p.color() * glm::abs(glm::dot(glm::normalize(p.direction()),glm::normalize(normal)));
	}
	return density;
}