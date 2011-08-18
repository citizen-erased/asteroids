#include "precompiled.h"
#include <guichan/opengl.hpp>
#include "gui_font.h"
#include "gui_input.h"
#include "gui_image_loader.h"
#include "game/game.h"
#include "menu.h"

MainMenu::MainMenu(RenderWindowPtr window, Game *game)
{
    this->window = window;
    this->game = game;

    gui = new gcn::Gui();
    graphics = new gcn::OpenGLGraphics();
    input = new GuiInput();
    image_loader = new GuiImageLoader();

    font = new GuiFont(window, "fonts/steinerlight.ttf", 30);
    gcn::Widget::setGlobalFont(font);

    logo_font = new GuiFont(window, "fonts/gamecube.ttf", 50);

    gcn::Image::setImageLoader(image_loader);
    gui->setGraphics(graphics);
    gui->setInput(input);

    top = new gcn::Container();
    //top->setOpaque(false);
    gui->setTop(top);

    logo_label = new gcn::Label("ASTEROIDS");
    logo_label->setFont(logo_font);
    top->add(logo_label);

    address_field = new gcn::TextField("localhost");
    top->add(address_field);

    port_field = new gcn::TextField("4567");
    top->add(port_field);

    player_name_field = new gcn::TextField("player");
    top->add(player_name_field);

    address_label = new gcn::Label("Address:");
    top->add(address_label);

    port_label = new gcn::Label("Port:");
    top->add(port_label);

    player_name_label = new gcn::Label("Player:");
    top->add(player_name_label);

    connect_button = new gcn::Button("connect");
    connect_button->addActionListener(this);
    top->add(connect_button);

    create_button = new gcn::Button("create");
    create_button->addActionListener(this);
    top->add(create_button);

    setSize(window->GetWidth(), window->GetHeight());
}

MainMenu::~MainMenu()
{
    delete logo_font;
    delete font;
    delete image_loader;
    delete graphics;
    delete input;
    delete gui;

    delete top;
    delete address_field;
    delete port_field;
    delete player_name_field;
    delete connect_button;
    delete create_button;
}

void MainMenu::setSize(int w, int h)
{
    graphics->setTargetPlane(w, h);
    top->setDimension(gcn::Rectangle(0, 0, w, h));

    int hw = w/2;
    int hh = h/2;

    address_label->setWidth(150);
    address_field->setWidth(200);
    port_label->setWidth(150);
    port_field->setWidth(200);
    player_name_label->setWidth(150);
    player_name_field->setWidth(200);
    connect_button->setWidth(200);
    create_button->setWidth(200);

    logo_label->setSize(logo_font->getWidth("ASTEROIDS"), logo_font->getHeight());
    logo_label->setPosition(hw-logo_label->getWidth()/2, 0);

    address_label->setPosition(hw-address_label->getWidth(), 100);
    address_field->setPosition(hw, 100);

    port_label->setPosition(hw-port_label->getWidth(), address_field->getY()+address_field->getHeight()+10);
    port_field->setPosition(hw, address_field->getY()+address_field->getHeight()+10);

    player_name_label->setPosition(hw-player_name_label->getWidth(), port_field->getY()+port_field->getHeight()+10);
    player_name_field->setPosition(hw, port_field->getY()+port_field->getHeight()+10);

    connect_button->setPosition(hw-connect_button->getWidth()/2, player_name_field->getY()+player_name_field->getHeight()+10);
    create_button->setPosition(hw-create_button->getWidth()/2, connect_button->getY()+connect_button->getHeight()+10);
}

void MainMenu::action(const gcn::ActionEvent &evt)
{
    if(evt.getSource() == connect_button)
    {
        print("connect pressed\n");
        Game::ClientArgs args;
        args.server_address = address_field->getText();
        args.server_port = 4567;
        args.player_name = player_name_field->getText();

        std::istringstream ss(port_field->getText());
        ss >> args.server_port;

        game->changeToClient(args);
    }
    else if(evt.getSource() == create_button)
    {
        print("create pressed\n");
        Game::ClientServerArgs args;
        args.server_address = address_field->getText();
        args.server_port = 4567;
        args.player_name = player_name_field->getText();
        args.max_players = 8;

        std::istringstream ss(port_field->getText());
        ss >> args.server_port;

        game->changeToClientServer(args);
    }
}

void MainMenu::tick()
{
    gui->logic();
}

void MainMenu::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    gui->draw();
}

void MainMenu::event(const sf::Event &evt)
{
    input->event(evt);
}


