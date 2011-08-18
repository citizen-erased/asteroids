#ifndef AST_SHIP_H
#define AST_SHIP_H

#include "types.h"
#include "entity.h"

class Ship : public Entity
{
private:
    b2WorldPtr physics;
    b2Body  *body;
    int tick_num;
    int shot_ticks;
    int last_shot_tick;
    FlameEmitterPtr flame1, flame2;
    cml::vector4f body_color;
    int player_id;
    std::string name;

    bool forward, reverse, left, right, shooting;


public:
    Ship(IEntityGameCallback *game, const ShipSpawnData &data);
    ~Ship();

    void tick();
    void draw();

    void onDie();

    void onHit(EntityPtr e);

    void processMessage(MessagePtr msg);

    void getSpawnData(ShipSpawnData *data) const;
    void getStateData(ShipStateData *data) const;
    void setStateData(const ShipStateData &data);

    void getControlData(ShipControlData *data) const;
    void setControlData(const ShipControlData &data);

    void event(const sf::Event &e);

    void clientOnDie();

    cml::vector2f getPosition() const;
    cml::vector4f getBodyColor() const;
    const std::string& getName() const;
    unsigned int getPlayerID() const;

private:
    void updateBounds();
};

#endif /* AST_SHIP_H */
