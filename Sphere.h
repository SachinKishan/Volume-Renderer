#pragma once

#include <cmath>

#include "ray.h"
#include "Object.h"
const double infinity = std::numeric_limits<double>::infinity();
class Sphere:public Object
{

public:
    Sphere() = default;
    Sphere(vec3 c, vec3 sc, float r, float s) :center(c), scatter(sc), radius(r), sigma(s) {}

    
	bool intersect(ray r, IsectData &iData) const override 
    {
    	vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        iData.t0 = (-half_b - sqrtd) / a;
        iData.t1 = (-half_b + sqrtd) / a;
        iData.inside = true;
        return true;
    }
    vec3 center;
    vec3 scatter;
    float sigma;
    float radius;
};

