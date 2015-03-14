#define _VARIADIC_MAX 10
#include "Definitions.hpp"
#include "Math.hpp"
#include "Raytracer.hpp"
#include "PhotonMap.hpp"
#include "EnhancedFilter.hpp"
#include"Tile.hpp"
#include<tuple>
#include<mutex>
#include<random>
#include<functional>
#include<string>


Raytracer::Raytracer(): m_resolution(0,0)
{ 
}

Raytracer::Raytracer(Vec2i resolution): m_resolution(resolution) {

	init(resolution);
}

void Raytracer::init(Vec2i resolution) {
	
	m_resolution = resolution;
	m_framebuffer.init(resolution.x, resolution.y,Framebuffer::RGB);
	m_hit_points.resize(resolution.x * resolution.y, hit_point());

}

void Raytracer::setFramebuffer(const Framebuffer& fmbfr) {

	m_framebuffer = fmbfr;
}
void Raytracer::drawPhotonPaths() {
	
	float lineWidth = 0;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(2.0f);
	glColor3f(1,1,1);
	for(uint i = 0; i < m_photonPaths.size(); i++) {
		glBegin(GL_LINE_STRIP);
		for(uint j = 0; j < m_photonPaths[i].size(); j++) 
			glVertex3fv(glm::value_ptr(m_photonPaths[i][j]));
		glEnd();
	}
	glLineWidth(lineWidth);

}

void Raytracer::start_density(uint nnCount, Scene* const scene, bool use_hit_points, size_t threadCount) {

	std::vector<std::thread> threads;
	size_t inc = (m_resolution.y + threadCount - 1) / threadCount;	
	m_hit_points.resize(m_resolution.x * m_resolution.y, hit_point());
	debug("\ndensity render start\n");

	std::function<Color3f(size_t,size_t, Vec3f, Vec3f, IFace*)> fn = 
		[this, &scene, &nnCount, &use_hit_points](size_t x, size_t y, Vec3f intersection, Vec3f l_dir, IFace* res_face)->Color3f{
		double radius = use_hit_points ? this->m_hit_points[y * m_resolution.x + x].radius : 0;
		Vec3f flux = Vec3f(0);
		if(use_hit_points)
			flux = scene->photonMap()->radiusSearch(intersection, radius, res_face->worldNormal(), nnCount , Vec3f(1),  false);
		else	
			flux = scene->photonMap()->photonDensity(intersection, res_face->worldNormal(), nnCount, radius, Vec3f(1),false);

		return res_face->color() *( flux /(float)(radius * PI *  PhotonMap::globalPhotonCount));
	};	
	m_rendered_row = 0;
	for(int from  = 0; from < m_resolution.y; from+=inc) {
		std::thread(&Raytracer::raytrace, this,scene, fn, from, std::min<size_t>(m_resolution.y,from+inc)).detach();	
	}

	while(is_working()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		size_t percentage = (m_rendered_row / (float)m_resolution.y) * 100;
		debug("rendering density: [ %d%%]\n",percentage);
	}
	Vec3f avg_variance(0);
	for(size_t x = 0 ; x < m_resolution.x; x++) {
		for(size_t y = 0 ; y < m_resolution.x; y++) {
			avg_variance+=m_framebuffer.getPixel(x,y);
		}
	}
	avg_variance /= (float)m_resolution.x * m_resolution.y;
	debug("average variance: %f, %f, %f\n", avg_variance.x, avg_variance.y, avg_variance.z);
}

bool Raytracer::start_raytracing(Scene* const scene, bool lighting, bool texturing, size_t threadCount) {

	size_t inc = (m_resolution.y + threadCount - 1) / threadCount;
	
	std::function<Color3f(size_t,size_t, Vec3f, Vec3f, IFace*)> fn = 
		[&lighting, &texturing](size_t x, size_t y, Vec3f intersection, Vec3f l_dir, IFace* res_face)->Color3f{
		const Vec3f& color  = texturing ?  res_face->color() * res_face->sampleTexture(intersection) : res_face->color();
		if(lighting) 
			return color* glm::abs(glm::dot(l_dir, glm::normalize(res_face->worldNormal())));
		else
			return color;	
	};	
	m_workingThreads = threadCount;
	m_rendered_row = 0;
	for(int from  = 0; from < m_resolution.y; from+=inc)
		std::thread(&Raytracer::raytrace, this,scene, fn, from, std::min<size_t>(m_resolution.y,from+inc)).detach();

	
	while(is_working()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		size_t percentage = (m_rendered_row / (float)m_resolution.y) * 100;
		debug("raytracing: [ %d%%]\n",percentage);
	}

	return true;
}


void Raytracer::generate_hit_points(Scene* scene,size_t photonCount, size_t threadCount) {

	size_t inc = (m_resolution.y + threadCount - 1) / threadCount;
	m_hit_points.assign(m_resolution.x * m_resolution.y, hit_point());

	std::function<Color3f(size_t,size_t, Vec3f, Vec3f, IFace*)> fn = 
	[this, &scene,&photonCount](size_t x, size_t y, Vec3f intersection, Vec3f l_dir, IFace* res_face)->Color3f{
		hit_point& hp = this->m_hit_points[y * this->m_resolution.x + x];
		scene->photonMap()->photonDensity(intersection, res_face->worldNormal(), photonCount, hp.radius, Vec3f(1),  false);
		hp.position = intersection;		hp.normal = res_face->worldNormal();
		hp.pixel = Vec2i(x,y);			hp.photon_count = photonCount;
		hp.color = res_face->color();	hp.tile = 0;
		hp.flux = Vec3f(0);				return Vec3f(0);
	};
	m_rendered_row = 0;
	m_workingThreads = threadCount;
	for(int from  = 0; from < m_resolution.y; from+=inc)
		std::thread(&Raytracer::raytrace, this,scene, fn, from, std::min<size_t>(m_resolution.y,from+inc)).detach();

	while(is_working()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		size_t percentage = (m_rendered_row / (float)m_resolution.y) * 100;
		debug("generating hit points: [ %d%%]\n",percentage);
	}

}

void Raytracer::start_progressive_render(Scene* scene, float alpha, size_t photonCount, size_t nnCount, int iterations, size_t threadCount) {

	start_photon_shoot(photonCount,scene,threadCount);
	scene->buildPhotonMap();
	generate_hit_points(scene, nnCount, threadCount);

	IFace** tiles = scene->tiles();

	int counter = 0;
	while(true) {

		if(iterations > 0 && counter > iterations) break; 
		counter++;

		scene->clearPhotons();				
		start_photon_shoot(photonCount, scene, threadCount, false);
		//PhotonMap::globalPhotonCount += photonCount;	
		scene->photonMap()->rebuildKdTree();
		
		if(iterations > 0)
			debug("ITERATION %d OUT OF %d\n", counter, iterations);
		else
			debug("ITERATION %d\n", counter);

		int inc = (m_hit_points.size() + threadCount - 1) / threadCount;			
		
		m_workingThreads = threadCount;
		for(size_t from  = 0; from < m_hit_points.size(); from+=inc)
			std::thread(&Raytracer::progressive_pass, this, scene, m_hit_points.data() + from,
														std::min<size_t>(m_hit_points.size() - from, inc), alpha).detach();

		while (is_working()){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		m_framebuffer.save_framebuffer("results/prgoressive.bmp");
	}
}

void Raytracer::progressive_pass(Scene* scene, hit_point* hit_points, size_t count, float alpha) {
	
	for(size_t i = 0; i < count; i++) {
		hit_point& hp = hit_points[i];
		size_t photon_count = 0;
		hp.flux += scene->photonMap()->radiusSearch(hp.position,(float)hp.radius, hp.normal, photon_count, Vec3f(1), false);
		float beta = (hp.photon_count + alpha * photon_count) / (float)(hp.photon_count + photon_count);
		hp.radius *= beta;
		hp.flux *= beta;
		hp.photon_count += photon_count;
		Vec3f flux(hp.flux.x / (double)(hp.radius * PI *  PhotonMap::globalPhotonCount),
							hp.flux.y / (double)(hp.radius * PI *  PhotonMap::globalPhotonCount),
								hp.flux.z / (double)(hp.radius * PI *  PhotonMap::globalPhotonCount));

		m_framebuffer.setPixel(hp.pixel.x, hp.pixel.y, hp.color * flux);
	}

	std::lock_guard<std::mutex> lg(m_mutex);
	m_workingThreads--;
}

void Raytracer::start_filtered_render(Scene* scene, size_t photonCount, size_t nnCount,size_t threadCount) {

	//start_photon_shoot(photonCount,scene,threadCount);
	//scene->buildPhotonMap();
	//
	//scene->filterScene(3);
	//generate_hit_points(scene, nnCount, threadCount);
	std::vector<Photon> photons;
	photons.reserve(scene->photonMap()->photonCount());
	for(size_t i = 0 ;i < scene->photonMap()->photonCount(); i++) 
		photons.push_back(scene->photonMap()->photon(i));
	
	for(size_t i = 0;  i < m_hit_points.size(); i++) 
		m_hit_points[i].flux = Vec3f(0);
	std::vector<std::thread> threads;
	size_t max_iteration = 0;
	for(size_t i = 0; i < scene->tiles_count(); i++) {
		Tile* tile = static_cast<Tile*>(*(scene->tiles() + i));
		max_iteration = std::max(max_iteration, tile->similarTilesCount());
	}
	
	size_t iteration_counter = 0;
	 while(true) {
		debug("ITERATION %d OUT OF %d \n", iteration_counter + 1, max_iteration + 1);
		bool go_on = false;
		 scene->photonMap()->clearPhotons();
		 for(size_t i = 0; i < scene->tiles_count(); i++) {
			Tile* tile = static_cast<Tile*>(*(scene->tiles() + i));
			if(tile->similarTilesCount() <= iteration_counter) continue;

			go_on = true;
			Tile::SimilarTile sm_tile = tile->similarTile(iteration_counter);
			Tile* t = std::get<0>(sm_tile);
			double weight = std::get<1>(sm_tile);
			const Mat4f& mat  = std::get<2>(sm_tile);
			const Photon* photons = t->photons();
			for(size_t p = 0 ; p < t->photonCount(); p++) {
				Photon ph(photons[p]);
				ph.setPostition( Vec3f(tile->transform() * mat * Vec4f(ph.postition(),1)));
				ph.setDirection( Vec3f(tile->transform() *  mat * Vec4f(ph.direction(),0)));				
				ph.setColor(ph.color() * (float)weight);
				scene->photonMap()->insertPhoton(ph);
			}		
		 }
		 iteration_counter++;
		 if(!go_on) break;
		 scene->buildPhotonMap();
		int inc = (m_hit_points.size() + threadCount - 1) / threadCount;			
		for(size_t from  = 0; from < m_hit_points.size(); from+=inc)
			threads.push_back(std::thread(&Raytracer::filtered_pass, this, scene, m_hit_points.data() + from,
														std::min<size_t>(m_hit_points.size() - from - 1, inc), 
														nnCount, iteration_counter));
		for(size_t j  = 0; j < threads.size(); j++) {
			if(threads[j].joinable())
				threads[j].join();

			m_framebuffer.save_framebuffer("results/iteration_" + std::to_string(iteration_counter) + ".bmp");
		}
		threads.clear();
	 }
	 scene->photonMap()->clearPhotons();
	for(size_t i = 0 ;i < photons.size(); i++) 
		scene->photonMap()->insertPhoton(photons[i]);
		//photons.push_back(scene->photonMap()->photon(i));

	scene->photonMap()->rebuildKdTree();
}
void Raytracer::filtered_pass(Scene* scene, hit_point* hit_points, size_t count, size_t nnCount, int iteration) {
	
	for(size_t i = 0; i < count; i++) {
		double radius;
		hit_point& hp = hit_points[i];
		if(hit_points[i].radius == -1) continue;

		Vec3f val  = scene->photonMap()->photonDensity(hp.position,hp.normal, nnCount,radius, Vec3f(1), false);
		hp.flux += val /(float)(radius * PI );
		Vec3f flux(hp.flux.x / (double)(PhotonMap::globalPhotonCount),
							hp.flux.y / (double)(PhotonMap::globalPhotonCount),
								hp.flux.z / (double)(PhotonMap::globalPhotonCount));

		m_framebuffer.setPixel(hp.pixel.x, hp.pixel.y, hp.color * flux);
	}
}



///Raytrayces a portion of the screen - the params "from" and "to" indicate 
// the screen rows where raystracing should be preformed
void Raytracer::raytrace(Scene* const scene, renderer render, size_t from, size_t to) {

	if(scene == nullptr) return;

	Camera* cam = scene->getMainCamera();
	Light*	light = scene->getLight(0);
	Vec3f l_pos = Vec3f(light->tranformation() * Vec4f(0,0,0,1));
	float inv_resX  = 1.0f /(float)m_resolution.x;
	float inv_resY  = 1.0f /(float)m_resolution.y;
	Mat4f inv_proj = glm::inverse(cam->projection());
	Vec3f dir(0), norm(0);
	Vec4f origin(0);
	Vec3f l_dir, intersection;
	IFace* res_face;
	float t = 0;
	IFace* res_face2;
	size_t i = from * m_resolution.x;
	for(size_t y = from; y < to; y++) {	
		for(int x = 0; x < m_resolution.x; x++) {
			
			//m_timer.start(m_otherStage);
			origin.x = 2 * x * inv_resX  - 1.0f;
			origin.y = 2 * y * inv_resY  - 1.0f;
			origin.z = 0;
			origin.w = 1;
			origin = origin * inv_proj; 
			origin/=origin.w;		
			dir = Vec3f(origin);
			dir = glm::normalize(Vec3f(cam->tranformation() * Vec4f(origin.x, origin.y, origin.z,0) )); 
			origin = cam->tranformation() * origin;
			t = scene->traceRay(Vec3f(origin), dir, &res_face); 
			if(t < FLT_MAX) {		
				intersection = (Vec3f(origin) + dir * t);				
				l_dir = glm::normalize(l_pos - intersection);
				float light_t = ((l_pos - intersection) / l_dir).y;				
				t = scene->traceRay(intersection, l_dir, &res_face2);
				m_framebuffer.setPixel(x,y,render(x, y, intersection, t > light_t ? l_dir : Vec3f(0), res_face));
			}
		}
		std::lock_guard<std::mutex> lg(m_mutex);
		m_rendered_row++;
	}

	std::lock_guard<std::mutex> lg(m_mutex);
	m_workingThreads--;
}

void Raytracer::start_photon_shoot(uint count, Scene* const scene, size_t threadCount, bool clear) {

	size_t inc = (count + threadCount - 1) / threadCount;

	if(clear) {
		PhotonMap::globalPhotonCount = 0;
		scene->clearPhotons();
		//scene->photonMap()->reserve_data(count * 10);
	}
	PhotonMap::globalPhotonCount += count;
	
	debug("\nphoton shoot begin\n");
	m_photonCount = 0;
	m_workingThreads = threadCount;
	for(size_t i  = 0; i < threadCount; i++) {
		std::thread t(&Raytracer::shootPhotons, this, inc, scene);
		t.detach();
	}

	
	while(is_working()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		size_t percentage = (m_photonCount / (float)count) * 100;
		debug("shooting photons: [ %d%%]\n",percentage);
	}

	debug("\nphoton shoot end\n");

}

void Raytracer::shootPhotons(uint count, Scene* const scene) {

	Light*	light = scene->getLight(0);
	Color3f light_color =light->color();
	Vec3f l_pos = light->position();
	Vec3f normal, dir, origin = l_pos; IFace* face;

	
	for(uint  i = 0; i < count; i++) {
		
		light->random_dir(origin, dir);
		Photon photon(origin, dir, light_color);

		do {

			float t = scene->traceRay(photon.postition(), photon.direction(), &face);
			if(!(t < FLT_MAX))	 break;
			photon.setPostition(photon.postition() + (photon.direction() * t));
			normal = face->worldNormal();
			scene->photonMap()->insertPhoton(Photon(photon));
			
			if(face->type() == TILE) {					
				Tile* tile = static_cast<Tile*>(face);
				tile->insertPhoton(photon);
			}		
			Vec3f color = face->color();
			float prob = glm::compMax(Vec3f(color.x * photon.color().x, color.y * photon.color().y, color.z * photon.color().z )) /   glm::compMax(color);
			color /= prob;
			
			if(dist01(random_engine) >prob) break;
			
			photon.setColor(Vec3f(photon.color().x * color.x, photon.color().y * color.y, photon.color().z * color.z));
			photon.setDirection(sampleCossHemi(normal));				
		} while(true);
		std::lock_guard<std::mutex> lg(m_mutex);
			m_photonCount++;
	}

	std::lock_guard<std::mutex> lg(m_mutex);
	m_workingThreads--;
}