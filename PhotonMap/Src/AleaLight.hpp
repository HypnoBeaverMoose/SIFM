#pragma once
#include "Light.hpp"


class AreaLight : public Light {

public:
	AreaLight(float size, const Color3f& color, float intensity = 1.0f, const char* name = "", SceneNode* parent = 0);

	void random_dir(Vec3f& origin, Vec3f& dir);

	void draw();

private:
	Vec3f m_normal;
	float m_size;
};