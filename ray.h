#pragma once

#include "vec3.h"

class ray {
public:

    point3 orig;
    vec3 dir;

    //getters
    point3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    //constructor
    ray() {}
    ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}


    //interpolation
    point3 at(double t) const
    {
        return orig + t * dir;
    }


};