#include"Definitions.hpp"
#include "Math.hpp"
#include "Tile.hpp"
#include"MetricDensityDir.hpp"


double MetricDensityDir::distance(Tile* const left, Tile* const right, Mat4f& transform) const {

	Vec3f leftDensity = calculateDensity(left, left->worldNormal(), 1.0f);	
	Vec3f rightDensity = calculateDensity(right, right->worldNormal(), 1.0f);

	Vec3f leftNormal = averageNormal(left->photonMap());
	Vec3f rightNormal = averageNormal(right->photonMap());

	float min_angle = 0, min_dist = 1.0f;
	for(float angle = 0; angle < 2 * glm::pi<float>(); angle+= glm::half_pi<float>()) {
		Vec3f normal = Vec3f(glm::rotate(angle, Vec3f(0,0,1)) * Vec4f(rightNormal,0));
		float dist = 1.0f - glm::dot(normal, leftNormal);
		if(dist < min_dist) {
			min_dist = dist;
			min_angle = angle;
		}
	}

	transform = glm::rotate(min_angle, Vec3f(0,0,1));
	return glm::distance(leftDensity,rightDensity);
}

Vec3f MetricDensityDir::averageNormal(PhotonMap* const map)  const {

	Vec4f norm(0);
	for(size_t i = 0; i < map->photonCount(); i++) {
		norm+=  map->localTranform() * Vec4f(map->photon(i).direction(),0);
	}

	glm::normalize(norm/=map->photonCount());	
	return Vec3f(norm);
}