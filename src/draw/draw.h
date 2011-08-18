#ifndef AST_DRAW_H
#define AST_DRAW_H

#include <vector>
#include <cml/cml.h>
#include "types.h"

namespace Draw
{
void drawRect(float x, float y, float w, float h);
void drawAABB(const AABB &aabb);
void drawPoly(const std::vector<cml::vector2f> &verts);
void drawTexturedRect(float x, float y, float w, float h);
}

#endif /* AST_DRAW_H */

