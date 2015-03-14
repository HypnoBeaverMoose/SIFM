#include "Definitions.hpp"
#include "Math.hpp"
#include "Light.hpp"


Light::Light(SceneNode* parent): Entity("", Entity::Light, parent),  m_color(1,1,1), m_intensity(1.0f) {
}

Light::Light(const Color3f& color, float intensity, const char* name, SceneNode* parent):
	Entity(name, Entity::Light, parent), m_color(color), m_intensity(intensity){

}

void Light::set_color(const Color3f& color) { 
	m_color = color; 
}

void Light::set_intensity(float intensity) {
	m_intensity = intensity; 
}

void Light::draw() {

	
	glColor3f(1.0f,0.9f,0.0f);
	glPushMatrix();
	glScalef(0.5f,0.5f,0.5f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);	
		for(float angle = 0; angle <= 2.0f * (float)PI; angle+=PI / 8) {

			glVertex2f(cos(angle),sin(angle));
			glVertex2f(1.5f * cos(angle), 1.5f * sin(angle));
			glVertex2f(cos(angle),sin(angle));
		}
	glEnd();
	glPopMatrix();
}

void Light::random_dir(Vec3f& origin, Vec3f& dir) {

	origin  = position();
	dir = glm::sphericalRand(1.0f);
}