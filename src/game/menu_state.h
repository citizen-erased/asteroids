#ifndef AST_MENU_STATE
#define AST_MENU_STATE

#include "types.h"
#include "game_state.h"

class MenuState : public GameState
{
private:
    MainMenuPtr menu;

public:
    MenuState(RenderWindowPtr window);

    void setSize(int w, int h);

    void tick();
    void draw();
    void event(const sf::Event &evt);
};
#endif /* AST_MENU_STATE */

