#ifndef AST_AABB_H
#define AST_AABB_H

class AABB
{
private:
    cml::vector2f p0, p1;

public:
    AABB();
    AABB(const cml::vector2f &p0, const cml::vector2f &p1);

    void set(const cml::vector2f &p0, const cml::vector2f &p1);

    const cml::vector2f& getP0() const;
    const cml::vector2f& getP1() const;

    float getWidth() const;
    float getHeight() const;

    bool intersects(const AABB &o) const;
    bool containsPoint(const cml::vector2f &p) const;
};

#endif /* AST_AABB_H */

