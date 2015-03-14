#pragma once

class Tile;

class IMetric {
	
public:
	virtual double distance(Tile* const left, Tile* const right, Mat4f& transform) const = 0;
};