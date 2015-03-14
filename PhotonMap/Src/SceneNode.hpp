#pragma once
#include"instance_count_base.hpp"
#include<vector>
class Entity;

class SceneNode { //: public istance_count_base {

public:
	SceneNode() : m_entity(nullptr), m_transformation(1) {}

	void setParent(SceneNode* const node);

	void addChild( SceneNode* const node) {	m_children.push_back(std::auto_ptr<SceneNode>(node)); }

	void removeChild(uint index) { m_children.erase(m_children.begin() + index); }

	SceneNode* const getChild(uint index) { return m_children[index].get(); }

	uint getChildrenCount() { 	return m_children.size(); }
	
	void setTransform(const Mat4f& transofrm) {	m_transformation = transofrm; }

	Entity* getEntity() { return m_entity.get(); }

	void setEntity(Entity* ent) { m_entity.reset(ent); }

	const Mat4f&  transform() { return m_transformation; }

	bool isEmpty() { 	return m_entity.get() == nullptr; }

private:
	Mat4f									m_transformation;	
	std::vector<std::auto_ptr<SceneNode>>	m_children;
	std::auto_ptr<Entity>					m_entity;
};