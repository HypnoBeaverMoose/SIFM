#include "Definitions.hpp"
#include "Math.hpp"
#include "Tile.hpp"
#include"DirHistDescrtiptor.hpp"


DirHistDescrtiptor::DirHistDescrtiptor(size_t phi_res, size_t theta_res, size_t tile_res) 
	: m_phiResolution(phi_res), m_thetaResolution(theta_res), m_tileTesolution(tile_res){

}

void DirHistDescrtiptor::rebuildDescriptor(Tile* tile) {


	double phi_bin = glm::pi<double>() / (m_phiResolution * 2);
	double theta_bin = 2 * glm::pi<double>() / (m_thetaResolution);

	const Photon* photons = tile->photons();
	Mat4f inv_transform = glm::inverse(tile->transform());
	for(size_t p = 0; p < tile->photonCount(); p++) {
		Vec3f dir = Vec3f(inv_transform *  Vec4f(photons[p].direction(), 0));
		//get spherical coordinates of the incoming direction 
		//add add +1 for each bin in phi and theta that the coordinates fall in
	}
}

bool DirHistDescrtiptor::empty() const {
	
	return false;
}

void DirHistDescrtiptor::value(float** transformed_values) const{
}

void DirHistDescrtiptor::value(const Mat3f& transform, float** transformed_values) const {

}