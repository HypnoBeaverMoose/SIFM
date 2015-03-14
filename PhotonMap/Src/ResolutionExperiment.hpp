#pragma once
#include"Experiment.hpp"

class Raytracer;
class Scene;
class ErrorEstimator;

class resolution_experiment : public iexperiment {

public:
	resolution_experiment(const parameters& params, Raytracer* raytracer, Scene* scene, size_t max_res, size_t thread_count);

	virtual void execute();

	virtual bool success() { return m_success; }
private:
	size_t m_thread_count;
	bool m_success;
	parameters m_params;
	Raytracer* m_raytracer;
	Scene* m_scene;
	size_t m_max_res;
};