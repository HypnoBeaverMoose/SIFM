#pragma once
#include<nanoflann.hpp>
#include "Photon.hpp"

#include<map>
#include<mutex>

class PhotonMap {

public:
	typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, PhotonMap>, PhotonMap, 3>  KdTree;

public:
	PhotonMap();

	PhotonMap(const PhotonMap& other);


	void reserve_data(size_t photonCount) { }

	void rebuildKdTree() { if(m_photons.size() > 0) m_tree.buildIndex(); }
	
	bool needsRebuild() { return m_tree.size() != m_photons.size(); }
	
	void drawPhotons();

	void clearPhotons();
	
	size_t photonCount() { return m_photons.size(); }
	size_t photonCount() const { return m_photons.size(); }

	Photon& photon(size_t index) { return m_photons[index]; }
	const Photon& photon(size_t index) const { return m_photons.at(index); }

	Color3f radiusSearch(const Vec3f& point, float radius, const Vec3f& worldNormal, size_t& nnCount, const Vec3f& color,  bool normalize);

	const Mat4f& localTranform() {return m_localTransform; }
	void setLocalTransform(const Mat4f& trans) { m_localTransform = trans; } 

	void set_preview(bool on) { m_showPreview = on; }

	void insertPhoton(const Photon& p);

	Color3f photonDensity(const Vec3f& point, const Vec3f& worldNormal, uint nnCount, double& radius, const Vec3f& color, bool normalize, Vec3f& variance = Vec3f(0)) const;

	Color3f variance(const Vec3f& point, const Vec3f& worldNormal, uint nnCount, double& radius, bool normalize) const;

	void resetThreadData(size_t threadCount);

	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }

	inline size_t kdtree_get_point_count() const;

	inline float kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const;

	inline float kdtree_get_pt(const size_t idx, int dim) const;

public:
	static float globalExposure;
	static uint	 globalPhotonCount;

private:
	Mat4f				m_localTransform;
	std::vector<Photon> m_photons;
	///std::map<size_t, Photon> m_photons;
	KdTree				m_tree;
	mutable std::mutex	m_accessMutex;
	bool				m_showPreview;
	//mutable std::map<size_t, Vec3f>	m_currentNormal;
	//mutable	std::map<size_t, Vec3f>	m_currentPoint;
	//mutable bool m_updatingCurrent;
};