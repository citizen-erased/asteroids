#include "precompiled.h"
#include "aabb.h"
#include "draw.h"

void Draw::drawRect(float x, float y, float w, float h)
{
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x, y+h);
    glEnd();
}

void Draw::drawAABB(const AABB &aabb)
{
    drawRect(aabb.getP0()[0], aabb.getP0()[1], aabb.getWidth(), aabb.getHeight());
}

void Draw::drawPoly(const std::vector<cml::vector2f> &verts)
{
    glBegin(GL_LINE_LOOP);
        for(unsigned int i = 0; i < verts.size(); i++)
            glVertex2fv(verts[i].data());
    glEnd();
}

void Draw::drawTexturedRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x+w, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x+w, y+h);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y+h);
    glEnd();
}

