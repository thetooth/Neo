#pragma once

#include "kami.h"
#include "version.h"
#include "tween.h"
#include "../UI/UI.h"
#include "minilzo.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <functional>
#include <Python.h>

#ifdef _DEBUG
//#include <vld.h>
#endif // DEBUG

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
#define chop(x)  ((x)>=0?(long)((x)+0.9):(long)((x)-0.9))

#define NEO_VERSION "v0.2.0"
#define NEO_NAMESPACE "neo"
const int APP_SCREEN_W = 640;
const int APP_SCREEN_H = 360;

#define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

inline int floorMpl(float x,int m){
	x = floor(x);
	if((int)x%m != 0){
		return (int)x;
	}else{
		return (int)x-((int)x%m);
	}
}

inline float getAngle(klib::Point<float> a, klib::Point<float> b){
	if (b.x == a.x){
		return 0.0f;
	}
	return atan((b.y - a.y) / (b.x - a.x))*180.0f/PI;
}