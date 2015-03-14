#pragma once
#include<memory>
#include<vector>
#include"Math.hpp" 
struct parameters {
public:
	size_t	photon_count;
	size_t	density_photon_count;
	float	alpha;
	size_t	knn_count;
	float	max_distance;
	size_t	tile_scale;
	size_t	resolution;
	double	sigma;
	std::string file_suffix;
	std::string path;
	std::string scene;

};


class iexperiment {

public:
	virtual void execute() = 0;

	virtual bool success() =  0;

protected:

	void save_image(const std::vector<float>& img, const Vec2i& size, const std::string& filename);

	double compute_mse(const std::vector<float>& ground_truth,const std::vector<float>& image);


};
