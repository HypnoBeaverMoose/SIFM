#include<cimg\CImg.h>
#include"Definitions.hpp"
#include"PhotonMap.hpp"
#include"Math.hpp"
#include "Experiment.hpp"


void iexperiment::save_image(const std::vector<float>& img, const Vec2i& size, const std::string& filename) {


	std::vector<byte> byte_img;
	
	const float* ptr = img.data();
	for(size_t channel  = 0 ; channel < 3; channel++) {
		for(int y = size.y - 1; y >= 0; y--) {
			for(int x = 0; x < size.x; x++) {
				byte_img.push_back((byte)(glm::min(255.0f, ptr[y * size.x * 3 + x * 3 +channel ] *PhotonMap::globalExposure * 255)));
			}
		}
	}
	cimg_library::CImg<byte> image(byte_img.data(), size.x, size.y, 1, 3, false);
	image.save(filename.c_str());
}

double iexperiment::compute_mse(const std::vector<float>& ground_truth,const std::vector<float>& image) {

	double mse = 0;

	for(size_t i = 0 ; i < ground_truth.size(); i++) {
		mse += (ground_truth[i] - image[i]) * (ground_truth[i] - image[i]);
	}

	return mse /(double) ground_truth.size();
}
