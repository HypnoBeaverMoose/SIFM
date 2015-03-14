#include "Definitions.hpp"
#include"Objects.hpp"
#include "Math.hpp"
#include "Scene.hpp"
#include"AleaLight.hpp"
#include"EnhancedFilter.hpp"
#include "NonLocalFilter.hpp"
#include "MetricPhotonCount.hpp"
#include "MetricPhotonDensity.hpp"
#include "MetricDensityDir.hpp"
#include "MetricTexture.hpp"
#include "MetricDescriptor.hpp" 

#include"DensityDescriptor.hpp"

#include<assimp\Importer.hpp>
#include<assimp\scene.h>
#include<sstream>
#include<queue>

//static size_t max_resolution = 2;

Scene::Scene():m_octree(30,5), 
	m_rootNode(new SceneNode()), m_useTiles(false), m_tilesFilter(0) {
	m_tilesFilter.reset(new EnhancedFilter(&m_photonMap));
}

 SceneNode* const Scene::getRoot() {
	 
	return m_rootNode.get();
	
}

void Scene::splitTile(const Tile& tile, std::vector<Tile*>& tiles, size_t iter) {

	static Vec3f offsets[4] = { Vec3f(-0.5f, -0.5f,0), Vec3f(0.5f, -0.5f,0),
								Vec3f(0.5f, 0.5f,0), Vec3f(-0.5f, 0.5f,0)};

	for(int i = 0; i < 4; i++) {		
		SceneNode* newNode = new SceneNode();
		newNode->setTransform( tile.mesh()->tranformation() * glm::translate(offsets[i]) * 
																	glm::scale(Vec3f(0.5f,0.5f,1.0f)));
		Mesh* newMesh = new Mesh(*(tile.mesh()));
		newNode->setEntity(newMesh);
		newMesh->setSceneNode(newNode);
		tile.mesh()->scene_node()->addChild(newNode);
		newMesh->updateFaces();
		
		Tile* new_tile = new Tile(newMesh);
		newMesh->upload();
		const Photon* photons = tile.photons();
		for(size_t i = 0; i < tile.photonCount(); i++) {
			if(new_tile->contains((Vec3f)(tile.transform() * Vec4f(photons[i].postition(),1)))) {
				Photon ph(photons[i]);
				ph.setPostition((Vec3f)(tile.transform() * Vec4f(ph.postition(),1)));
				ph.setDirection((Vec3f)(tile.transform() * Vec4f(ph.direction(),0)));
				new_tile->insertPhoton(ph);
			}	
		}
		tiles.push_back(new_tile);
	}	
 }

void Scene::splitTile(const Mesh& mesh, SceneNode* node, std::vector<Mesh*>& meshes, size_t iter) {

	static Vec3f offsets[4] = { Vec3f(-0.5f, -0.5f,0), Vec3f(0.5f, -0.5f,0),
								Vec3f(0.5f, 0.5f,0), Vec3f(-0.5f, 0.5f,0)};

	size_t iterations = iter - 1;
	for(int i = 0; i < 4; i++) {
		Mesh* newMesh = new Mesh(mesh);
		SceneNode* newNode = new SceneNode();
		newNode->setTransform( node->transform() * glm::translate(offsets[i]) * 
																	glm::scale(Vec3f(0.5f,0.5f,1.0f)));
		newNode->setEntity(newMesh);
		newMesh->setSceneNode(newNode);
		node->addChild(newNode);
		newMesh->updateFaces();
		if(iterations == 0)
			meshes.push_back(newMesh);
		else
			splitTile(*newMesh, newNode, meshes, iterations);
	}
}

void Scene::LoadFromFile(std::string filename,  bool requireTiles, size_t splitIters) {
	
	m_useTiles = requireTiles;
	Assimp::Importer importer;
	const aiScene* scene =  importer.ReadFile(filename,0);	

	//// Load all lights
	for(int i = 0;  i < (int)scene->mNumLights; i++) {
		Color3f color(1.0f, 0.85f, 0.43f);
		m_lights.push_back(new AreaLight(1.0f,Vec3f(1), 1.0f,scene->mLights[i]->mName.C_Str()));
	}
	//// Load cameras
	for(int i = 0;  i <(int) scene->mNumCameras; i++) {
		
		aiCamera* cam = scene->mCameras[i];
		m_cameras.push_back(new Camera(	cam->mHorizontalFOV,	 cam->mAspect, 
										cam->mClipPlaneNear, cam->mClipPlaneFar, 
										cam->mName.C_Str()));
	}
	if(m_cameras.size() > 0)
		m_mainCamera = m_cameras[0];
	else
		m_mainCamera = new Camera(45,1.3f,1.0f,100.0f,"cam");

	std::queue< std::pair<SceneNode*, aiNode*>> nodes;
	nodes.push(std::pair<SceneNode*, aiNode*>(m_rootNode.get(), scene->mRootNode));

	do {

		SceneNode		*myParent = nodes.front().first;
		aiNode*			parent = nodes.front().second;				
		aiMatrix4x4 mat = parent->mTransformation;		
		mat.Transpose();
		myParent->setTransform( myParent->transform() * Mat4f(	mat.a1, mat.a2, mat.a3, mat.a4,
																	mat.b1, mat.b2, mat.b3, mat.b4, 
																	mat.c1, mat.c2, mat.c3, mat.c4, 
																	mat.d1, mat.d2, mat.d3, mat.d4 ));
		bool foundObject = false;
		if(parent->mNumMeshes > 0) {

			//we allow only one mesh per node, and in general only one entity
			aiMesh* ai_mesh = scene->mMeshes[parent->mMeshes[0]];
			int mat_index = ai_mesh->mMaterialIndex;
			aiColor4D diffuse;
			aiGetMaterialColor(scene->mMaterials[mat_index],AI_MATKEY_COLOR_DIFFUSE,&diffuse);
			Mesh* mesh = new Mesh(ai_mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE ? TRIANGLES : QUADS, ai_mesh->mName.C_Str());
			mesh->setSceneNode(myParent);
			mesh->set_color(Vec3f(diffuse.r, diffuse.g, diffuse.b));
			mesh->set_albedo(1.0f);
			for(int j = 0; j < (int)ai_mesh->mNumVertices; j++) {
				mesh->addVertex(Vec3f(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z), 
								Vec3f(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z));
			}
			for(int j = 0; j < (int)ai_mesh->mNumFaces; j++) {
				
				if(ai_mesh->mFaces[j].mNumIndices > 4) {
					OutputDebugString("WARNING: FACE WITH MORE THAT 4 VERTS DETECTED, SKIPPING!");
						continue;
				}
				mesh->addFace(ai_mesh->mFaces[j].mIndices, ai_mesh->mFaces[j].mNumIndices);
			}

			//std::vector<Mesh*> meshes;
			//bool is_Tile = isTile(ai_mesh);		
			//m_meshes.push_back(mesh);	
			//if(requireTiles && (splitIters > 0) &&  is_Tile ) {	
			//	splitTile(*mesh, myParent,meshes, splitIters);	
			//	delete mesh;
			//}
			//else{
			//}
			IFace* tile = new Tile(mesh);
			m_tiles.push_back(tile);

			myParent->setEntity(mesh);
			mesh->upload();
			m_meshes.push_back(mesh);

			//for(size_t i = 0; i < meshes.size(); i++) {	
			//	if((requireTiles && is_Tile)) {	
			//		IFace* tile = new Tile(meshes[i]);
			//		meshes[i]->upload();
			//		m_tiles.push_back(tile);
			//		m_meshes.push_back(meshes[i]);
			//	}
			//}

			foundObject = true;
		}
	
		
		if(!foundObject) {
			for(int i = 0; i < (int)m_lights.size(); i++) {
				std::string name(parent->mName.C_Str());
				if(name == m_lights[i]->getName()) {
					myParent->setEntity(m_lights[i]);
					m_lights[i]->setSceneNode(myParent);
					foundObject = true;
					break;
				}
			}
		}

		if(!foundObject) {
			for(int i = 0; i < (int)m_cameras.size(); i++) {
				std::string name(parent->mName.C_Str());

				if(name == m_cameras[i]->getName()) {
					myParent->setEntity(m_cameras[i]);
					m_cameras[i]->setSceneNode(myParent);
					foundObject = true;
					break;
				}
			}		

		}
		for(int i = 0; i<(int) parent->mNumChildren; i++) {			
			SceneNode* node = new SceneNode();
			nodes.push(std::pair<SceneNode*, aiNode*>(node, parent->mChildren[i]));
			node->setTransform(myParent->transform());
			myParent->addChild(node);
		}

		nodes.pop();

	} while(!nodes.empty());

	if(!requireTiles) {
		for(uint i = 0; i < m_meshes.size(); i++) {
			for(uint j = 0; j < m_meshes[i]->faceCount(); j++)
				m_tiles.push_back((IFace*)(&(m_meshes[i]->face(j))));
		}
	}
	Tile::globalTilesCount = m_tiles.size();
	m_octree.loadData(m_tiles.data(),m_tiles.size());
	
	//if(m_photonMap.photonCount() > 0) {
	//	for(size_t i = 0; i < m_photonMap.photonCount(); i++) {
	//		m_photonMap.photon(i);
	//		for(size_t j = 0; j < m_tiles.size(); j++) {
	//			Tile* t = static_cast<Tile*>(m_tiles[j]);
	//			if(t->contains(m_photonMap.photon(i).postition()))
	//				t->insertPhoton(m_photonMap.photon(i));
	//		}
	//	}
	//}
	//photonMap()->clearPhotons();
	//for(size_t i = 0; i < m_tiles.size(); i++) {
	//	Tile* t = static_cast<Tile*>(m_tiles[i]);
	//		for(size_t p = 0 ; p < t->photonCount(); p++) {
	//			Photon ph(t->photons()[p]);
	//			ph.setPostition( Vec3f(t->transform() * Vec4f(ph.postition(),1)));
	//			ph.setDirection( Vec3f(t->transform() * Vec4f(ph.direction(),0)));
	//			photonMap()->insertPhoton(ph);
	//		}				
	//}
	//buildPhotonMap();
}

bool Scene::isTile(aiMesh* const  mesh) const {
	
	//we expect a mesh with 2 faces
	if(mesh->mNumFaces != 2) return false;
	aiVector3D n = mesh->mNormals[0];
	for(uint i = 1; i < mesh->mNumVertices; i++) {
		if(n != mesh->mNormals[i]) return false;
	}

	return true;
}

void Scene::setFilterParams(bool useTree, bool useKnn, float searchParam,  double param, int descriptor, int metric) {

}
void Scene::filterScene(size_t resolution) {

	if(!m_useTiles || m_tiles.size() == 0) return;

	//size_t req_photons = density_photons * resolution * resolution;
	//size_t tile_res = 0;
	//
	//while(true) {

	size_t req_photons =  density_photons * resolution * resolution;
		size_t tile_res = 0;
		while(tile_res < 5) {
			float avg  = photonMap()->photonCount() /(float)tiles_count();
			if(avg > req_photons) {
				subdivide();
				tile_res++;
			}
			else 
				break;
		}

//	debug("Tile Resolution: %d\n", tile_res);
	m_tilesFilter->createIndex(m_tiles.data(), m_tiles.size(),density_photons,-1, 1.0f,resolution);
	m_tilesFilter->filterTiles(m_tiles.data(),m_tiles.size(),true, 20,0.0001f,false);
	//photonMap()->clearPhotons();

	//for(size_t i = 0; i < m_tiles.size(); i++) {
	//	Tile* t = static_cast<Tile*>(m_tiles[i]);
	//		for(size_t p = 0 ; p < t->photonCount(); p++) {
	//			Photon ph(t->photons()[p]);
	//			ph.setPostition( Vec3f(t->transform() * Vec4f(ph.postition(),1)));
	//			ph.setDirection( Vec3f(t->transform() * Vec4f(ph.direction(),0)));
	//			photonMap()->insertPhoton(ph);
	//		}				
	//}
	//buildPhotonMap();
}

void Scene::set_textures() {
	
	for(size_t i = 0 ; i < m_tiles.size(); i++) {

		Tile* tile = static_cast<Tile*>(m_tiles[i]);
		std::vector<float> tex(EnhancedFilter::textures[i].begin(), EnhancedFilter::textures[i].end());
		std::for_each(tex.begin(), tex.end(), [] (float& d) { d =  (d* PhotonMap::globalExposure); });
		size_t res = m_tilesFilter->descriptor_resolution();
		tile->mesh()->setTexture(tex.data(),Vec2i(res,res),3);
	}

}
float Scene::traceRay(const Vec3f& origin, const Vec3f& dir, IFace** out_face) {
	
	return m_octree.traceRay(origin,dir,out_face);
}
	
void Scene::buildPhotonMap() {
	
	m_photonMap.rebuildKdTree();
}

void Scene::clearPhotons() {

	if(m_useTiles) {
		for(size_t i = 0 ; i < m_tiles.size(); i++) {
			Tile* tile = dynamic_cast<Tile*>(m_tiles[i]);
			if(tile)
				tile->clearPhotons();
		}
	}
	m_photonMap.clearPhotons();
}



void Scene::clean_scene()  {
	
	for(size_t i = 0; i < m_tiles.size(); i++) {
		delete m_tiles[i];
	}

	m_lights.clear();
	m_cameras.clear();
	m_meshes.clear();

	m_tiles.clear();
	m_octree.clean();
	m_rootNode.reset(new SceneNode());

//	debug("\n Scene node istances left: %i", SceneNode::instances);
//	debug("\n Face istances left: %i", Face::instances);

}


void Scene::subdivide() {
	
	std::vector<Tile*> all_tiles;
	m_meshes.clear();
	for(size_t i = 0; i < m_tiles.size(); i++) {
		std::vector<Tile*> tiles;
		Tile* tile = static_cast<Tile*>(m_tiles[i]);
		splitTile(*tile,tiles,0);
		
		for(size_t j = 0; j < tiles.size(); j++) {
			all_tiles.push_back(tiles[j]);
			m_meshes.push_back(tiles[j]->mesh());
		}
		tile->mesh()->scene_node()->setEntity(0);
		delete m_tiles[i];
	}
	m_tiles.clear();
	m_tiles.assign(all_tiles.begin(), all_tiles.end());
	m_octree.clean();
	m_octree.loadData(m_tiles.data(),m_tiles.size());
}

void Scene::reset_filter() { m_tilesFilter.reset(new EnhancedFilter(&m_photonMap)); }
