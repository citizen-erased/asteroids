#include "precompiled.h"
#include "client/client.h"
#include "server/server.h"
#include "game.h"
#include "client_server_state.h"

ClientServerState::ClientServerState(RenderWindowPtr window, const Game::ClientServerArgs &args)
{
    server = ServerPtr(new Server(args.server_address, args.server_port, args.max_players));
    server->start();
    client = ClientPtr(new Client(window, args.server_address, args.server_port, args.player_name));
    client->start();
}

void ClientServerState::tick()
{
    server->tick();
    client->tick();
}
void ClientServerState::draw()
{
    client->draw();
}

void ClientServerState::event(const sf::Event &evt)
{
    client->event(evt);
}

