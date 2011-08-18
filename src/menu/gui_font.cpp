#include "precompiled.h"
#include <guichan/opengl.hpp>
#include "gui_font.h"

#include "draw/draw.h"


GuiFont::GuiFont(RenderWindowPtr window, const std::string &filename, unsigned int char_size)
{
    this->char_size = char_size;
    this->window = window;

    if(!font.LoadFromFile(filename, char_size))
        fatal(format("failed to load font \"%1%\"\n") % filename);
}

int GuiFont::getWidth(const std::string& str) const
{
    sf::String s(str, font, char_size);
    return s.GetRect().GetWidth();
}

int GuiFont::getHeight() const
{
    return font.GetCharacterSize();
}

void GuiFont::drawString(gcn::Graphics* graphics, const std::string& str, int x, int y)
{
    gcn::OpenGLGraphics* gl_graphics = dynamic_cast<gcn::OpenGLGraphics*>(graphics);

    if(gl_graphics == NULL)
        return;

    const gcn::ClipRectangle& top = gl_graphics->getCurrentClipArea();
    int rx = x + top.xOffset;
    int ry = y + top.yOffset;

    sf::String text(str, font, char_size);
    text.SetPosition(rx, ry);

    gcn::Color c = gl_graphics->getColor();
    text.SetColor(sf::Color(c.r, c.g, c.b, c.a));

    //guichan opengl graphics sets the texture environment to replace
    //so set it back to modulate to draw the string.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    window->Draw(text);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

