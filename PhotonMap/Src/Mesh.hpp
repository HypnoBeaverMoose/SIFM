#pragma once
#include"instance_count_base.hpp"
#include "SceneNode.hpp"
#include"Objects.hpp"
#include"Entity.hpp"

class Mesh : public Entity {

public:
	Mesh(Primitive type = TRIANGLES, const char* name = "", SceneNode* parent = 0, const Vec3f& color = Vec3f(1), float albedo = 0.75f);
	
	uint addVertex(const Vec3f& vertex); 

	uint addVertex(const Vec3f& vertex, const Vec3f& normal); 

	void insertTexCoord(const Vec2f& coord, uint index);

	void addFace(const unsigned int* indices, uint count);

	uint faceCount() const { return m_indices.size() / m_type; }

	const Face& face(uint index) const { return  m_faces[index]; }

	const AABB& aabb() const { return m_aabb; }

	const Mat4f& tranformation() { return m_node->transform(); }
	
	const Face* faces();
	
	void updateFaces();

	void clear();

	void upload();

	void draw();

	uint drawMode() { return m_drawMode; }

	void setDrawMode(uint mode) { m_drawMode = mode; }

	void setTexture(float* tex, Vec2i size, uint depth);

	const Vec3f& color() { return m_color; }

	void set_color(const Vec3f& col) { m_color = col; }

	const float albedo() { return m_albedo; }

	void set_albedo(float albedo) { m_albedo = glm::clamp(albedo, 0.0f, 1.0f); }

	const Vec3f& sample_texture(const Vec2f& coord);

	const std::vector<Vec3f>& get_texture() { return m_texture; }

	~Mesh();

private:
	unsigned int		m_glVbuffer;
	unsigned int		m_glIbuffer;
	unsigned int		m_glNbuffer;
	unsigned int		m_glTbuffer;
	unsigned int		m_drawMode;
	unsigned int		m_textureId;
	AABB		m_aabb;
	Primitive	m_type;
		
	static Vec3f s_empty_pixel;
protected:
	std::vector<Vec2f>	m_texcoords;
	std::vector<Vec3f>	m_vertices;
	std::vector<Vec3f>	m_normals;
	std::vector<uint>	m_indices;
	std::vector<Face>	m_faces;
	std::vector<Vec3f>	m_texture;
	Vec3f				m_color;
	Vec2i				m_texSize;
	float				m_albedo;

};