#pragma once
#include "Object.h"
#include "ray.h"

class Box: public Object
{
public:
	Box(const vec3 &vmin, const vec3 &vmax )
	{

		bounds[0] = vmin;
		bounds[1] = vmax;
	}
	bool intersect(ray r, IsectData& iData) const override
	{
        vec3 min = bounds[0];
        vec3 max = bounds[1];
		iData.t0= (min.x() - r.orig.x()) / r.dir.x();
        iData.t1= (max.x() - r.orig.x()) / r.dir.x();

		if (iData.t0 > iData.t1) std::swap(iData.t0, iData.t1);

		float tymin = (min.y() - r.orig.y()) / r.dir.y();
        float tymax = (max.y() - r.orig.y()) / r.dir.y();

        if (tymin > tymax) std::swap(tymin, tymax);

        if ((iData.t0 > tymax) || (tymin > iData.t1))
            return false;

        if (tymin > iData.t0)
            iData.t0 = tymin;

        if (tymax < iData.t1)
            iData.t1 = tymax;

        float tzmin = (min.z() - r.orig.z()) / r.dir.z();
        float tzmax = (max.z() - r.orig.z()) / r.dir.z();

        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((iData.t0 > tzmax) || (tzmin > iData.t1))
            return false;

        if (tzmin > iData.t0)
            iData.t0 = tzmin;

        if (tzmax < iData.t1)
            iData.t1 = tzmax;

        return true;
	}
	vec3 bounds[2];//min and max extent of the box
};


class Grid:public Box
{
public:
    Grid(const vec3& vmin, const vec3& vmax):Box(vmin,vmax)
    {}

    float* density;
    size_t dimension = 128;
};
