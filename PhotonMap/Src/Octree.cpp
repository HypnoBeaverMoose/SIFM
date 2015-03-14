#include"Definitions.hpp"
#include "Math.hpp"
#include "Octree.hpp"


Octree::Octree(uint maxLeafSize, uint MaxDepth) :
	m_maxLeafSize(maxLeafSize), m_maxDepth(MaxDepth), m_avgDepth(0), m_avgLeafSize(0), m_leafNodes(0) {
}

void Octree::loadData(IFace** data, uint count) {
	m_rootNode.children.clear();
	m_rootNode.faces.clear();
	m_rootNode.aabb.min = Vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
	m_rootNode.aabb.max = Vec3f(FLT_MIN, FLT_MIN, FLT_MIN);

	for(uint i = 0; i < count; i++) {
		for(uint pos = 0; pos < 3; pos++) {
			m_rootNode.aabb.min = glm::min(m_rootNode.aabb.min, data[i]->worldPosition(pos));
			m_rootNode.aabb.max = glm::max(m_rootNode.aabb.max,  data[i]->worldPosition(pos));
		}
	}
	m_rootNode.aabb.min +=0.1f * m_rootNode.aabb.min;
	m_rootNode.aabb.max +=0.1f * m_rootNode.aabb.max;
	m_rootNode.aabb.min.x = glm::compMin(m_rootNode.aabb.min);
	m_rootNode.aabb.min.y = glm::compMin(m_rootNode.aabb.min);
	m_rootNode.aabb.min.z = glm::compMin(m_rootNode.aabb.min);

	m_rootNode.aabb.max.x = glm::compMax(m_rootNode.aabb.max);
	m_rootNode.aabb.max.y = glm::compMax(m_rootNode.aabb.max);
	m_rootNode.aabb.max.z = glm::compMax(m_rootNode.aabb.max);
	std::vector<IFace*> face_ptrs;
	for(uint i = 0; i < count; i++) {
		face_ptrs.push_back(data[i]);
	}
	splitNode(m_rootNode, face_ptrs.data(), count, 0);
		m_avgDepth/=m_leafNodes;
		m_avgLeafSize/=m_leafNodes;
	debug("\n---------------------------------------------\n"
			"Octree creation finished. Average depth achieved is %f, Average leaf size is %f", m_avgDepth, m_avgLeafSize);
}

void Octree::splitNode(Octree::Node& node, IFace**  data, uint count, size_t depth) {

	if(count < m_maxLeafSize || depth == m_maxDepth) {
		for(uint i = 0; i < count; i++) {
			node.faces.push_back(data[i]);
		}

		m_avgDepth +=depth;
		m_avgLeafSize += count;
		m_leafNodes++;
		node.split = node.aabb.min;
		return;
	}

	node.split = (node.aabb.max + node.aabb.min) * 0.5f;
	std::vector<IFace*> faces;
	for(int i = 0; i < 8; i++) {
		faces.clear();
		node.children.push_back(Node());
		Node& child = node.children.back();
		generateSplitPointAABB(node.split,node.aabb, child.aabb, i);
		for(uint face = 0; face < count; face++) {
			bool contains = false;	
			for(uint point = 0; point < data[face]->size(); point++) {
				contains|=child.aabb.contains(data[face]->worldPosition(point));
			}

			if(contains)
				faces.push_back(data[face]);
		}
		if(faces.size() > 0)
			splitNode(child,faces.data(),faces.size(), depth + 1);
		faces.clear();
	}
}

void Octree::generateSplitPointAABB(const Vec3f splitPoint, const AABB& prentAABB, AABB& childAABB, uint index) {
	
	float minZpos =  prentAABB.min.z, maxZpos = splitPoint.z;
	if(index > 3)  {
		index-=4;
		minZpos = splitPoint.z;
		maxZpos = prentAABB.max.z;
	}

	switch(index) {
	case 0:
		childAABB.min = Vec3f(prentAABB.min.x, prentAABB.min.y, minZpos);
		childAABB.max = Vec3f(splitPoint.x, splitPoint.y, maxZpos);
		break;
	case 1:
		childAABB.min = Vec3f(prentAABB.min.x, splitPoint.y, minZpos);
		childAABB.max = Vec3f(splitPoint.x, prentAABB.max.y,maxZpos);
	break;
	case 2:
		childAABB.min = Vec3f(splitPoint.x, splitPoint.y, minZpos);
		childAABB.max = Vec3f(prentAABB.max.x, prentAABB.max.y,maxZpos);
	break;
	case 3:
		childAABB.min = Vec3f(splitPoint.x, prentAABB.min.y,minZpos);
		childAABB.max = Vec3f(prentAABB.max.x, splitPoint.y,maxZpos);
	break;

	}
}
void Octree::draw(uint child) {

	drawNode(&m_rootNode, child);
}

void Octree::drawNode(Octree::Node* node, uint child) {
	
	node->aabb.draw();
	//if(node->children.size() > 0)
	//	drawNode(&(node->children[child]), child);
	for(uint i = 0; i < node->children.size(); i++) {
		drawNode(&(node->children[i]), child);
	}
}
 
void Octree::traverseTree(Node& node, const Vec3f& origin, const Vec3f& dir, IFace** face, float& current_t) {
		
	if(node.faces.size() == 0 && node.isLeaf()) return;
	float t  = current_t;
	bool intersect = node.aabb.intersect(origin, dir, t);
	if(intersect && t <= current_t) {
		if(node.isLeaf()) {
			for(uint i = 0 ; i < node.faces.size(); i++) {
				if(node.faces[i]->intersectWorld(origin, dir, t)) {
					if(t < current_t) {
						current_t = t;
						*face = node.faces[i];
					}
				}
		
			}
		}
		else {
			for(uint i = 0; i < node.children.size(); i++) {
				float t = current_t;
				IFace* out_face;
				traverseTree(node.children[i],origin, dir, &out_face, t);
				if(t < current_t) {
					current_t = t;
					*face = out_face;
				}				
			}
		
		}

	}
}


void Octree::clean() {

	m_rootNode.children.clear();
	m_rootNode.faces.clear();
}
float Octree::traceRay(const Vec3f& origin, const Vec3f& dir, IFace** face) {

	float t = FLT_MAX;
	traverseTree(m_rootNode, origin, dir, face,t);

	return t;
}