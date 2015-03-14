#pragma once
#include "SceneNode.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Light.hpp"
#include "Octree.hpp"
#include "Scene.hpp"
#include "Framebuffer.hpp"
#include "Photon.hpp"
#include <nanoflann.hpp>

#include"timer.h" 
#include<thread>
#include<mutex>
class Raytracer {
	typedef std::function<Color3f(size_t,size_t,const Vec3f&, const Vec3f&,IFace*)> renderer;

public:
	struct Ray {

		Ray() :dir(0,0,0), origin(0,0,0,1) {}
		Ray(const Vec3f& _dir, const Vec4f& _origin ) :dir(_dir), origin(_origin) {}
		void transform(const Mat4f& mat) { dir = Vec3f(mat * glm::normalize(Vec4f(dir,0) )); origin = mat * origin ; }
		
		Vec3f dir;
		Vec4f origin;
	};

	struct hit_point {
		hit_point() : radius(-1) {}
		Vec3f position;
		Vec3f normal;
		Vec3f inc_dir;
		Vec2i pixel;
		double radius;
		Tile* tile;
		size_t photon_count;
		Vec3f flux;
		Vec3f color;
	};

	Raytracer();

	Raytracer(Vec2i resolution);

	void init(Vec2i resolution);

	const Framebuffer& framebuffer() { return m_framebuffer; }

	void setFramebuffer(const Framebuffer& fmbfr);
	
	void start_density(uint nnCount, Scene* const scene, bool use_hit_points, size_t threadCount);

	bool start_raytracing(Scene* const scene, bool lighting, bool texturing, size_t threadCount);

	void start_photon_shoot(uint count, Scene* const scene, size_t threadCount, bool clear = true) ;
	
	void start_progressive_render(Scene* scene, float alpha, size_t photonCount, size_t nnCount, int iterations, size_t threadCount);

	void start_filtered_render(Scene* scene, size_t photonCount, size_t nnCount,size_t threadCount);

	void start_filtered_render2(Scene* scene, size_t photonCount, size_t nnCount,size_t threadCount);

	void generate_hit_points(Scene* scene, size_t photonCount, size_t threadCount);

	void drawRays();

	void drawPhotonPaths();

	//bool is_working() { return m_workingThreads > 0; }
	

private:

	void filtered_pass(Scene* scene, hit_point* hit_points, size_t count, size_t nnCount, int iteration);

	void progressive_pass(Scene* scene, hit_point* hit_points, size_t count, float alpha);
	
	void raytrace(Scene* const scene, renderer render, size_t from, size_t to);

	void shootPhotons(uint count, Scene* const scene);

private:
	Framebuffer m_framebuffer;
	Vec2i	m_resolution;	
	size_t	m_photonCount;
	size_t  m_rendered_row;
	std::vector<hit_point> m_hit_points;
	std::mutex m_mutex;
};
