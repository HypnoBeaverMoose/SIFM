#include"Definitions.hpp"
#include"Math.hpp"
#include"MetricTexture.hpp"
#include"Photon.hpp"
#include"Tile.hpp"

std::map<Tile*, std::vector<Vec3f>> MetricTexture::s_textureCache;


double MetricTexture::distance(Tile* const left, Tile* const right, Mat4f& transform) const {


	std::vector<Vec3f>& left_tex = s_textureCache[left];
	if(left_tex.size() == 0)
		left_tex = generateTexture(left, m_resolution);

	std::vector<Vec3f>& right_tex = s_textureCache[right];
	if(right_tex.size() == 0)
		right_tex = generateTexture(right, m_resolution);


	float dist_accum1 = 0.0f;
	for(size_t y = 0; y < m_resolution; y++)  {
		for(size_t x = 0; x < m_resolution; x++) {
			size_t index = y * m_resolution + x;
			dist_accum1 +=glm::distance(left_tex[index], right_tex[index]) * glm::distance(left_tex[index], right_tex[index]);
		}
	}
	//dist_accum1 = glm::sqrt(dist_accum1);
	//float dist_accum2 = 0.0f;
	//for(size_t y = 0; y < m_resolution; y++)  {
	//	for(size_t x = 0; x < m_resolution; x++) {
	//		size_t index = y * m_resolution + x;
	//		size_t r_index = y* m_resolution + (m_resolution - x - 1);
	//		dist_accum2 += glm::distance(left_tex[index], right_tex[r_index]) * glm::distance(left_tex[index], right_tex[r_index]);
	//	}
	//}
	//dist_accum2 = glm::sqrt(dist_accum2);
	//float dist_accum3 = 0.0f;
	//for(size_t y = 0; y < m_resolution; y++)  {
	//	for(size_t x = 0; x < m_resolution; x++) {
	//		size_t index = y * m_resolution + x;
	//		size_t r_index = (m_resolution - y - 1) * m_resolution + x;
	//		dist_accum3 +=glm::distance(left_tex[index], right_tex[r_index]) * glm::distance(left_tex[index], right_tex[r_index]);
	//	}
	//}
	//dist_accum3 = glm::sqrt(dist_accum3);
	//transform = dist_accum1 < dist_accum2 ? Mat4f(1) : dist_accum2 < dist_accum3 ? glm::scale(Vec3f(-1,1,1)) : glm::scale(Vec3f(1,-1,1));	
	//return glm::min( glm::min(dist_accum1, dist_accum2), dist_accum3);
	return dist_accum1;
}

std::vector<Vec3f> MetricTexture::generateTexture(Tile* tile, size_t resolution) const {

	size_t mult = 1;
	float pixelSize = 2.0f / resolution;
	float div = pixelSize;
	while(div < 1.0f) {  div*= 10; mult*= 10; }
	std::vector<Vec3f> tex(resolution * resolution);
	const Photon* photons = tile->photons();

	float exp = (PhotonMap::globalExposure /((float)PhotonMap::globalPhotonCount *  pixelSize * pixelSize)) * 10;
	for(size_t p = 0; p < tile->photonCount(); p++) {
		size_t x = (size_t)(((photons[p].postition().x + 1.0f) * mult) / div);
		size_t y = (size_t)(((photons[p].postition().y + 1.0f) * mult) / div);
		const Color3f& col = photons[p].color();
		tex[ y* resolution + x] += col;
		glm::abs(exp * glm::dot(glm::normalize(tile->worldNormal()), glm::normalize(photons[p].direction())));
	}
	return tex;
}
void MetricTexture::clearTextureCache() {
	s_textureCache.clear();
}