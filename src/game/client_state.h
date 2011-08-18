#include "precompiled.h"
#include "types.h"
#include "game.h"
#include "game_state.h"

class ClientState : public GameState
{
private:
    ClientPtr client;

public:
    ClientState(RenderWindowPtr window, const Game::ClientArgs &args);

    void tick();
    void draw();
    void event(const sf::Event &evt);
};
