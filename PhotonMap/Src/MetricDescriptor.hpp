#pragma once
#include "IMetric.hpp"
#include<map>

class DensityDescriptor;
class PhotonMap;

enum eRotations { RIdentity = 0, R90, R180, R270, RSize };
enum eMirrors { MIdentity = 0, MirrorX, MirrorY, MirrorSize };

class MetricDescriptor : public IMetric  {

public:
	MetricDescriptor(size_t resolution, PhotonMap* const map): m_resolution(resolution), m_photonMap(map) {};

	double distance(Tile* const left, Tile* const right, Mat4f& transform) const;


public:
	static std::vector<Vec3f> getTexture(Tile* tile);
	static void clearDescriptorCache();
	static std::map<Tile*, DensityDescriptor*> s_descriptorCache;
	static Mat3f Rotations[RSize];
	static Mat3f Mirros[MirrorSize];

private:
	DensityDescriptor* getDescriptor(Tile* const tile) const;


	
private:
	size_t m_resolution;
	PhotonMap* m_photonMap;
	
};