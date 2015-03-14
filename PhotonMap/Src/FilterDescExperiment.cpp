#pragma once
#include"Definitions.hpp"
#include"FilterDescExperiment.hpp"
#include"Raytracer.hpp"
#include"Scene.hpp"
#include"EnhancedFilter.hpp"

#include<string>
#include<fstream>
filter_desc_experiment::filter_desc_experiment(const parameters& params, Raytracer* raytracer,  Scene* scene, 
														const std::vector<float> ground_truth, size_t thread_count) : 
			m_params(params), m_raytracer(raytracer), m_scene(scene), m_thread_count(thread_count), 
				m_ground_truth(ground_truth.begin(), ground_truth.end()) {

}

void filter_desc_experiment::execute() {
	
	if(m_params.path.length() > 0)
		if(CreateDirectory(m_params.path.c_str(),NULL) == ERROR_PATH_NOT_FOUND)
			return;

	std::ofstream mse(m_params.path + "/mse_" + ".txt",std::ios::out | std::ios::app);
	mse<<m_params.file_suffix<<std::endl;

	m_scene->clean_scene();
	m_scene->LoadFromFile(m_params.scene,true,m_params.tile_scale);
	m_scene->getMainCamera()->setAspect(m_raytracer->framebuffer().size().x  / (float)m_raytracer->framebuffer().size().y);
	
	m_raytracer->start_photon_shoot(m_params.photon_count, m_scene, m_thread_count);
	m_scene->buildPhotonMap();

	m_scene->filter()->createIndex(m_scene->tiles(), m_scene->tiles_count(), 
		m_params.density_photon_count, -1,1.0f, m_params.resolution);

	m_scene->set_textures();
	m_raytracer->start_raytracing(m_scene,false, true, m_thread_count);

	std::vector<float> original(m_raytracer->framebuffer().getPtr(), 
			m_raytracer->framebuffer().getPtr() + m_raytracer->framebuffer().size().x * m_raytracer->framebuffer().size().y * 3);
	
	m_raytracer->framebuffer().save_framebuffer(m_params.path +  "/" + m_params.file_suffix + "_original" + ".bmp");


	m_scene->filter()->filterTiles(m_scene->tiles(), m_scene->tiles_count(),true, m_params.knn_count,m_params.sigma,true);
	m_scene->set_textures();
	m_raytracer->start_raytracing(m_scene,false, true, m_thread_count);
	std::vector<float> filtered(m_raytracer->framebuffer().getPtr(), 
			m_raytracer->framebuffer().getPtr() + m_raytracer->framebuffer().size().x * m_raytracer->framebuffer().size().y * 3);

	m_raytracer->framebuffer().save_framebuffer(m_params.path +  "/" + m_params.file_suffix + "_filtered" +".bmp");

	double mse_original = compute_mse(original,m_ground_truth);
	double mse_filtered = compute_mse(filtered,m_ground_truth);

	mse<<"density/original "<<mse_original<<" density/filtered "<<mse_filtered
		<<" original/filtered "<<compute_mse(original,filtered)<<" ratio "<<mse_filtered / mse_original<<std::endl;

	m_success = true;
}
