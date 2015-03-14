#pragma once
#include"IDescriptor.hpp"
#include<vector>

class Tile;
class PhotonMap;


class DirHistDescrtiptor : public IDescriptor {

public:

	DirHistDescrtiptor(size_t phi_res, size_t theta_res, size_t tile_res);

	void rebuildDescriptor(Tile* tile);

	virtual size_t resolution() const { return 0;} 

	virtual size_t value_size() const { return 0;} 

	bool empty() const;

	void value(float** transformed_values) const;

	void value(const Mat3f& transform, float** transformed_values) const;

private:
	size_t				m_phiResolution;
	size_t				m_thetaResolution;
	size_t				m_tileTesolution;
	std::vector<double> m_values;
};