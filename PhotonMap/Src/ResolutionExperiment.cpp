#include<cimg\CImg.h>

#include"Definitions.hpp"
#include"ResolutionExperiment.hpp"
#include"Raytracer.hpp"
#include"Scene.hpp"
#include"EnhancedFilter.hpp"

#include<string>
#include<fstream>

resolution_experiment::resolution_experiment(const parameters& params, Raytracer* raytracer, Scene* scene, size_t max_res, size_t thread_count):
 m_params(params), m_raytracer(raytracer), m_scene(scene), m_thread_count(thread_count), m_max_res(max_res) {

}

void resolution_experiment::execute() {
	
	if(m_params.path.length() > 0)
		if(CreateDirectory(m_params.path.c_str(),NULL) == ERROR_PATH_NOT_FOUND)
			return;

	m_scene->LoadFromFile(m_params.scene,true,m_params.tile_scale);
	m_scene->getMainCamera()->setAspect(m_raytracer->framebuffer().size().x  / (float)m_raytracer->framebuffer().size().y);

	std::ofstream resolution_mse(m_params.path + "/resolution_mse_"+ m_params.file_suffix + ".txt");
	if(!resolution_mse.good()) return;

	m_raytracer->start_photon_shoot(m_params.photon_count, m_scene, m_thread_count);
	m_scene->buildPhotonMap();
	m_raytracer->start_density(m_params.density_photon_count,m_scene,false,m_thread_count);

	const float* img = m_raytracer->framebuffer().getPtr();
	std::vector<float> ground_truth(img, img + m_raytracer->framebuffer().size().x * m_raytracer->framebuffer().size().y * 3);
	m_raytracer->framebuffer().save_framebuffer(m_params.path +  "/ground_truth"+ m_params.file_suffix +".bmp");

	for(size_t res = 2; res < m_max_res; ++res) {

		m_scene->filter()->createIndex(m_scene->tiles(), m_scene->tiles_count(), 
			m_params.density_photon_count, -1, 1.0f, res, RES_STRATEGY_MIN);
		m_scene->set_textures();
		m_raytracer->start_raytracing(m_scene,false, true, m_thread_count);
		
		std::vector<float> image(m_raytracer->framebuffer().getPtr(), 
			m_raytracer->framebuffer().getPtr() + m_raytracer->framebuffer().size().x * m_raytracer->framebuffer().size().y * 3);

		double mse = compute_mse(ground_truth, image);

		m_raytracer->framebuffer().save_framebuffer(
			m_params.path +  "/resolution_" + std::to_string(res) + "_" + m_params.file_suffix +".bmp");

		resolution_mse<<"resolution " + std::to_string(res)<<"  "<<mse<<std::endl;
	}
	bool m_success = true;
}