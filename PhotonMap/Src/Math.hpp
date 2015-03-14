#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SSE2
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm\gtc\random.hpp>
#include<glm/gtx/simd_mat4.hpp>
#include<glm/gtx/simd_vec4.hpp>
#include<random>
#include<chrono>
#define DEG2RAD 0.0174532925f
#define PI		3.14159265f

typedef glm::vec4 Vec4f;
typedef glm::vec3 Vec3f;
typedef glm::ivec2 Vec2i;
typedef glm::vec2 Vec2f;
typedef glm::vec3 Color3f;

typedef glm::mat4 Mat4f;
typedef glm::mat3 Mat3f;

static std::default_random_engine random_engine((unsigned long)(std::chrono::system_clock::now().time_since_epoch().count()));
static std::uniform_real_distribution<double> dist01(0.0f,1.0f);

inline Vec3f sampleSphere(float radius = 1.0f) {

	//random_engine.seed((long)(std::chrono::system_clock::now().time_since_epoch().count()));
	double phi = 2 * dist01(random_engine) * glm::pi<double>();
	double theta =  glm::acos<double>(2 * dist01(random_engine) - 1);
	///random_engine.seed((long)(dist01(random_engine)*100000));
	return radius * Vec3f((float)(glm::sin<double>(theta) * glm::cos<double>(phi)), 
		(float)(glm::sin<double>(theta) * glm::sin<double>(phi)), (float)glm::cos<double>(theta));
}


inline Vec3f sampleCossHemi(const Vec3f& normal) {

	//Vec3f vec = glm::normalize(sampleSphere(1.0f));
	Vec3f vec = sampleSphere(1.0f);
	return glm::normalize(vec + normal);
}


inline Vec3f sampleHemi(const Vec3f& normal) {

	Vec3f vec;
	do {
		vec = glm::normalize(sampleSphere(1.0f));
	}while(glm::dot(vec, normal)<= 0);

	return vec;
}
