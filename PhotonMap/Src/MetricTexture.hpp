#pragma once
#include "IMetric.hpp" 
#include<vector>
#include<map>

class MetricTexture : public IMetric  {

public:
	MetricTexture(size_t resolution): m_resolution(resolution) {};

	double distance(Tile* const left, Tile* const right, Mat4f& transform) const;

public:
	static void clearTextureCache();
	static std::map<Tile*, std::vector<Vec3f>> s_textureCache;
private:
	size_t m_resolution;
	std::vector<Vec3f> generateTexture(Tile* tile, size_t resolution) const;
	
};