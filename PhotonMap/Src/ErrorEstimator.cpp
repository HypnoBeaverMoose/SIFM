#include<cimg\CImg.h>
#include"Definitions.hpp"
#include"Math.hpp"
#include"ErrorEstimator.hpp"



void ErrorEstimator::load_groundTruth(const std::string& filename) {
	

	cimg_library::CImg<byte> image(filename.c_str());

	for(int y = 0 ; y < image.height(); y++) {
		for(int x = 0 ; x < image.width(); x++) {
				m_groundTruth.push_back(image.data()[ y * image.width() * image.depth() + x* image.depth()] /(float)std::numeric_limits<byte>::max());
				m_groundTruth.push_back(image.data()[ y * image.width() * image.depth() + x* image.depth() + 1] / (float)std::numeric_limits<byte>::max());
				m_groundTruth.push_back(image.data()[ y * image.width() * image.depth() + x* image.depth() + 2] / (float)std::numeric_limits<byte>::max());
		}
	}

	std::vector<byte> img;
	for(int y = 0 ; y < image.height(); y++) {
		for(int x = 0 ; x < image.width(); x++) {
			img.push_back((byte)(glm::min(255.0f, m_groundTruth[y * image.width()* image.depth() + x* image.depth()])));
			img.push_back((byte)(glm::min(255.0f, m_groundTruth[y * image.width()* image.depth() + x* image.depth() + 1])));
			img.push_back((byte)(glm::min(255.0f, m_groundTruth[y * image.width()* image.depth() + x* image.depth() + 2])));
			
		}
	}

	cimg_library::CImg<byte> image2(img.data(),image.width(), image.height(), 1, 1, false);
	image.save("results/test.bmp");
}

double ErrorEstimator::computeMSE(const std::vector<float>& image) {

	if(image.size() != m_groundTruth.size()) return -1;

	double mse;
	for(size_t i = 0 ; i < image.size(); i++) {
		mse+= ((double)m_groundTruth[i] - (double)image[i]) * ((double)m_groundTruth[i] - (double)image[i]);
	}
	return mse / image.size();
 }