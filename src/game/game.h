#ifndef AST_GAME_H
#define AST_GAME_H

#include "types.h"

class Game
{
public:
    /*
     * arguments for starting a server
     */
    struct ServerArgs
    {
        std::string server_address;
        std::string server_port;
        int max_players;
    };

    /*
     * arguments for connecting to a game
     */
    struct GameworldArgs
    {
        std::string server_address;
        std::string server_port;
        std::string player_name;
    };

    /*
     * arguments for changing to the menu
     */
    struct MenuArgs
    {
        std::string error;
    };

    struct ClientArgs
    {
        std::string server_address;
        int server_port;
        std::string player_name;
    };

    struct ClientServerArgs
    {
        std::string server_address;
        int server_port;
        std::string player_name;
        int max_players;
    };

private:
    RenderWindowPtr window;
    sf::View view;

    GameStatePtr state;
    MainMenuPtr menu;
    ClientStatePtr client;
    ClientServerStatePtr client_server;

    bool dead;


public:
    Game();
    ~Game();

    void tick();
    void draw();

    void changeToMenu(const MenuArgs &args);
    void changeToClient(const ClientArgs &args);
    void changeToClientServer(const ClientServerArgs &args);

    bool isDead() const;

private:
    void doWindowEvents();
};
#endif /* AST_GAME_H */

