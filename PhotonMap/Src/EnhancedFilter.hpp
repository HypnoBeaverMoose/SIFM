#pragma once
#include"timer.h"
#include<nanoflann.hpp>
class IFace;
class DensityDescriptor;
#include<vector>

class EnhancedFilter {	

	typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Adaptor<float,EnhancedFilter,float>, EnhancedFilter>  KdTree;
public:
	EnhancedFilter(PhotonMap* const map);

	float createIndex(IFace** tiles, size_t size, size_t density_photons, 
		int max_dimenstions = -1, float pca_fraction = 1.0f, size_t resolution = 0, RESOLUTION_STRATEGY strategy = RES_STRATEGY_MAX);

	std::pair<float, float> filterTiles(IFace** tiles, size_t size, bool useKnn, float search_param, float sigma, bool filter_textures = false);

	size_t minimum_neighbours() { return m_mininum_neighbours; }

	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }

	inline size_t kdtree_get_point_count() const;

	inline float kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const;

	inline float kdtree_get_pt(const size_t idx, int dim) const;

	void draw_descriptors();
	
	size_t descriptor_resolution();

	void filter_texture(size_t dest_index,	size_t src_index, float weight, const Mat4f& transform);

	~EnhancedFilter();

public:
	struct Descriptor {
		Descriptor(size_t _index,  const Mat4f& t) :
			index(_index), transform(t) {};

		size_t index;
		std::vector<float> value;
		std::vector<float> variance;
		Mat4f transform;
	};
		

	typedef std::pair<size_t, float> instance;

private:
	void filterPhotonMap(std::vector<Photon>& dst,  Tile* const src, double weight, const Mat4f& transform);
	
	void save_descriptors(size_t index, const std::vector<std::pair<size_t, float>>& unique_distances);

	size_t determine_resolution(IFace** tiles, size_t size, RESOLUTION_STRATEGY strategy);

private:

	size_t m_mininum_neighbours;
	std::vector<Descriptor> m_instances;
	std::vector<Descriptor*> m_descriptors;
	std::auto_ptr<DensityDescriptor> m_prototype;

	PhotonMap* m_map;
	std::auto_ptr<KdTree> m_tree;

	std::vector<float> m_mean;
	std::vector<float> m_eigenvalues;
	std::vector<Vec3f> m_points;

	timer m_timer;
	size_t m_descriptorGenStage;
	size_t m_rotateDescriptorStage;
	size_t m_buildIndexStage;
	size_t m_searchStage;
	size_t m_weigthCalculationStage;
	size_t m_filterStage;
	size_t m_transferDataStage;
	

	std::vector<std::pair<size_t, float>> m_distances;

public:
	static std::vector<std::vector<float>> textures;
	static std::vector<std::vector<float>> filtered_textures;
};