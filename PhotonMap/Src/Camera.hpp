#pragma once
#include "SceneNode.hpp"
#include"Entity.hpp"


class Camera : public Entity {
	
public:
	Camera(float fov, float aspect, float near, float far, const char* name = "", SceneNode* parent = 0);

	float getFov() const { return m_fov; };

	float getAspect() const { return m_aspect; };

	void setFov(float fov);

	void setAspect(float aspect); 

	void draw();

	const Mat4f& projection() { return m_projectionMatrix; }

	const Mat4f& tranformation() { return m_node->transform(); }

private:
	void invalidateMatrix();

private:
	Mat4f m_projectionMatrix;
	//SceneNode* m_sceneNode;
	float m_fov;
	float m_aspect;
	float m_near;
	float m_far;
};