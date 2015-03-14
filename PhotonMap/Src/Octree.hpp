#pragma once
#include"Objects.hpp"
#include "SceneNode.hpp"
#include"Mesh.hpp" 

#include<vector>

class Octree {

struct Node {
	std::vector<Node> children;
	std::vector<IFace*> faces;
	bool isLeaf() { return children.size() == 0; }
	Vec3f split;
	AABB	aabb;
};

public:

	Octree(uint maxLeafSize, uint MaxDepth);

	void loadData(IFace** data, uint count);

	float traceRay(const Vec3f& origin, const Vec3f& dir, IFace** face);

	void draw(uint child);
	
	void clean();

private:

	void splitNode(Node& node, IFace** data, uint count, size_t depth);

	void generateSplitPointAABB(const Vec3f splitPoint, const AABB& prentAABB, AABB& childAABB, uint index);

	void drawNode(Node* node, uint child);

	void traverseTree(Node& node, const Vec3f& origin, const Vec3f& dir, IFace** face, float& current_t);

private:
	Node	m_rootNode;
	uint	m_maxLeafSize;
	uint	m_maxDepth;
	float	m_avgDepth;
	float	m_avgLeafSize;
	size_t	m_leafNodes;
};