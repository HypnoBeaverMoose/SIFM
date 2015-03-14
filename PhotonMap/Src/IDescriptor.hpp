#pragma once
#include<vector>

class Tile;
class PhotonMap;


class IDescriptor {

public:
	virtual void rebuildDescriptor(Tile* tile) = 0;

	virtual bool empty() const = 0;

	virtual size_t resolution() const = 0; 

	virtual size_t value_size() const  = 0;

	virtual void value(float** transformed_values) const = 0;

	virtual void value(const Mat3f& transform, float** transformed_values) const = 0;
};