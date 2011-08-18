#include "precompiled.h"
#include "game_state.h"
#include "client_state.h"
#include "client_server_state.h"
#include "menu/menu.h"
#include "game.h"

Game::Game()
{
    window = RenderWindowPtr(new sf::RenderWindow(sf::VideoMode(1024, 768, 32), "Asteroids", (24, 8, 2)));
    window->PreserveOpenGLStates(true);
    window->SetView(view);
    view.SetFromRect(sf::FloatRect(0.0f, 0.0f, window->GetWidth(), window->GetHeight()));

    GLenum glew_err = glewInit();
    if(glew_err != GLEW_OK)
        fatal(format("Error initializing GLEW: %1%\n") % glewGetErrorString(glew_err));

    //FIXME move this somewhere else?
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1024, 0, 768);
    glMatrixMode(GL_MODELVIEW);

    menu = MainMenuPtr(new MainMenu(window, this));
    menu->setSize(window->GetWidth(), window->GetHeight());
    state = menu;
    dead = false;
}

Game::~Game()
{
}

void Game::tick()
{
    doWindowEvents();
    state->tick();
}

void Game::doWindowEvents()
{
    sf::Event e;
    while(window->GetEvent(e))
    {
        switch(e.Type)
        {
            case sf::Event::KeyPressed:
            {
                if(e.Key.Code == sf::Key::Escape)
                    dead = true;
                break;
            }
            case sf::Event::Closed:
                dead = true;
                break;
            case sf::Event::Resized:
            {
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, e.Size.Width, 0, e.Size.Height);
                glViewport(0, 0, e.Size.Width, e.Size.Height);

                //print(format("resized: w=%1% h=%2%\n") % e.Size.Width % e.Size.Height);
                view.SetFromRect(sf::FloatRect(0.0f, 0.0f, window->GetWidth(), window->GetHeight()));

                menu->setSize(window->GetWidth(), window->GetHeight());
                break;
            }
            default:
                break;
        }

        state->event(e);
    }
}

void Game::draw()
{
    //window->SetActive(true);
    state->draw();

    window->Display();
}

void Game::changeToMenu(const MenuArgs &args)
{
    if(client_server) client_server.reset();
    if(client) client.reset();

    state = menu;
}

void Game::changeToClient(const ClientArgs &args)
{
    if(client_server) client_server.reset();
    if(client) client.reset();

    client = ClientStatePtr(new ClientState(window, args));
    state = client;
}

void Game::changeToClientServer(const ClientServerArgs &args)
{
    if(client_server) client_server.reset();
    if(client) client.reset();

    client_server = ClientServerStatePtr(new ClientServerState(window, args));
    state = client_server;
}

bool Game::isDead() const
{
    return dead;
}

