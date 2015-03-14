#pragma once
#include "Definitions.hpp"
#include "Math.hpp"

class Photon {
public:
	Photon(): m_position(0), m_direction(0), m_color(0) {}
	Photon(Vec3f origin, Vec3f dir, Color3f color) : m_position(origin), m_direction(dir), m_color(color) {}

	
	const Vec3f& postition() const { return m_position; }
	const Vec3f& postition() { return m_position; }
	void setPostition(const Vec3f& pos) { m_position = pos; }

	const Vec3f& direction() const { return m_direction; }
	const Vec3f& direction() { return m_direction; }
	void setDirection(const Vec3f& dir) { m_direction = dir; }


	const Color3f& color() const { return m_color; }
	const Color3f& color() { return m_color; }
	void setColor(const Color3f& col) { m_color = col; }
	
	uint compare(float value, uint axis);

	bool contatins(const Vec3f pnt) { return false; };

	inline float x() const { return m_position.x; }

	inline float y() const { return m_position.y; }

	inline float z() const { return m_position.z; }

private:
	Vec3f	m_position;
	Vec3f	m_direction;
	Color3f m_color;
};





