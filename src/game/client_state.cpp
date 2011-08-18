#include "precompiled.h"
#include "client/client.h"
#include "game.h"
#include "client_state.h"

ClientState::ClientState(RenderWindowPtr window, const Game::ClientArgs &args)
{
    client = ClientPtr(new Client(window, args.server_address, args.server_port, args.player_name));
    client->start();
}

void ClientState::tick()
{
    client->tick();
}
void ClientState::draw()
{
    client->draw();
}

void ClientState::event(const sf::Event &evt)
{
    client->event(evt);
}

