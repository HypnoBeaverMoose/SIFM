#pragma once

class Tile;
class IMetric;
class IDescriptor;
class ITilesFilter {

public:
	virtual void createIndex(IFace** tiles, size_t size) = 0;

	virtual void filterTiles(IFace** tiles, size_t size) = 0;

	virtual double filterParam() = 0;

	virtual void setfilterParam(double) = 0;

	virtual void draw_descriptors() {};
};