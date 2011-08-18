#ifndef AST_MENU_H
#define AST_MENU_H

#include <guichan.hpp>
#include "game/game_state.h"
#include "types.h"

class GuiInput;
class GuiFont;
class GuiImageLoader;

class MainMenu : public GameState, gcn::ActionListener
{
private:
    RenderWindowPtr window;
    Game                *game;

    gcn::Gui            *gui;
    GuiInput            *input;
    gcn::OpenGLGraphics *graphics;
    GuiImageLoader      *image_loader;

    gcn::Container      *top;
    gcn::TextField      *address_field;
    gcn::TextField      *port_field;
    gcn::TextField      *player_name_field;
    gcn::Button         *connect_button;
    gcn::Button         *create_button;
    gcn::Label          *logo_label;
    gcn::Label          *address_label;
    gcn::Label          *port_label;
    gcn::Label          *player_name_label;

    GuiFont             *font;
    GuiFont             *logo_font;

public:
    MainMenu(RenderWindowPtr window, Game *game);
    virtual ~MainMenu();

    void setSize(int w, int h);

    void tick();
    void draw();
    void event(const sf::Event &evt);

    void action(const gcn::ActionEvent &evt);
};

#endif /* AST_MENU_H */

