#pragma once

#define GLEW_STATIC
#define NOMINMAX
//#include<vld.h>
#include<cfloat>
#include"glew.h"
#include <gl\GL.h>
#include <gl\GLU.h>
#include<vector>
#include<windows.h>


#undef near
#undef far
//
//#undef NEAR
//#undef FAR

typedef size_t	uint;
typedef unsigned char	byte;

#define UINF MAXUINT32


enum DMode { DM_LIGHT = 0, DM_FLAT, DM_WIREFRAME, DM_POINTS, DM_SIZE };
enum PMode { PM_NONE = 0, PM_POINTS, PM_DESCRIPTORS, PM_SIZE};

enum RESOLUTION_STRATEGY { RES_STRATEGY_MAX = 0, RES_STRATEGY_MIN, RES_STRATEGY_AVERAGE, RES_STRATEGY_SIZE};

static std::string STRATEGY_STRINGS[RES_STRATEGY_SIZE] = {"STRATEGY_MAX", "STRATEGY_MIN", "STRATEGY_AVERAGE"};

inline void debug(const char* format, ...) {

	
	char buff[4096];
	vsprintf_s( buff,format, ((char*)&format) + sizeof(void*));
	OutputDebugString(buff);
}

enum DesctiptorTypes { DensityDesc = 0, DirHistogramDesc };

enum MetricTypes { PhotonCountMetric = 0, DensityMetric, DensityDirMetric};

extern int density_photons;