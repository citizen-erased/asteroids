#include "precompiled.h"
#include "menu/menu.h"
#include "menu_state.h"

MenuState::MenuState(RenderWindowPtr window)
{
    menu = MainMenuPtr(new MainMenu(window));
}

void MenuState::setSize(int w, int h)
{
    menu->setSize(w, h);
}

void MenuState::tick()
{
    menu->tick();
}
void MenuState::draw()
{
    menu->draw();
}

void MenuState::event(const sf::Event &evt)
{
    menu->event(evt);
}

