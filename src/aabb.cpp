#include "precompiled.h"
#include "aabb.h"

AABB::AABB(){}

AABB::AABB(const cml::vector2f &p0, const cml::vector2f &p1)
{
    set(p0, p1);
}

void AABB::set(const cml::vector2f &p0, const cml::vector2f &p1)
{
    this->p0[0] = std::min(p0[0], p1[0]);
    this->p0[1] = std::min(p0[1], p1[1]);
    this->p1[0] = std::max(p0[0], p1[0]);
    this->p1[1] = std::max(p0[1], p1[1]);
}

const cml::vector2f& AABB::getP0() const
{
    return p0;
}

const cml::vector2f& AABB::getP1() const
{
    return p1;
}

float AABB::getWidth() const
{
    return p1[0] - p0[0];
}

float AABB::getHeight() const
{
    return p1[1] - p0[1];
}

bool AABB::intersects(const AABB &o) const
{
    const cml::vector2f &p2 = o.p0;
    const cml::vector2f &p3 = o.p1;

    if(p2[0] > p1[0]) return false;
    if(p3[0] < p0[0]) return false;
    if(p2[1] > p1[1]) return false;
    if(p3[1] < p0[1]) return false;

    return true;
}

bool AABB::containsPoint(const cml::vector2f &p) const
{
    if(p[0] < p0[0]) return false;
    if(p[0] > p1[0]) return false;
    if(p[1] < p0[1]) return false;
    if(p[1] > p1[1]) return false;

    return true;
}

