#pragma once
#include "Entity.hpp"

class SceneNode;
class Light : public Entity {
	
public:

	Light(SceneNode* parent = 0);

	Light(const Color3f& color, float intensity = 1.0f, const char* name = "", SceneNode* parent = 0);

	const Color3f& color() const { return m_color; }

	const float intensity() const { return m_intensity; }

	void set_color(const Color3f& color);

	void set_intensity(float intensity);

	const Mat4f& tranformation() { return m_node->transform(); }

	const Vec3f position() { return Vec3f(m_node->transform() * Vec4f(0,0,0,1)); }

	virtual void random_dir(Vec3f& origin, Vec3f& dir);

	virtual void draw();

private:
	Color3f m_color;
	float m_intensity;
	std::string m_name;
};