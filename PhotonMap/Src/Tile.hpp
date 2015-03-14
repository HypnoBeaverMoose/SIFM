#pragma once
#include"Mesh.hpp"
#include"Photon.hpp"
#include"PhotonMap.hpp"

#include<tuple>
#include<mutex>

class Tile: public IFace {


public:
	static float mirrorRarius;
	static size_t globalTilesCount;
	typedef std::tuple<Tile*,double,Mat4f> SimilarTile;

public:
	Tile(Mesh* mesh = 0);

	const Vec3f& position(uint index) { return m_objectPoints[index]; }
	const Vec3f& position(uint index) const { return m_objectPoints[index]; }

	const Vec3f& normal() { return m_normal; }
	const Vec3f& normal() const { return m_normal; }

	const Vec3f& worldPosition(uint index) { return m_worldPoints[index]; }
	const Vec3f& worldPosition(uint index) const { return m_worldPoints[index]; }

	const Vec3f& worldNormal() { return m_worldNormal; }
	const Vec3f& worldNormal() const { return m_worldNormal; }

	const Mat4f& transform() { return m_transform; }
	const Mat4f& transform() const  { return m_transform; }

	const Mat4f& inv_transform() { return m_transformInv; }
	const Mat4f& inv_transform() const  { return m_transformInv; }

	const AABB& aabb() { return m_aabb; }
	const AABB& aabb() const { return m_aabb; }

	PhotonMap*	photonMap() {return m_photonMap.get(); }
	PhotonMap*	photonMap() const {return m_photonMap.get(); }
	
	void setPhotonMap(const std::vector<Photon>& map);

	Color3f photonDensity(const Vec3f& point, uint nnCount) const ;

	//void insertPhoton(const Photon& p);


	bool intersect(const Vec3f& origin, const Vec3f& dir, float& t) const;

	bool intersectWorld(const Vec3f& origin, const Vec3f& dir, float& t) const;

	uint size() { return 4; }
	uint size() const { return 4; }

	void insertPhoton(const Photon& p, bool transform = true) ;
	
	bool contains(const Vec3f& position) const;

	void clearPhotons() { m_photons.clear(); m_photons.shrink_to_fit(); } 
	size_t photonCount() { return m_photons.size(); }
	size_t photonCount() const { return m_photons.size(); }
	
	const Photon* photons() const {return m_photons.data(); } 

	Mesh* const mesh() { return m_mesh; }

	Mesh* const mesh() const { return m_mesh; }

	void addSimilarTile(Tile* tile, double weight, const Mat4f& transform);

	const std::tuple<Tile*, double, Mat4f>& similarTile(size_t index) { return m_similarTiles[index]; }
	
	size_t similarTilesCount() { return m_similarTiles.size(); }

	void clear_similar_tiles() {m_similarTiles.clear(); }

	const Vec3f& color() { return m_mesh->color(); }
	const float albedo() { return m_mesh->albedo();}


	const Vec3f& sampleTexture(const Vec3f& pos);

	virtual ~Tile() {};

private:
	static Vec2f s_Texcoords[6];

private:
	Vec3f worldToLocal(const Vec3f& point, bool dir = false) const;
	Vec3f localtoWorld(const Vec3f& point, bool dir = false) const;

private:
	Mesh*						m_mesh;

	std::auto_ptr<PhotonMap>	m_photonMap;	
	std::vector<Vec3f>			m_objectPoints;
	std::vector<Vec3f>			m_worldPoints;

	Mat4f						m_transform;
	Mat4f						m_transformInv;
	Vec3f						m_worldNormal;
	Vec3f						m_normal;
	Vec3f						m_color;
	Vec3f						m_min;
	Vec3f						m_max;
	float						m_albedo;

	AABB						m_aabb;
	std::vector<Photon>			m_photons;
	std::mutex					m_accessMutex;
	//std::vector<std::pair<Tile*, double>>	m_similarTiles;
	std::vector<std::tuple<Tile*, double, Mat4f>> m_similarTiles;
	
};