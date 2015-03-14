
#include"PhotonMap.hpp"
#include<thread>
#include<list>
float	PhotonMap::globalExposure  = 0.0f;
uint	PhotonMap::globalPhotonCount = 0;

PhotonMap::PhotonMap(): m_tree(3, *this, nanoflann::KDTreeSingleIndexAdaptorParams(1)),m_localTransform(1), m_showPreview(true) {

} ;


PhotonMap::PhotonMap(const PhotonMap& other): m_tree(3, *this, nanoflann::KDTreeSingleIndexAdaptorParams(10)) {

	if(other.m_photons.size() > 0) {
		m_photons.assign(other.m_photons.begin(), other.m_photons.end());	
		m_tree.buildIndex();
	}
	m_localTransform = other.m_localTransform;
}	

void PhotonMap::drawPhotons() {

	//if(!m_showPreview) return;
	//glPointSize(1);
	//
	//glBegin(GL_POINTS);
	//	for(uint i = 0; i < m_photons.size(); i++) {
	//		//m_accessMutex.lock();
	//		Photon p = m_photons[i];
	//		//m_accessMutex.unlock();
	//		glColor3fv(glm::value_ptr(p.color()));
	//		glVertex4fv(glm::value_ptr(Vec4f(p.postition(),1)));
	//	}
	//glEnd();
	
}
Color3f PhotonMap::photonDensity(const Vec3f& point, const Vec3f& worldNormal, uint nnCount, double& radius, const Vec3f& col, bool normalize, Vec3f& variance) const {
	
	if(m_photons.size() == 0) return Color3f(0,0,0);

	float pnt[7];
	pnt[0] = point.x;
	pnt[1] = point.y;
	pnt[2] = point.z;
	pnt[3] = worldNormal.x;
	pnt[4] = worldNormal.y;
	pnt[5] = worldNormal.z;
	pnt[6] = (float)radius;
	std::vector<size_t> nearestIndices(nnCount,0);
	std::vector<float> distances(nnCount,0.0f);
	m_tree.knnSearch(pnt,nnCount,nearestIndices.data(),distances.data());

	const float smth = 0.1f;
	Color3f color(0,0,0);
	radius = distances.back();

	for(uint i = 0; i < nearestIndices.size(); i++) {
		const Photon& p = m_photons.at(nearestIndices[i]);
		color += p.color() * glm::abs(glm::dot(glm::normalize(p.direction()),glm::normalize(worldNormal)));
	}
	return color * col;
}

Color3f PhotonMap::variance(const Vec3f& point, const Vec3f& worldNormal, uint nnCount, double& radius, bool normalize) const {
	if(m_photons.size() == 0) return Color3f(0,0,0);

	float pnt[7];
	pnt[0] = point.x;
	pnt[1] = point.y;
	pnt[2] = point.z;
	pnt[3] = worldNormal.x;
	pnt[4] = worldNormal.y;
	pnt[5] = worldNormal.z;
	pnt[6] = (float)radius;
	
	std::vector<size_t> nearestIndices(nnCount,0);
	std::vector<float> distances(nnCount,0.0f);
	std::vector<Vec3f> color_samples;
	m_tree.knnSearch(pnt,nnCount,nearestIndices.data(),distances.data());
	
	Color3f color(0,0,0), variance(0);
	radius = distances.back();

	for(uint i = 0; i < nearestIndices.size(); i++) {
		const Photon& p = m_photons.at(nearestIndices[i]);
		color_samples.push_back(p.color() * glm::abs(glm::dot(glm::normalize(p.direction()),glm::normalize(worldNormal))));
		color += color_samples.back();
	}
	const Vec3f mean_color = color / (float)nearestIndices.size();
	for(uint i = 0; i < color_samples.size(); i++) {
		variance += (color_samples[i] - mean_color) * ( color_samples[i] - mean_color);
	}
	variance /= (color_samples.size() - 1);
	return variance;
}

Color3f PhotonMap::radiusSearch(const Vec3f& point, float radius, const Vec3f& worldNormal, size_t& nnCount, const Vec3f& col,  bool normalize) {

	if(m_tree.size() == 0) return Color3f(0,0,0);

	std::vector<std::pair<size_t,float>> distances;
	float pnt[6];
	pnt[0] = point.x;
	pnt[1] = point.y;
	pnt[2] = point.z;
	pnt[3] = worldNormal.x;
	pnt[4] = worldNormal.y;
	pnt[5] = worldNormal.z;
	m_tree.radiusSearch(pnt, radius, distances, nanoflann::SearchParams());
	nnCount = distances.size();
	Color3f color(0,0,0);
	for(size_t i = 0; i < distances.size(); i++) {
		const Photon& p = m_photons[distances[i].first];
		color  += p.color()* glm::abs(glm::dot(glm::normalize(p.direction()),glm::normalize(worldNormal)));		
	}	
	return color;
}

void PhotonMap::insertPhoton(const Photon& p) {
	

	static size_t index = 0;
	//std::list<Photon> l;
	//l
	try {
	std::lock_guard<std::mutex> lock(m_accessMutex);
	m_photons.push_back(p);
	}catch(const std::exception& e) {
		debug(e.what());
	}

}

void PhotonMap::clearPhotons() {
	
	std::lock_guard<std::mutex> lock(m_accessMutex);
	m_photons.clear();
	m_tree.freeIndex();
}

inline size_t PhotonMap::kdtree_get_point_count() const{ 
	return m_photons.size(); 
}

inline float PhotonMap::kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const {

	const float d0=p1[0]- m_photons.at(idx_p2).x();
	const float d1=p1[1]- m_photons.at(idx_p2).y();
	const float d2=p1[2]- m_photons.at(idx_p2).z();
	const float sq_len = d0*d0 + d1*d1 + d2*d2;
	//Vec3f  vec(d0,d1,d2);
	//float len = glm::dot(glm::normalize(-1.0f * vec), glm::normalize(Vec3f(p1[3], p1[4], p1[5]))) * glm::length(vec);
	//
	//if(len >  std::sqrt(p1[6]))
	//	return FLT_MAX;

	return sq_len;

}

inline float PhotonMap::kdtree_get_pt(const size_t idx, int dim) const {
	return m_photons.at(idx).postition()[dim];
}
