#include<cimg\CImg.h>


#include "Definitions.hpp"
#include "Math.hpp"
#include "Framebuffer.hpp"
#include"PhotonMap.hpp" 


Framebuffer::Framebuffer() : m_size(0,0), m_type(RGB){

}
void Framebuffer::init(uint width, uint height, Type type)  {

	m_size = Vec2i(width,height);
	m_type = type;
	uint texType = type == RGBA ? GL_RGBA : GL_RGB;

	m_image.assign(width * height * type, 0.0f);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1,&m_glTextureId);
	glBindTexture(GL_TEXTURE_2D,m_glTextureId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,0,texType,width,height,0,texType,GL_FLOAT,m_image.data());
}

Color3f Framebuffer::getPixel(int x, int y) {
	
	Color3f color(0,0,0);
	if(x < m_size.x && y < m_size.y)  {	
		memcpy(glm::value_ptr(color),&(m_image[y * m_size.x * m_type + x * m_type]), 3 * sizeof(float));
	}
	
	return color;
}

void Framebuffer::setPixel(int x, int y, Color3f color) {
	
	if(color.x == std::numeric_limits<float>().quiet_NaN())
		debug("something is wrong");
	memcpy(&(m_image[y * m_size.x * m_type + x * m_type]),glm::value_ptr(color), 3 * sizeof(float));
}

const float* Framebuffer::getPtr() const{
	
	return m_image.data();
}

uint Framebuffer::getTextureID() {
	
	return m_glTextureId;
}

void Framebuffer::updateTexure() const{
	
	std::vector<float> exp_image;
	for(size_t i = 0; i < m_image.size(); i++) {
		exp_image.push_back(m_image[i] * PhotonMap::globalExposure);
	}

	glBindTexture(GL_TEXTURE_2D,m_glTextureId);
	uint texType = m_type == RGBA ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0,texType,m_size.x,m_size.y, 0,texType,GL_FLOAT,exp_image.data());
	//glGenerateMipmap(GL_TEXTURE_2D);
}


void Framebuffer::bindFramebuffer() const {
	
	glBindTexture(GL_TEXTURE_2D,m_glTextureId);
}

void Framebuffer::clear(Color3f clearColor) {

		for(int x = 0; x < m_size.x; x++) {
			for(int y = 0; y < m_size.y; y++) {
				setPixel(x,y,clearColor);
			}
		}
}

void Framebuffer::save_framebuffer(const std::string filename)  const {

	std::vector<byte> img;
	const float* ptr = m_image.data();
	for(size_t channel = 0; channel < 3; channel++) {
		for(int y = m_size.y - 1; y >= 0; y--) {
			for(int x = 0; x < m_size.x; x++) {
				img.push_back((byte)(glm::min(255.0f, ptr[y * m_size.x * 3 + x * 3 + channel] *PhotonMap::globalExposure * 255)));
			}

		}
	}
	cimg_library::CImg<byte> image(img.data(), m_size.x, m_size.y, 1, 3, false);
	image.save(filename.c_str());
}