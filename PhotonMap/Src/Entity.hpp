#pragma once
#include"Definitions.hpp"
#include "SceneNode.hpp"
class SceneNode;
class Entity {

public:
	enum Type { Mesh = 0, Camera, Light};

	Entity(const char* name, Type type , SceneNode* node = 0) : m_name(name), m_node(node) {} 

	virtual void setName(const char* name) { m_name.assign(name); }

	virtual const std::string& getName(){ return m_name; }

	Type type() { return m_type; }

	virtual void draw() = 0;
	
	virtual const Mat4f& tranformation() = 0;

	void setSceneNode(SceneNode* node) { m_node = node; }
	
	SceneNode* scene_node() { return m_node; }

	virtual ~Entity() {}

protected:
	SceneNode*	m_node;
	std::string m_name;
	Type m_type;
};
