#include "Definitions.hpp" 
#include "Math.hpp"
#include "Tile.hpp"
#include "NonLocalFilter.hpp"
#include "IMetric.hpp"


NonLocalFilter::NonLocalFilter(IMetric* const metric, IDescriptor* const descriptor) 
	: m_filterAmount(1.0), m_timer(false), m_metric(metric), m_descriptor(descriptor) {

	m_timer.register_stage(m_distanceStage);
	m_timer.register_stage(m_filterStage);
	m_timer.register_stage(m_transferStage);
	
};

void NonLocalFilter::createIndex(IFace** tiles, size_t size) {

	if(m_metric == 0) return;
	m_weights.clear();
	m_photonCache.clear();
	double squareFilter = m_filterAmount * m_filterAmount;
	double avg_maxWeight = 0;
	m_timer.reset();
	m_timer.start(m_distanceStage);
	for(size_t i = 0; i < size; i++) {
		std::vector<Weight> weights;
		double maxWeight = 0;
		Tile* left = static_cast<Tile*>(tiles[i]);
		for(size_t j = 0; j < size; j++) {
			if( i == j) continue;
			
			Tile* right = static_cast<Tile*>(tiles[j]);
			Weight w; w.index = j; w.transform =  Mat4f(1);
			double distance = m_metric->distance(left, right, w.transform) / squareFilter;
			w.weight = glm::exp( -distance );
			maxWeight+= w.weight;
			weights.push_back(w);
		}

		double max = 0;
		for(size_t k = 0; k < size - 1; k++) {
			weights[k].weight /= maxWeight;
			//if(weights[k].weight < 0.00001) 
			//	weights[k].weight = 0;
			max = glm::max(max, weights[k].weight);
		}
		avg_maxWeight+=max;
		m_weights.push_back(std::pair<size_t, std::vector<Weight>>(i,weights));
	}
	avg_maxWeight /= size ;
	debug("\n Average max weight: %f",avg_maxWeight);
	m_timer.stop();
}
void NonLocalFilter::filterTiles(IFace** tiles, size_t size) {

	m_timer.start(m_filterStage);
	for(size_t i = 0; i < m_weights.size(); i++) {
		Tile* left = static_cast<Tile*>(tiles[m_weights[i].first]);
		std::vector<Photon>& cache = m_photonCache[m_weights[i].first];
		for(size_t j = 0 ; j < m_weights[i].second.size(); j++) {
			Tile* right = static_cast<Tile*>(tiles[m_weights[i].second[j].index]);
			if(m_weights[i].second[j].weight > 0)
				filterPhotonMap(cache, right, m_weights[i].second[j].weight, m_weights[i].second[j].transform);
		}
	}
	m_timer.start(m_transferStage);
	for(std::map<size_t,std::vector<Photon>>::iterator i = m_photonCache.begin(); i !=m_photonCache.end(); i++) {	
		Tile* t = static_cast<Tile*>(tiles[i->first]);
		t->clearPhotons();
		for(size_t p = 0; p < i->second.size(); p++) {
			t->insertPhoton(i->second[p], false);
		}
	}
	m_timer.stop();

	debug("--------------------------------------------------------\n" 
		"FILTER TIME: %f, out of which: \n\t %f - DISTANCE CALCULATION"
		 "\n\t %f - FILTERING\n\t %f  - PHOTON TRASFER",
		 m_timer.elapsed(), m_timer.elapsed(m_distanceStage), m_timer.elapsed(m_filterStage), m_timer.elapsed(m_transferStage));

}
void NonLocalFilter::filterPhotonMap(std::vector<Photon>& dst,  Tile* const src, float weight, const Mat4f& transform) {

	const Photon* photons = src->photons();
	for(size_t p = 0; p <src->photonCount(); p++) {
		if(glm::linearRand(0.0f, 1.0f) < weight) {
			Photon ph(photons[p]);	
			ph.setPostition(Vec3f(transform * Vec4f(ph.postition(),1)));
			dst.push_back(ph);
		}
	}
}
