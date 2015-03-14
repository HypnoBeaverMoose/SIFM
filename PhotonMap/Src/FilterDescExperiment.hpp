#pragma once
#include"Experiment.hpp"


class Scene;
class Raytracer;

class filter_desc_experiment : public iexperiment {

public:
	filter_desc_experiment(const parameters& params, Raytracer* raytracer,  Scene* scene, 
											const std::vector<float> ground_truth, size_t thread_count);

	virtual void execute();

	virtual bool success() { return m_success; };

private:
	parameters m_params;
	bool m_success;
	Scene* m_scene;
	Raytracer* m_raytracer;
	size_t m_thread_count;
	std::vector<float> m_ground_truth;
};