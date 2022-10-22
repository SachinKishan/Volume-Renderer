#pragma once

#include "vec3.h"
constexpr float floatMax = std::numeric_limits<float>::max();

class IsectData
{
public:
    float t0{ floatMax }, t1{ floatMax };
    vec3 pHit;
    vec3 nHit;
    bool inside{ false };

};

class Object
{
public:
    vec3 color;
    int type{ 0 };
    virtual bool intersect(ray, IsectData&) const = 0;
    virtual ~Object() = default;
    Object() = default;
};
