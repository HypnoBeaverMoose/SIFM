#pragma once

class Framebuffer {

public:
	enum Type { RGB = 3, RGBA =4 };

public:
	Framebuffer();

	void init(uint width, uint height, Type type);

	Color3f getPixel(int x, int y);

	void setPixel(int x, int y, Color3f color);

	const float* getPtr() const;

	uint getTextureID();

	void updateTexure() const;

	void bindFramebuffer() const;

	void save_framebuffer(const std::string filename) const;

	void clear(Color3f clearColor);

	const Vec2i& size() const { return m_size; }

private:
	std::vector<float>	m_image;
	Vec2i				m_size;
	unsigned int		m_glTextureId;	
	Type				m_type;
};