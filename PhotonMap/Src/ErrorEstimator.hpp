#pragma once
#include<vector>
#include<string>

class ErrorEstimator {
public:

 void load_groundTruth(const std::string& filename);

 double computeMSE(const std::vector<float>& image);


private:
	std::vector<float> m_groundTruth;

};