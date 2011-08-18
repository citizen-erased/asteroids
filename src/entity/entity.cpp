#include "precompiled.h"
#include "network/message.h"
#include "entity.h"

int Entity::entity_count = 0;

Entity::Entity(IEntityGameCallback *game, int id, EntityType::Enum type)
{
    print(format("+Entity %1%\n") % ++entity_count);

    this->game = game;
    this->id = id;
    this->type = type;

    on_server = game->isServer();
    dead = false;
}

Entity::~Entity()
{
    print(format("-Entity %1%\n") % --entity_count);
}

bool Entity::isDead() const
{
    return dead;
}

void Entity::die()
{
    if(!dead)
    {
        dead = true;
        onDie();
    }
}

const AABB& Entity::getBounds()
{
    return bounds;
}

int Entity::getID() const
{
    return id;
}

EntityType::Enum Entity::getType() const
{
    return type;
}

