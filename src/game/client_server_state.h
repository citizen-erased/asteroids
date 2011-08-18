#include "precompiled.h"
#include "types.h"
#include "game.h"
#include "game_state.h"

class ClientServerState : public GameState
{
private:
    ServerPtr server;
    ClientPtr client;

public:
    ClientServerState(RenderWindowPtr window, const Game::ClientServerArgs &args);

    void tick();
    void draw();
    void event(const sf::Event &evt);
};
