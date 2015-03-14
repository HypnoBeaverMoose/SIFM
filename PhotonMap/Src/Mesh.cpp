#include "Definitions.hpp"
#include "Math.hpp"
#include "Mesh.hpp"
#include"PhotonMap.hpp"
#include<vector>


Vec3f Mesh::s_empty_pixel = Vec3f(0);
Mesh::Mesh(Primitive type, const char* name, SceneNode* parent, const Vec3f& color, float albedo) :
	Entity(name,  Entity::Mesh, parent), m_type(type), m_drawMode(0), m_color(color), m_albedo(albedo) {
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);	
}

uint Mesh::addVertex(const Vec3f& vertex) {
	
	m_vertices.push_back(vertex);
	m_normals.push_back(Vec3f(0));
	m_texcoords.push_back(Vec2f(0));
	for(int i = 0 ;i < 3; i++) {
		m_aabb.min[i] = std::min(m_aabb.min[i], vertex[i]);
		m_aabb.max[i] = std::min(m_aabb.max[i], vertex[i]);
	}
	return m_vertices.size() - 1;
}

uint Mesh::addVertex(const Vec3f& vertex, const Vec3f& normal) {
	m_vertices.push_back(vertex);
	m_normals.push_back(normal);
	m_texcoords.push_back(Vec2f(0));
	for(int i = 0 ;i < 3; i++) {
		m_aabb.min[i] = std::min(m_aabb.min[i], vertex[i]);
		m_aabb.max[i] = std::max(m_aabb.max[i], vertex[i]);
	}

	return m_vertices.size() - 1;

}

void Mesh::insertTexCoord(const Vec2f& coord, uint index) {
	
	if(index < m_texcoords.size())
		m_texcoords[index] = coord;
}

void Mesh::addFace(const unsigned int* indices, uint count) {

	m_indices.insert(m_indices.end(), indices, indices + count);
	std::vector<Vec3f> pos, norm;
	Vec3f min, max;
	for(uint i = 0; i < count; i++) {
		pos.push_back(m_vertices[indices[i]]);
		norm.push_back(m_normals[indices[i]]);
	}
	m_faces.emplace_back(Face(pos.data(),norm.data(),m_type, m_node->transform(),m_color, m_albedo));
}

void Mesh::clear() {

	m_vertices.clear();
	m_normals.clear();
	m_indices.clear();
}


void Mesh::upload() {
	
	glGenTextures(1,&m_textureId);
	glGenBuffers(1,&m_glIbuffer);
	glGenBuffers(1,&m_glVbuffer);
	glGenBuffers(1,&m_glNbuffer);
	glGenBuffers(1,&m_glTbuffer);

	glBindBuffer(GL_ARRAY_BUFFER, m_glVbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vec3f),m_vertices.data(),GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_glNbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(Vec3f), m_normals.data(),GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_glTbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_texcoords.size() * sizeof(Vec2f), m_texcoords.data(),GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint), m_indices.data(), GL_STATIC_DRAW);
}

void Mesh::draw() {
	
	glPolygonOffset(1.0, 2);
	if(m_drawMode  == DMode::DM_LIGHT || m_drawMode  == DMode::DM_FLAT || m_drawMode == DMode::DM_POINTS) {
		glCullFace(GL_BACK);

		if(m_drawMode  == DMode::DM_LIGHT) {
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
		}		
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		
 		glBindBuffer(GL_ARRAY_BUFFER, m_glVbuffer);	
		glVertexPointer(m_type, GL_FLOAT, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, m_glNbuffer);	
		glNormalPointer(GL_FLOAT, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, m_glTbuffer);
		glTexCoordPointer(2, GL_FLOAT,0,0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIbuffer);
		glColor3fv(glm::value_ptr(m_color));
//		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);				
		glDrawElements(m_drawMode == DMode::DM_POINTS ? GL_POINTS : GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0); //
//		glDisable(GL_POLYGON_OFFSET_FILL);
//		glColor4f(0.0f,0.0f, 0.0f, 1.0f);
//		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);		
//		glDrawElements(m_drawMode == DMode::DM_POINTS ? GL_POINTS : GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0); 

		if(m_drawMode  == DMode::DM_LIGHT) {
			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHTING);
		}		

	}
	//else if(m_drawMode == DMode::DM_WIREFRAME) 
	//{
	//glDisable(GL_DEPTH_TEST);
/*	glColor4f(0.0f,0.0f, 0.0f, 1.0f);
	float lineWidth = 0;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(2.0f);
	for(uint i = 0; i < m_faces.size(); i++) {
		glBegin(GL_LINE_LOOP);
		for(int v = 0; v < m_type; v++) {
			glVertex3fv(glm::value_ptr(m_faces[i].position(v)));
		}
		glEnd();			
	}
		}	*/			

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Mesh::setTexture(float* tex, Vec2i size, uint depth) {
	
	m_texSize = size;
	m_texture.clear();
	for(int x = 0; x < size.x; x++) {
		for(int y = 0; y < size.y; y++) {
			m_texture.push_back(Vec3f(	tex[y * size.x * 3 + x * 3], 
										tex[y * size.x * 3 + x * 3 + 1], 
										tex[y * size.x * 3 + x * 3 + 2]));
		}
	}
	std::for_each(m_texture.begin(), m_texture.end(), [] (Vec3f& d) { d =  (d / (float)PhotonMap::globalExposure); });
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_FLOAT, tex);
	glBindTexture(GL_TEXTURE_2D,0);

	
}

const Vec3f& Mesh::sample_texture(const Vec2f& coord) {

	if(m_texture.size() == 0) return s_empty_pixel;
	return m_texture[(size_t)(coord.x * m_texSize.y) * m_texSize.x + (size_t)(coord.y * m_texSize.x)];
}

void Mesh::updateFaces() {
	m_faces.clear();	
	std::vector<Vec3f> pos, norm; int size = 0;
	for(uint i = 0 ; i < m_indices.size(); i++) {

		pos.push_back(m_vertices[m_indices[i]]);
		norm.push_back(m_normals[m_indices[i]]);		
		if((i + 1) % (int)m_type == 0) {
		Mat4f transform = m_node == nullptr ? Mat4f(1) : m_node->transform();
			m_faces.push_back(Face(pos.data(),norm.data(),m_type, transform, m_color, m_albedo));
			//size = 0;
			pos.clear(); norm.clear();
		}
	}

}


Mesh::~Mesh() {
	
	glDeleteTextures(1,&m_textureId);
	glDeleteBuffers(1,&m_glIbuffer);
	glDeleteBuffers(1,&m_glVbuffer);
	glDeleteBuffers(1,&m_glNbuffer);
	glDeleteBuffers(1,&m_glTbuffer);
}