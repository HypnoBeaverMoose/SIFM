#pragma once
#include"ITilesFilter.hpp"
#include "PhotonMap.hpp"
#include<vector>
#include<map>

#include "timer.h"
//typedef std::map<Tile*, float> DistanceMap;
//typedef std::map<Tile*, float>::iterator DistanceIter;
//typedef std::map<Tile*, float>::const_iterator constDistanceIter;

class NonLocalFilter : public ITilesFilter {

private:

struct Weight {
	size_t index;
	double weight;
	Mat4f transform;
};
public:
	NonLocalFilter(IMetric* const metric = 0, IDescriptor* const descriptor = 0);

	void createIndex(IFace** tiles, size_t size);
	
	void filterTiles(IFace** tiles, size_t size);

	double filterParam() { return m_filterAmount; }
	void setfilterParam(double filter) { m_filterAmount = filter; }

private: 
	void filterPhotonMap(std::vector<Photon>& dst,  Tile* const src, float weight, const Mat4f& transform);

private:
	std::vector<std::pair<size_t, std::vector<Weight>>> m_weights;
	std::map<size_t, std::vector<Photon>> m_photonCache;
	double m_filterAmount;

	timer m_timer;
	size_t m_distanceStage;
	size_t m_filterStage;
	size_t m_transferStage;
	IMetric* m_metric;
	IDescriptor* m_descriptor;

};