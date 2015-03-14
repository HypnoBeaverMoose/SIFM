#include"Definitions.hpp"
#include"Math.hpp"
#include"AleaLight.hpp"
#include<random>

AreaLight::AreaLight(float size, const Color3f& color, float intensity, const char* name, SceneNode* parent) :
	Light(color,intensity, name, parent), m_size(size) {

}

void AreaLight::random_dir(Vec3f& origin, Vec3f& dir) {


	origin = Vec3f(tranformation() *  Vec4f(dist01(random_engine) * m_size - m_size / 2, dist01(random_engine) * m_size - m_size / 2, 0, 1));
	dir = sampleCossHemi(Vec3f(0,-1, 0));

	//	dir = Vec3f(tranformation() * glm::normalize(Vec4f(dir, 0)));
	
}

void AreaLight::draw() { 

	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(-m_size / 2, -m_size / 2, 0);
		glVertex3f(m_size / 2, -m_size / 2, 0);
		glVertex3f(m_size / 2, m_size / 2, 0);
		glVertex3f(-m_size / 2, m_size / 2, 0);
	glEnd();
	glBegin(GL_LINE);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0,  - m_size / 2);
	glEnd();

}