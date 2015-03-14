#include "Definitions.hpp"
#include "Math.hpp"
#include "Camera.hpp"

#include <vector>


Camera::Camera(float fov, float aspect, float near, float far, const char* name,  SceneNode* parent) : Entity(name, Entity::Camera, parent),
	m_fov(fov), m_aspect(aspect), m_near(near), m_far(far) {
		
	invalidateMatrix();
}

void Camera::setFov(float fov) 
{ 
	m_fov = fov; 
	invalidateMatrix();
};

void Camera::setAspect(float aspect) { 

	m_aspect = aspect; 
	invalidateMatrix();
}


void Camera::draw() {

	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINE_LOOP);
		glVertex4f(0,0,0,1);
		glVertex4f(0.5f,0.5f,-1,1);
		glVertex4f(-0.5f,0.5f,-1,1);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex4f(0,0,0,1);
		glVertex4f(0.5f,-0.5f,-1,1);
		glVertex4f(-0.5f,-0.5f,-1,1);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex4f(0,0,0,1);
		glVertex4f(0.5f,-0.5f,-1,1);
		glVertex4f(0.5f,0.5f,-1,1);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex4f(0,0,0,1);
		glVertex4f(-0.5f,-0.5f,-1,1);
		glVertex4f(-0.5f,0.5f,-1,1);
	glEnd();
};

void Camera::invalidateMatrix() {
	
	m_near = 0.52f;
	//todo: m_fov to m_fovy
	m_projectionMatrix = glm::perspective(m_fov / m_aspect, m_aspect,m_near,m_far);
}