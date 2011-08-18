#ifndef AST_GAME_STATE_H
#define AST_GAME_STATE_H

class GameState
{
public:
    virtual void tick(){}
    virtual void draw(){}
    virtual void event(const sf::Event&){}
};

#endif /* AST_GAME_STATE_H */

