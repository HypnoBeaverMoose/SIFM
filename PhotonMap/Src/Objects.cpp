#include"Definitions.hpp"
#include"Math.hpp"
#include"Objects.hpp"


Vec3f IFace::black_pixel = Vec3f(0);

#define EPS 0.0001f
Face::Face(Vec3f* positions, Vec3f* normals,Primitive type, const Mat4f& transform, const Vec3f color, float albedo) :
	IFace(FACE),  m_type(type), 
	m_transform(transform),
	m_color(color), m_albedo(albedo),	
	m_normals(normals, normals + (int)type),
	m_positions(positions, positions + (int)type) {

	m_aabb.min = Vec3f(FLT_MIN);
	m_aabb.min = Vec3f(FLT_MIN);
	for(int i = 0; i < type; i++) {
		Vec3f worldPos = Vec3f(m_transform * Vec4f(m_positions[i],1));
		m_worldPositions.push_back(worldPos);
		m_worldNormals.push_back(glm::normalize(Vec3f(m_transform * Vec4f(m_normals[i],0))));
		for(uint j = 0; j < 3; j++) {
			m_aabb.min[j] = glm::min(m_aabb.min[j], worldPos[j]);
			m_aabb.max[j] = glm::max(m_aabb.max[j], worldPos[j]);
		}
	}
}


bool Face::intersect(const Vec3f& origin, const Vec3f& dir, float& out)  const {

	glm::simdVec4 _dir(dir,0);
	glm::simdVec4 e1(m_positions[1] - m_positions[0],0), e2(m_positions	[2] - m_positions[0],0);
	glm::simdVec4  t(origin - m_positions[0],0);
	glm::simdVec4  p(glm::cross(_dir,e2));
	glm::simdVec4  q(glm::cross(t,e1));

	float det = glm::dot(e1,p);
	if(glm::abs(det) < EPS) return false;
	det = 1.0f /  det;

	float u = det * glm::dot(t,p);
	if(u <0.0f || u > 1.0f) return false;
				
	float v = glm::dot(_dir,q) * det;
	if(v <0.0f ||  u + v  > 1.0f) return false;

	out = glm::dot(e2,q) * det;
	return out > EPS;
}
bool Face::intersectWorld(const Vec3f& origin, const Vec3f& dir, float& out) const {

	glm::simdVec4 _dir(dir,0);
	glm::simdVec4 e1(m_worldPositions[1] - m_worldPositions[0],0), e2(m_worldPositions[2] - m_worldPositions[0],0);
	glm::simdVec4  t(origin - m_worldPositions[0],0);
	glm::simdVec4  p(glm::cross(_dir,e2));
	glm::simdVec4  q(glm::cross(t,e1));


	float det = glm::dot(e1,p);
	if(glm::abs(det) < EPS) return false;
	det = 1.0f /  det;

	float u = det * glm::dot(t,p);
	if(u <0.0f || u > 1.0f) return false;
				
	float v = glm::dot(	_dir,q) * det;
	if(v <0.0f ||  u + v  > 1.0f) return false;

	out = glm::dot(e2,q) * det;
	return out > EPS;
}

bool AABB::intersect(const Vec3f &origin, const Vec3f &dir, float& t) const {

	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float inv_x = 1.0f / dir.x;
	float inv_y = 1.0f / dir.y;

	if (dir.x >= 0) {
		tmin = (min.x - origin.x) * inv_x; 
		tmax = (max.x - origin.x) * inv_x;
	}
	else {
		tmin = (max.x - origin.x) * inv_x;
		tmax = (min.x - origin.x) * inv_x;
	}

	if (dir.y >= 0) {
		tymin = (min.y - origin.y) * inv_y;
		tymax = (max.y - origin.y) * inv_y;
	}
	else {
		tymin = (max.y - origin.y) * inv_y;
		tymax = (min.y - origin.y)  * inv_y;
	}
	
	if ( (tmin > tymax) || (tymin > tmax) )
	return false;

	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	float inv_z = 1.0f / dir.z;
	if (dir.z >= 0) {
		tzmin = (min.z - origin.z) * inv_z;
		tzmax = (max.z - origin.z)  * inv_z;
	}
	else {
		tzmin = (max.z - origin.z)  * inv_z;
		tzmax = (min.z - origin.z)  * inv_z;
	}
	
	if ( (tmin > tzmax) || (tzmin > tmax) )
		return false;
	
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;
	t = tmin;
	return tmin < tmax;
}
void AABB::draw() const {
	glColor3f(1.0f,1.0f, 1.0f);
	float lineWidth =0;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(1.0f);

	glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(min.x, max.y, min.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, min.z);
		glVertex3f(min.x, max.y, max.z);
	glEnd();

	glLineWidth(lineWidth);
}
void AABB::draw() {

		glColor3f(1.0f,1.0f, 1.0f);
	float lineWidth =0;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(1.0f);

	glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(min.x, max.y, min.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, min.z);
		glVertex3f(min.x, max.y, max.z);
	glEnd();

	glLineWidth(lineWidth);
}

bool AABB::contains(const Vec3f &point) const  {

	return glm::all(glm::lessThanEqual(min,point)) && glm::all(glm::greaterThanEqual(max, point));
}

bool AABB::contains(const Face &face) const {

	/*AABB faceAABB;
	for(int i = 0; i < face.type(); i++) {
		for(int j = 0; j < 3; j++) {
			faceAABB.min[j] = glm::min(faceAABB.min[j], face.worldPosition(i)[j]);
			faceAABB.max[j] = glm::max(faceAABB.max[j], face.worldPosition(i)[j]);
		}
	}*/
	AABB faceAABB = face.aabb();
	Vec3f  v1 = glm::abs(this->min - faceAABB.min) + glm::abs(this->max - faceAABB.max);
	Vec3f  v2 = (faceAABB.max - faceAABB.min) + (this->max - this->min);
	return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z;
}
