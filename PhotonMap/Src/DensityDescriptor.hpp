#pragma once
#include<vector>

class Tile;
class PhotonMap;


class DensityDescriptor{

public:

	DensityDescriptor(size_t resolution, PhotonMap* const map);

	void rebuildDescriptor(Tile* tile,  size_t density_photons);

	size_t resolution() const { return m_resolution; }

	size_t value_size() const  { return m_resolution *m_resolution* 3; }


	bool empty() const;

	//void value(float** transformed_values) const;
	
	//void value(const Mat3f& transform, float** transformed_values) const;

	void value(std::vector<float>& value) const;

	void variance(std::vector<float>& variance) const;

	void value(const Mat3f& transform, std::vector<float>& transformed_values) const;

	void variance(const Mat3f& transform, std::vector<float>& transformed_variance) const;

	static void rotate_descriptor(const Mat3f& transform, const float* input, const float* output, size_t resolution);

	~DensityDescriptor() {};
private:
	size_t				m_resolution;
	std::vector<float>	m_values;
	std::vector<float>	m_variance;
	PhotonMap*			m_photonMap;
};