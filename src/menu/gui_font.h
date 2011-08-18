#ifndef AST_GUI_FONT_H
#define AST_GUI_FONT_H

#include <string>
#include <guichan/font.hpp>
#include "types.h"

class GuiFont : public gcn::Font
{
private:
    sf::Font font;
    unsigned int char_size;
    RenderWindowPtr window;

public:
    GuiFont(RenderWindowPtr window, const std::string &filename, unsigned int char_size);

    int getWidth(const std::string& str) const;
    int getHeight() const;
    void drawString(gcn::Graphics* graphics, const std::string& str, int x, int y);
};

#endif /* AST_GUI_FONT_H */

