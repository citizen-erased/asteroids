#ifndef AST_SERVER_GAME_H
#define AST_SERVER_GAME_H

#include "entity/entity.h"
#include "entity/asteroid.h"
#include "entity/ship.h"
#include "entity/bullet.h"
#include "aabb.h"

class b2World;
class Server;


class ServerGame : public IEntityGameCallback, b2ContactListener
{
private:
    struct ServerGamePlayer
    {
        int id;
        std::string name;

        int points;
        cml::vector4f color;
        ShipPtr ship;
        unsigned int dead_since;
        std::map<int, unsigned int> last_ent_updates;
    };

    struct LastUpdateSorter
    {
        ServerGamePlayer &player;

        LastUpdateSorter(ServerGamePlayer &player);
        bool operator()(const AsteroidPtr a, const AsteroidPtr b) const;
    };

private:
    ServerPtr server;
    b2WorldPtr physics;


    /*
     *                       The Void
     *
     *  -----------------------------------------------------
     *  |                Asteroid Bounds                    |
     *  |                                                   |
     *  |      --------------------------------------       |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |      |            Game Bounds             |       |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |      --------------------------------------       |
     *  |                                                   |
     *  |                                                   |
     *  -----------------------------------------------------
     *
     * The game bounds contains the world area the player can see.
     * Ships and bullets should be killed when they leave the game bounds.
     * Asteroid Bounds is the area asteroids spawn in.
     * Anything that enters the void is killed.
     *
     */
    AABB game_bounds;
    AABB asteroid_bounds;

    std::vector<EntityPtr> entities;
    std::map<int, EntityPtr> id_to_entity;
    std::vector<AsteroidPtr> asteroids;
    std::vector<ShipPtr> ships;
    std::vector<BulletPtr> bullets;
    std::vector<ServerGamePlayer> players;

    AsteroidDataCache asteroid_data;

    std::map<b2Body*, int> body_to_entity_id;

    b2Body *world_bounds_body;

    int entity_id;
    unsigned int tick_num;
    unsigned int last_send_score_tick;

    std::vector<cml::vector4f> ship_colors;

public:
    ServerGame(ServerPtr server);
    ~ServerGame();

    void processPlayerMessage(int id, MessagePtr msg);
    void tick();

    void addPlayer(int id, const std::string &name);
    void removePlayer(int id);

    void getGamestate(std::vector<MessagePtr> *msgs);

    bool isServer() const;
    b2WorldPtr getPhysics();
    void queueReliableMessageAll(MessagePtr msg);

    void addBodyToEntity(b2Body *body, int id);
    void removeBodyToEntity(b2Body *body);
    EntityPtr getBodyEntity(b2Body *body);

    ParticleSystemPtr getParticleSystem();

private:
    void createWorldBounds();
    int genEntityID();

    void addEntity(EntityPtr e);
    void removeEntity(unsigned int i);

    ShipPtr spawnShip(ShipSpawnData &data);
    BulletPtr spawnBullet(BulletSpawnData &data);
    AsteroidPtr spawnAsteroidRandom();
    AsteroidPtr spawnAsteroid(AsteroidSpawnData &data);
    ShipPtr respawnPlayer(ServerGamePlayer &player);

    void getAsteroidsNear(const cml::vector2f &pos, std::vector<AsteroidPtr> &vec);

    void sendAsteroidUpdate(ServerGamePlayer &p, AsteroidPtr a);

    void givePlayerPoints(int player_id, int pts);

    /* Box2D contact callbacks */
    void BeginContact(b2Contact *contact);
    void EndContact(b2Contact *contact);
    void PreSolve(b2Contact *contact, const b2Manifold *oldManifold);
    void PostSolve(const b2Contact *contact, const b2ContactImpulse *impulse);
};

#endif /* AST_SERVER_GAME_H */

