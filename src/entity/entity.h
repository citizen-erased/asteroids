#ifndef AST_ENTITY_H
#define AST_ENTITY_H

#include "types.h"
#include "network/message.h"
#include "aabb.h"

class IEntityGameCallback
{
public:
    virtual bool isServer() const = 0;
    virtual b2WorldPtr getPhysics() = 0;
    virtual BulletPtr spawnBullet(BulletSpawnData &) = 0;
    virtual ParticleSystemPtr getParticleSystem() = 0;
    virtual void addBodyToEntity(b2Body *body, int id) = 0;
    virtual void removeBodyToEntity(b2Body *body) = 0;
    virtual void givePlayerPoints(int player_id, int pts) = 0;
    //virtual void queueReliableMessageAll(MessagePtr msg) = 0;
};

namespace EntityType
{
enum Enum
{
    UNKNOWN,
    ASTEROID,
    SHIP,
    BULLET,
};
}

class Entity
{
private:
    //FIXME only for debugging
    static int entity_count;

    int id;
    bool dead;
    EntityType::Enum type;

protected:
    IEntityGameCallback *game;
    bool on_server;
    AABB bounds;

public:
    Entity(IEntityGameCallback *game, int id, EntityType::Enum type);
    virtual ~Entity();

    virtual void tick(){};
    virtual void draw(){};

    void die();
    virtual void onDie(){};

    bool isDead() const;
    
    virtual void onHit(EntityPtr){};

    const AABB& getBounds();

    //virtual void getSpawnData(EntitySpawnData *data) const;
    //virtual void getStateData(EntityStateData&) const {};
    //virtual void setStateData(const EntityStateData&){};

    //virtual void processMessage(MessagePtr){};
    int getID() const;
    EntityType::Enum getType() const;
};


#if 0

class ServerEntity
{
private:
    int id;
    bool dead;

public:
    Entity(const EntitySpawnData &data);
    virtual ~Entity(){};

    virtual void tick(){};

    bool isDead() const;

    virtual void getSpawnData(EntitySpawnData &data) const;
    virtual void getUpdateData(EntityStateData &data) const {};
};


class ClientEntity
{
private:
    int id;
    bool dead;

public:
    Entity(const EntitySpawnData &data);
    virtual ~Entity(){};

    virtual void tick(){};
    virtual void draw(){};

    bool isDead() const;

    virtual void setUpdateData(const EntityStateData &data);
};
#endif

#endif /* AST_ENTITY_H */

