#include"Definitions.hpp"
#include "Math.hpp"
#include "Tile.hpp"

#include "DensityDescriptor.hpp"

#include<tuple>

float Tile::mirrorRarius = 0.2f;
size_t Tile::globalTilesCount = 0;
Vec2f Tile::s_Texcoords[6] = { Vec2f(0,0), Vec2f(1,0), Vec2f(0,1), Vec2f(1,0), Vec2f(1,1), Vec2f(0,1) };

Tile::Tile(Mesh* mesh):m_mesh(mesh), m_transform(1), IFace(TILE), m_photonMap(0) {

	if(mesh) {
		for(uint i = 0; i < mesh->face(0).size(); i++) {
			m_objectPoints.push_back(mesh->face(0).position(i));
			m_worldPoints.push_back(mesh->face(0).worldPosition(i));
		}
		for(uint i = 0; i < mesh->face(1).size(); i++) {

			if(std::find(m_objectPoints.begin(), m_objectPoints.end(), mesh->face(1).position(i)) == m_objectPoints.end()) {
				m_objectPoints.insert(m_objectPoints.end() - 1, mesh->face(1).position(i));
				m_worldPoints.insert(m_worldPoints.end() - 1, mesh->face(1).worldPosition(i));
				break;
			}
		}
		for(int  i = 0 ; i  < 6; i++) {
			m_mesh->insertTexCoord(s_Texcoords[i],i);
		}
		m_normal = mesh->face(0).normal(0);
		m_worldNormal = glm::normalize(mesh->face(0).worldNormal(0));
		m_transform = m_mesh->tranformation();
		m_transformInv = glm::inverse(m_transform);
	}
	m_min = m_objectPoints[0];
	m_max = m_objectPoints[0];
	for(auto point = m_objectPoints.begin(); point!=m_objectPoints.end(); ++point) {
		m_min.x = glm::min(m_min.x, point->x);
		m_min.y = glm::min(m_min.y, point->y);
		m_min.z = glm::min(m_min.z, point->z);
		m_max.x = glm::max(m_max.x, point->x);
		m_max.y = glm::max(m_max.y, point->y);
		m_max.z = glm::max(m_max.z, point->z);
	}
}

void Tile::setPhotonMap(const std::vector<Photon>& map) {
	m_photons.clear();
	std::lock_guard<std::mutex> lock(m_accessMutex);
	m_photons.insert(m_photons.end(), map.begin(), map.end());
}
void Tile::insertPhoton(const Photon& p, bool transform) {

	if(transform) {
		Photon p1(p);
		p1.setPostition(worldToLocal(p.postition()));
		p1.setDirection(worldToLocal(p.direction(),true));
		std::lock_guard<std::mutex> lock(m_accessMutex);
		m_photons.push_back(p1);
	}
	else {
		std::lock_guard<std::mutex> lock(m_accessMutex);
		m_photons.push_back(p);
	}
	return;
}


Color3f Tile::photonDensity(const Vec3f& point, uint nnCount) const {
		
	Vec3f pos = worldToLocal(point);
	double radius;
	return m_photonMap->photonDensity(pos, m_worldNormal, nnCount, radius, Vec3f(1), false);
}

Vec3f Tile::worldToLocal(const Vec3f& point, bool dir) const {

	return Vec3f(m_transformInv * Vec4f(point, dir ? 0 : 1));
}

Vec3f Tile::localtoWorld(const Vec3f& point, bool dir) const {

	return Vec3f(m_transform * Vec4f(point, dir ? 0 : 1));
}

bool Tile::intersect(const Vec3f& origin, const Vec3f& dir, float& t) const {
	

	const Vec3f t_origin = Vec3f(m_transformInv * Vec4f(origin,1));
	const Vec3f t_dir = Vec3f(m_transformInv * Vec4f(dir,0));
	
	if(m_mesh) {
		if(m_mesh->face(0).intersect(t_origin, t_dir, t))
			return true;
		else
			return m_mesh->face(1).intersect(t_origin, t_dir, t);
		
	}
	return false;
}
bool Tile::intersectWorld(const Vec3f& origin, const Vec3f& dir, float& t) const {
	
	if(m_mesh) {
		if(m_mesh->face(0).intersectWorld(origin, dir, t))
			return true;
		else
			return m_mesh->face(1).intersectWorld(origin, dir, t);
	}
	return false;
}

void Tile::addSimilarTile(Tile* tile, double weight, const Mat4f& transform)
{ 	
	m_similarTiles.push_back(SimilarTile(tile,weight,transform));	
}


const Vec3f& Tile::sampleTexture(const Vec3f& pos) {

	Vec3f local = worldToLocal(pos);
	//if(local.z != 0) 
	//	return Vec3f(0);
	return m_mesh->sample_texture((Vec2f(local.x, local.y) + Vec2f(1.0f)) * 0.5f);
}

bool Tile::contains(const Vec3f& position) const {

	Vec3f pos = worldToLocal(position);
	return 	pos.x > m_min.x && pos.y > m_min.y && pos.x <=m_max.x && pos.y <= m_max.y;
}
