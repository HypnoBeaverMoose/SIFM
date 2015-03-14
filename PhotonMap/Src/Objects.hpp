#pragma once
#include "Math.hpp" 
#include"instance_count_base.hpp"
#include<vector>

enum Primitive{TRIANGLES = 3, QUADS};

enum FaceType { FACE, TILE };
class Face;
struct AABB;

class IFace {

public:
	IFace( uint faceType) :  m_faceType(faceType){}

	virtual uint size() = 0;
	virtual uint size() const  = 0;

	virtual const Vec3f& position(uint index)  = 0;
	virtual const Vec3f& position(uint index) const = 0;
	
	virtual const Vec3f& normal()  = 0;
	virtual const Vec3f& normal() const  = 0;

	virtual const Vec3f& worldPosition(uint index)  = 0;
	virtual const Vec3f& worldPosition(uint index) const  = 0;

	virtual const Vec3f& worldNormal()  = 0;
	virtual const Vec3f& worldNormal() const  = 0;

	virtual const Mat4f& transform()  = 0;
	virtual const Mat4f& transform() const  = 0;

	virtual const AABB& aabb()  = 0;
	virtual const AABB& aabb() const  = 0;

	virtual bool intersect(const Vec3f& origin, const Vec3f& dir, float& out) const = 0;
	virtual bool intersectWorld(const Vec3f& origin, const Vec3f& dir, float& out) const = 0;

	virtual const Vec3f& color() = 0;
	virtual const float albedo() = 0;

	virtual const Vec3f& sampleTexture(const Vec3f& pos) { return black_pixel;}

	uint type() { return m_faceType; }
	uint type() const  { return m_faceType; }

	virtual ~IFace() {}

protected:
	uint m_faceType;
private:
	static Vec3f black_pixel;

};

struct AABB {
	bool intersect(const Vec3f &origin, const Vec3f &dir, float& t) const;

	bool contains(const Vec3f &point) const ;	
	bool contains(const Face &face) const;
	
	void draw();
	void draw() const;
	
	Vec3f min, max;
};


class Face : public IFace {

public:
	Face() :IFace(FACE) {};

	Face(Vec3f* positions, Vec3f* normals, Primitive type, const Mat4f& transform, const Vec3f color, float albedo);

	uint size() { return m_type; }

	uint size() const { return m_type; }

	const Vec3f& position(uint index) { return m_positions[index]; }
	const Vec3f& position(uint index) const { return m_positions[index]; }
	
	const Vec3f& normal(uint index) { return m_normals[index]; }
	const Vec3f& normal(uint index) const { return m_normals[index]; }

	const Vec3f& normal() { return m_normals[0]; }
	const Vec3f& normal() const { return m_normals[0]; }

	const Vec3f& worldPosition(uint index) { return m_worldPositions[index]; }
	const Vec3f& worldPosition(uint index) const { return m_worldPositions[index]; }

	const Vec3f& worldNormal(uint index) { return m_worldNormals[index]; }
	const Vec3f& worldNormal(uint index) const { return m_worldNormals[index]; }

	const Vec3f& worldNormal() { return m_worldNormals[0]; }
	const Vec3f& worldNormal() const { return m_worldNormals[0]; }

	const Mat4f& transform() { return m_transform; }
	const Mat4f& transform() const  { return m_transform; }

	const AABB& aabb() { return m_aabb; };
	const AABB& aabb() const { return m_aabb;};

	bool intersect(const Vec3f& origin, const Vec3f& dir, float& out) const;
	bool intersectWorld(const Vec3f& origin, const Vec3f& dir, float& out) const;

	void setAABB(const AABB& aabb) { m_aabb = aabb; }

	const Vec3f& color() { return m_color; }
	const float albedo() { return m_albedo;}

	virtual ~Face() {}

private:
	std::vector<Vec3f>	m_positions;
	std::vector<Vec3f>	m_normals;

	std::vector<Vec3f>	m_worldPositions;
	std::vector<Vec3f>	m_worldNormals;

	Mat4f				m_transform;
	Primitive			m_type;
	Vec3f				m_color;
	float				m_albedo;
	AABB				m_aabb;
};