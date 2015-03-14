#include<cimg\CImg.h>
#include"Definitions.hpp"
#include"Objects.hpp"
#include"Math.hpp"
#include"PhotonMap.hpp"
#include"Tile.hpp"
#include"EnhancedFilter.hpp"
#include"DensityDescriptor.hpp"
#include<opencv2\opencv.hpp>
#include<fstream>
#include<algorithm>

std::vector<std::vector<float>> EnhancedFilter::textures;
std::vector<std::vector<float>> EnhancedFilter::filtered_textures;

EnhancedFilter::EnhancedFilter(PhotonMap* const map)
	:m_map(map), m_tree(0), m_mininum_neighbours(0) {

	m_timer.register_stage(m_descriptorGenStage);
	m_timer.register_stage(m_rotateDescriptorStage);
	m_timer.register_stage(m_buildIndexStage);
	m_timer.register_stage(m_searchStage);
	m_timer.register_stage(m_weigthCalculationStage);
	m_timer.register_stage(m_filterStage);
	m_timer.register_stage(m_transferDataStage);
}

float EnhancedFilter::createIndex(IFace** tiles, size_t size, size_t density_photons, int max_dimenstions,
								 float pca_fractions, size_t resolution,  RESOLUTION_STRATEGY strategy) {

	static Mat3f transforms[8] = { Mat3f(1), Mat3f(-1,0,0, 0,1,0, 0,0,1), Mat3f(1,0,0, 0,-1,0, 0,0,1), Mat3f(-1,0,0, 0,-1,0, 0,0,1),
								 Mat3f(0,-1,0, -1,0,0, 0,0,1), Mat3f(0,1,0, -1,0,0, 0,0,1), Mat3f(0,-1,0, 1,0,0, 0,0,1),
								 Mat3f(0,1,0, 1,0,0, 0,0,1)};
	
	static const int transf = 8;
	//size_t res = ((resolution == 0) ? determine_resolution(tiles, size, strategy) : resolution);
	//debug("max resolution: %i\n", res);
	m_prototype.reset(new DensityDescriptor(resolution, m_map));

	textures.clear();
	filtered_textures.clear();
	m_descriptors.clear();
	m_instances.clear();
	m_instances.reserve(size * transf);
	m_timer.reset();
	
	m_timer.start(m_descriptorGenStage);
	cv::Mat mat;
	mat.create(size* transf, m_prototype->value_size(), CV_32F);	
	for(size_t i = 0; i < size; i++) {
		m_timer.set_stage(m_descriptorGenStage);
		Tile* tile = static_cast<Tile*>(tiles[i]);	
		m_prototype->rebuildDescriptor(tile, density_photons);
		
		m_timer.set_stage(m_rotateDescriptorStage);

		for(size_t t = 0; t <transf; t++) {			
			
			m_instances.emplace_back(i,Mat4f(transforms[t]));
			Descriptor& instance = m_instances.back();
			m_prototype->value(transforms[t], instance.value);
			//m_prototype->variance(transforms[t],instance.variance);
			for(size_t j = 0; j < m_prototype->value_size(); j++) {
				mat.at<float>(i * transf + t,j) = instance.value[j];
			}

			if(t == 0) {
				m_descriptors.push_back(&m_instances.back());
				textures.push_back(instance.value);
			}
		}
	}
	size_t max_components = m_descriptors[0]->value.size();
	std::vector<float> max_values(max_components, 0);
	for(size_t i = 0; i < m_descriptors.size(); i++) {
		const std::vector<float>& val =  m_descriptors[i]->value;
		for(size_t j = 0; j < val.size(); j++) {
			max_values[j] = glm::max(max_values[j], std::abs(val[j]));
		}
	}
	*std::max_element(max_values.begin(), max_values.end());	
	cv::PCA cv_pca, cv_points_pca;
	
	if(max_dimenstions == -1) {
		cv_pca.computeVar(mat, m_mean,CV_PCA_DATA_AS_ROW,pca_fractions);
	}else
		cv_pca(mat,m_mean,CV_PCA_DATA_AS_ROW,3);

	//m_eigenvalues = cv_pca.eigenvalues;
	////size_t max_components = m_eigenvalues.size();
	//debug("\nDimensions after PCA: %i\n", max_components);
	//cv_points_pca(mat, cv::Mat(), CV_PCA_DATA_AS_ROW, 3);

	////Vec3f max(0);
	//
	//std::vector<float> eigenvalues2 = cv_points_pca.eigenvalues;
	//for(size_t i = 0; i < m_instances.size(); i++) {
	//	Descriptor& desc = m_instances[i];
	//	std::vector<float> res, res2;
	//	cv_pca.project(desc.value, res);
	//	cv_points_pca.project(desc.value, res2);
	//	//desc.value.assign(res.begin(), res.end());		
	//	res.clear();	
	//	m_points.push_back(Vec3f(res2[0] , 
	//							res2[1], 
	//							0));
	//}
	//Vec3f min(10), max(-1000);
	//for(size_t i = 0; i < m_points.size(); i++){
	//	min.x = std::min(min.x, m_points[i].x);
	//	min.y = std::min(min.y, m_points[i].y);
	////	min.z = std::min(min.z, m_points[i].z);
	//	max.x = std::max(max.x, m_points[i].x);
	//	max.y = std::max(max.y, m_points[i].y);
	////	max.z = std::max(max.z, m_points[i].z);



	//}	
	//for(size_t i = 0; i < m_points.size(); i++) {
	//	m_points[i].x= (m_points[i].x - min.x) / (max.x - min.x);
	//	m_points[i].y= (m_points[i].y - min.y) / (max.x - min.y);
	//	//m_points[i].z= (m_points[i].z - min.z) / (max.z - min.z);
	//}

	//std::ofstream data("data.csv");
	//for(size_t i = 0; i < m_instances.size(); i++)  {
	//	//m_points[i]*=0.5f;
	//	//m_points[i] += Vec3f(0.5f, 0.5f, 0.5f);
	//	Tile* tile = static_cast<Tile*>(tiles[m_instances[i].index]);
	//	tile->mesh()->set_color(m_points[i]);
	//	//debug("color: %f %f %f\n", m_points[i].r, m_points[i].g, m_points[i].b);
	//	data<<m_points[i].x<<", "<<m_points[i].y<<", "<<m_points[i].z<<std::endl;
	//}
	m_timer.start(m_buildIndexStage);
	m_tree.reset(new KdTree(max_components, *this, nanoflann::KDTreeSingleIndexAdaptorParams(1)));
	m_tree->buildIndex();
	m_timer.stop();	
	return 0;
}

std::pair<float, float> EnhancedFilter::filterTiles(IFace** tiles, size_t size, bool useKnn, float search_param, float sigma, bool filter_textures)  {

	m_mininum_neighbours  = std::numeric_limits<size_t>().max();
	std::vector<std::vector<Photon>> maps(size, std::vector<Photon>());
	double one_over_sigma = 1.0 /  (sigma);
	debug("one over sigma: %f \n",one_over_sigma);
	double avgMaxWeight = 0;
	float avgResults = 0;
	size_t count = 0;
	m_timer.start(m_searchStage);

	if(filter_textures)
		filtered_textures.assign(textures.size(),std::vector<float>(m_prototype->value_size(),0));


	std::vector<size_t> nearestIndices;
	std::vector<float> nnDistances;
	
	for(size_t  i = 0 ; i < size; i++) {

		m_timer.set_stage(m_searchStage);
		m_distances.clear();
		const float* vec = m_descriptors[i]->value.data();
		count = (size_t)search_param;
		while(true) {
			if(useKnn) {				
				nearestIndices.assign(count,0);
				nnDistances.assign(count,0);				
				m_tree->knnSearch(vec, count,nearestIndices.data(),nnDistances.data());
				for(size_t index = 0; index < nearestIndices.size(); index++) 
					m_distances.emplace_back(nearestIndices[index], nnDistances[index]);
			}
			else
				m_tree->radiusSearch(vec, search_param, m_distances,  nanoflann::SearchParams());

			if(m_distances.size() ==  0)
				debug("something is wrong");
				
		//	debug("distances size %i'\n", m_distances.size());

			std::sort(m_distances.begin(), m_distances.end(), [this](const instance& i, const instance& j) 
			{ return this->m_instances[i.first].index < this->m_instances[j.first].index;});

			std::vector<std::pair<size_t, float>>::iterator end  = 
			std::unique(m_distances.begin(), m_distances.end(), [this](const instance& i, const instance& j) 
			{ return this->m_instances[i.first].index == this->m_instances[j.first].index;});
		
			bool big_enough = std::distance(m_distances.begin(),end) >=search_param;
			if(useKnn && big_enough)
				m_distances.resize((size_t)search_param);
			
			if(!useKnn ||(useKnn &&  big_enough)) {
					std::sort(m_distances.begin(), m_distances.end(), [](const instance& i, const instance& j) 
					{ return i.second < j.second; });
					break;			
			}

			if(useKnn && !big_enough){
				count*=2;
				nearestIndices.assign(count,0);
				nnDistances.assign(count,0);
			}
		}
		double sum = 0;
		m_timer.set_stage(m_weigthCalculationStage);
		
		one_over_sigma = sigma == -1 ? m_distances.back().second == 0 ? 1.0f : ( 1.0f / ((m_distances.back().second)) ) : 
			1.0 /  sigma;
		
		std::vector<std::pair<size_t, double>> dist;
		for(size_t k = 0; k < m_distances.size(); k++){	
			dist.push_back(std::make_pair(m_distances[k].first, std::exp( - m_distances[k].second * one_over_sigma)));
			sum += dist.back().second;
		}

		m_mininum_neighbours = glm::min(m_mininum_neighbours, m_distances.size());
		avgResults+=m_distances.size();
		double maxWeight = 0;
		Tile* tile = static_cast<Tile*>(tiles[i]);	
		//save_descriptors(i,m_distances);	
		tile->clear_similar_tiles();
		//tile->mesh()->set_color(m_points[i]);
		for(size_t k = 0; k < m_distances.size(); k++){
			double weight = dist[k].second >  0  ? dist[k].second / sum : dist[k].second;
			//debug("weight: %f\n",(float)weight);
			maxWeight = glm::max(maxWeight, weight);
			size_t index = m_instances[m_distances[k].first].index;
			Tile* t = static_cast<Tile*>(tiles[index]);	
			tile->addSimilarTile(t, weight, m_instances[m_distances[k].first].transform);
			if(filter_textures)
				filter_texture(i, index,  (float)weight, m_instances[m_distances[k].first].transform);
		
		}
		avgMaxWeight+=maxWeight;
	}
	if(filter_textures)
		textures.assign(filtered_textures.begin(), filtered_textures.end());

	debug("\n--------------------------------------------------------\n AVERAGE MAX WEIGHT: %f \n AVERAGE RESULTS FOUND: %f",
																						avgMaxWeight / size, avgResults / (float)size);
	m_timer.stop();
	return std::make_pair(avgResults / (float)size,  avgMaxWeight/ (float)size);

	/*debug("\n--------------------------------------------------------\n" 
		"FILTER TIME: %f, out of which: \n\t %f - DESCRIPTOR GENERATION"
		 "\n\t %f - DESCRIPTOR ROTATION\n\t %f  - INDEX CREATION"
		 "\n\t %f - SEARCH \n\t %f  - FILTER" 
		 "\n\t %f - WEIGHT CALCULATION \n\t %f - DATA TRANSFER",
		 m_timer.elapsed(), m_timer.elapsed(m_descriptorGenStage), 
		 m_timer.elapsed(m_rotateDescriptorStage), m_timer.elapsed(m_buildIndexStage), 
		 m_timer.elapsed(m_searchStage), m_timer.elapsed(m_weigthCalculationStage), 
		 m_timer.elapsed(m_filterStage), m_timer.elapsed(m_transferDataStage));*/

}
void EnhancedFilter::draw_descriptors() {
	
	const float half_scale = 10;
	glBegin(GL_LINE_LOOP);
		glColor3fv(glm::value_ptr(Vec3f(0,0,0)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,-half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,0,0)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,-half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,0,1)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,-half_scale,half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(0,0,1)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,-half_scale,half_scale)));
	glEnd();

	glBegin(GL_LINE_LOOP);
		glColor3fv(glm::value_ptr(Vec3f(0,1,0)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,1,0)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,1,1)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,half_scale,half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(0,1,1)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,half_scale,half_scale)));
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES) ;
		glColor3fv(glm::value_ptr(Vec3f(0,1,0)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(0,0,0)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,-half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,1,0)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,0,0)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,-half_scale,-half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,1,1)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,half_scale,half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(1,0,1)));
		glVertex3fv(glm::value_ptr(Vec3f(half_scale,-half_scale,half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(0,1,1)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,half_scale,half_scale)));
		glColor3fv(glm::value_ptr(Vec3f(0,0,1)));
		glVertex3fv(glm::value_ptr(Vec3f(-half_scale,-half_scale,half_scale)));
	glEnd();

	//glPointSize(3);
	//glBegin(GL_POINTS);
	//for(size_t i = 0 ; i < m_points.size(); i++) {

	//	glColor3fv(glm::value_ptr(m_points[i]));
	//	glVertex3fv(glm::value_ptr((m_points[i] * half_scale* 2.0f - Vec3f(half_scale))));
	//}
	//glEnd();
	//glPointSize(1);
}

void EnhancedFilter::save_descriptors(size_t index, const std::vector<std::pair<size_t, float>>& unique_distances) {
	
	static std::string dir = "descriptors";
	static size_t  resolution = m_prototype->resolution();
	std::string	path =  dir + "/" + std::to_string(index);
	CreateDirectory(path.c_str(),NULL);
	
	std::vector<float> distances;
	const std::vector<float>& original_tex = textures[index];

	for(size_t i = 0 ; i < unique_distances.size(); i++) {
		
		int ind = m_instances[unique_distances[i].first].index;
		Mat3f transform = Mat3f(m_instances[unique_distances[i].first].transform);
		
		//get the texture
		const std::vector<float>& tex = textures[ind];
		std::vector<float> new_tex(tex.size(), -1);
		
		//rotate it according to the transform
		DensityDescriptor::rotate_descriptor(transform, tex.data(),new_tex.data(), resolution);
	
		//dalculate the dinstance or MSE
		float sum = 0;
		for(size_t i = 0; i < original_tex.size(); ++i) {
			float val =original_tex[i] - new_tex[i];
			sum+=val* val;
		}
		sum/=original_tex.size();
		if(!distances.empty() && (distances.back() > sum)) {
			debug("Error:: descriptor order wrong! %i and %i,  %f and %f\n", index, ind, distances.back() * 100000, sum* 100000);
		}
		distances.push_back(sum);

		//save it under the appropriate name
		std::vector<byte> img;
		for(size_t y = 0 ; y < resolution; y++) {
			for(size_t x = 0; x < resolution; x++) {
				img.push_back((byte)(glm::min(255.0f, new_tex[y * resolution * 3 + x * 3 ] * PhotonMap::globalExposure * 255)));
				img.push_back((byte)(glm::min(255.0f, new_tex[y * resolution * 3 + x * 3 + 1] * PhotonMap::globalExposure * 255)));
				img.push_back((byte)(glm::min(255.0f, new_tex[y * resolution * 3 + x * 3 + 2] * PhotonMap::globalExposure * 255)));
			}

		}

		cimg_library::CImg<byte> image(img.data(), resolution, resolution, 1, 3, false);
		image.save((path + "/" +std::to_string(i) + "_" + std::to_string(ind) + ".bmp").c_str());	
	}
	std::ofstream out(path + "/distances.txt");

	for(auto dist = distances.begin(); dist!=distances.end(); ++dist)
		out<<*dist<<std::endl;

	out.close();
}

void EnhancedFilter::filterPhotonMap(std::vector<Photon>& dst,  Tile* const src, double weight, const Mat4f& transform) {

	const Photon* photons = src->photons();
	for(size_t p = 0; p <src->photonCount(); p++) {
			Photon ph(photons[p]);	
			ph.setPostition(Vec3f(transform * Vec4f(ph.postition(),1)));
			ph.setDirection(Vec3f(transform * Vec4f(ph.direction(),0)));
			ph.setColor(ph.color() * (float)weight);
			dst.push_back(ph);
	}

}

size_t EnhancedFilter::kdtree_get_point_count() const {
	
	return m_instances.size();
}


float EnhancedFilter::kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const {
	
	const float* val = m_instances[idx_p2].value.data();
	float sum = 0;
	for(size_t i = 0; i < size; i++) {
		sum += (p1[i] - val[i]) * (p1[i] - val[i]);
	}
	return sum;
}


float EnhancedFilter::kdtree_get_pt(const size_t idx, int dim) const {
	
	return m_instances[idx].value[dim];
}

size_t EnhancedFilter::determine_resolution(IFace** tiles, size_t size, RESOLUTION_STRATEGY strategy) {

	const size_t base_res = 2;
	size_t max_resolution = base_res;
	size_t min_resolution = std::numeric_limits<size_t>().max();
	float average = 0;
	for(size_t i = 0; i < size; i++) {
		Tile* t = static_cast<Tile*>(tiles[i]);
		float photons_per_tile = std::sqrt(t->photonCount() / (float)density_photons);
		max_resolution = glm::max(max_resolution, (size_t)photons_per_tile);
		min_resolution = glm::min(min_resolution, (size_t)photons_per_tile);
		average+=photons_per_tile;
	}
	return (strategy == RES_STRATEGY_MAX) ? max_resolution : (strategy == RES_STRATEGY_MIN) ? 
		glm::max(min_resolution, base_res) : glm::max((size_t)(average / size), base_res);
}

size_t EnhancedFilter::descriptor_resolution() { 
	
	if(m_prototype.get() ==0 ) return 0;

	return m_prototype->resolution();
}


void EnhancedFilter::filter_texture(size_t dest_index,	size_t src_index, float weight, const Mat4f& transform) {


	std::vector<float>& dest_texture = filtered_textures[dest_index];
	const std::vector<float>& src_texture = textures[src_index];	
	std::vector<float> rotated_texture(src_texture.size(),-1);

	DensityDescriptor::rotate_descriptor(Mat3f(transform),src_texture.data(),rotated_texture.data(),m_prototype->resolution());

	for(size_t i = 0 ; i < dest_texture.size(); i++) {
		dest_texture[i]+= rotated_texture[i] * weight;	
	}
}



EnhancedFilter::~EnhancedFilter() {
}