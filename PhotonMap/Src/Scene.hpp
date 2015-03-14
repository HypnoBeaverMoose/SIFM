#pragma once

#include "SceneNode.hpp"
#include "Camera.hpp" 
#include "Light.hpp"
#include "Mesh.hpp"
#include "Octree.hpp"
#include"Tile.hpp"
#include<vector>

class Enitity;
class SceneNode;
class Mesh;
class Camera;
class Light;
struct aiMesh;
class EnhancedFilter;
class Scene {

public:
	Scene();

	SceneNode* const getRoot();

	float traceRay(const Vec3f& origin, const Vec3f& dir, IFace** out_face);

	void LoadFromFile(std::string filename, bool requireTiles = false, size_t splitIters = 0);

	bool isTile(aiMesh* const  mesh) const;

	void setMainCamera(Camera* cam) {  m_mainCamera = cam; }

	Camera* const getMainCamera() { return m_mainCamera; }

	uint getLightCount() { return m_lights.size(); }

	Light* const getLight(int index) { return m_lights[index]; }

	uint getCameraCount() { return m_cameras.size(); }

	Camera* const getCamera(int index) { return m_cameras[index]; }

	uint geMeshCount() { return m_meshes.size(); }

	Mesh* const getMesh(int index) { return m_meshes[index]; }

	PhotonMap* const photonMap() { return &m_photonMap; }

	void drawOcttree(uint child) { m_octree.draw(child); }
	
	void setFilterParams(bool useTree, bool useKnn, float searchParam, double param, int Descriptor, int Metric);

	void filterScene(size_t resolution);
	
	void buildPhotonMap();

	void clearPhotons();

	IFace** const tiles() { return m_tiles.data(); }

	size_t tiles_count() { return m_tiles.size(); }
		
	EnhancedFilter* filter() const { return m_tilesFilter.get(); }

	void reset_filter();// { m_tilesFilter.reset(new EnhancedFilter(&m_photonMap)); }

	void set_textures();

	void clean_scene();

	void subdivide();

private:
	void splitTile(const Mesh& mesh, SceneNode* node, std::vector<Mesh*>& meshes, size_t iter);

	void splitTile(const Tile& tile, std::vector<Tile*>& tiles, size_t iter);

//	void transferPhotonMap(PhotonMap* const src, PhotonMap* const dst, const Mat4f& transform);

private:
	std::auto_ptr <SceneNode>	m_rootNode;
	std::vector<Light*>			m_lights;
	std::vector<Camera*>		m_cameras;
	std::vector<Mesh*>			m_meshes;
	std::vector<IFace*>			m_tiles;
	bool						m_useTiles;
	PhotonMap					m_photonMap;
	std::auto_ptr<EnhancedFilter> m_tilesFilter;
	Octree	m_octree;	
	Camera*	m_mainCamera;


};