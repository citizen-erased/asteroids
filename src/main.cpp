#include "precompiled.h"
#include "client/client.h"
#include "server/server.h"
#include "server/server_game.h"
#include "game/game.h"

#include "network/message.h"

void nonDedicated()
{
    GamePtr game(new Game());

    while(!game->isDead())
    {
        game->tick();
        game->draw();
        sf::Sleep(0.016f);
    }

    game.reset();
}

void dedicatedServer(const std::vector<std::string> &args)
{
    std::string address = "localhost";
    int port = 4567;
    int max_players = 8;

    for(unsigned int i = 0; i < args.size(); i++)
        if(args[i] == "--address" && i+1 < args.size())
            address = args[i+1];
        else if(args[i] == "--port" && i+1 < args.size())
            port = boost::lexical_cast<int>(args[i+1]);
        else if(args[i] == "--max-players" && i+1 < args.size())
            max_players = boost::lexical_cast<int>(args[i+1]);


    print(format("starting dedicated server on %1%:%2%  max_players=%3%\n") % address % port % max_players);

    ServerPtr server(new Server(address, port, max_players));
    server->start();

    while(true)
    {
        server->tick();
        sf::Sleep(0.016f);
    }

    server.reset();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> args;

    for(int i = 0; i < argc; i++)
        args.push_back(std::string(argv[i]));

    print("initializing ENet\n");
    if(enet_initialize() != 0)
        fatal("Error initializing ENet\n");

    bool dedicated = false;
    for(unsigned int i = 0; i < args.size(); i++)
        if(args[i] == "-s")
            dedicated = true;

    if(dedicated)
        dedicatedServer(args);
    else
        nonDedicated();

    print("shutting down ENet\n");
    enet_deinitialize();

    return 0;
}

/*
int main(int argc, char* argv[])
{
    print("initializing ENet\n");
    if(enet_initialize() != 0)
        fatal("Error initializing ENet\n");

    game1 = GamePtr(new Game());

    if(argc > 1 && std::string(argv[1]) == "-s")
    {
        Game::ClientServerArgs args;
        game1->changeToClientServer(args);
    }
    else
    {
        Game::ClientArgs args2;
        //game1->changeToClient(args2);
    }

    while(!game1->isDead())
    {
        game1->tick();
        game1->draw();
        sf::Sleep(0.016f);
    }

    game1.reset();

    print("shutting down ENet\n");
    enet_deinitialize();

    return 0;
}
*/
