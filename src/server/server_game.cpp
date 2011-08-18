#include "precompiled.h"
#include "network/message.h"
#include "server.h"
#include "server_game.h"

ServerGame::LastUpdateSorter::LastUpdateSorter(ServerGamePlayer &player)
: player(player){}

bool ServerGame::LastUpdateSorter::operator()(const AsteroidPtr a, const AsteroidPtr b) const
{
    unsigned int tick_a = player.last_ent_updates[a->getID()];
    unsigned int tick_b = player.last_ent_updates[b->getID()];
    return compareTicks(tick_b, tick_a);
}

ServerGame::ServerGame(ServerPtr server)
{
    this->server = server;

    ship_colors.push_back(cml::vector4f(1.0f, 0.0f, 0.0f, 1.0f));
    ship_colors.push_back(cml::vector4f(0.0f, 1.0f, 0.0f, 1.0f));
    ship_colors.push_back(cml::vector4f(0.0f, 0.0f, 1.0f, 1.0f));

    ship_colors.push_back(cml::vector4f(255.0f/255.0f,  20.0f/255.0f, 147.0f/255.0f, 1.0f));
    ship_colors.push_back(cml::vector4f( 70.0f/255.0f, 130.0f/255.0f, 180.0f/255.0f, 1.0f));
    ship_colors.push_back(cml::vector4f(255.0f/255.0f, 215.0f/255.0f,   0.0f/255.0f, 1.0f));
    ship_colors.push_back(cml::vector4f(255.0f/255.0f,  69.0f/255.0f,   0.0f/255.0f, 1.0f));
    ship_colors.push_back(cml::vector4f(113.0f/255.0f, 198.0f/255.0f, 113.0f/255.0f, 1.0f));

    tick_num = 0;
    last_send_score_tick = 0;
    entity_id = 0;

    physics = b2WorldPtr(new b2World(b2Vec2(0.0f, 0.0f), true));
    physics->SetContactListener(this);
    createWorldBounds();

    asteroid_data.load("asteroids.dat");

    for(int i = 0; i < 40; i++)
        spawnAsteroidRandom();
}

ServerGame::~ServerGame()
{

}

void ServerGame::createWorldBounds()
{
    float width = 500.0f;
    float height = 500.0f;

    game_bounds.set(cml::vector2f(0.0f, 0.0f), cml::vector2f(width, height));
    asteroid_bounds.set(cml::vector2f(-50.0f, -50.0f), cml::vector2f(width+50.0f, height+50.0f));

    //TODO get rid of this
    b2BodyDef b;
    b.type = b2_staticBody;
    b.position.Set(width/2.0f, height/2.0f);
    world_bounds_body = physics->CreateBody(&b);

    b2PolygonShape shape;
    shape.SetAsBox(width/2.0f, height/2.0f);
    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.isSensor = true;
    world_bounds_body->CreateFixture(&fixture);
}

void ServerGame::processPlayerMessage(int id, MessagePtr msg)
{
    ServerGamePlayer *player = NULL;

    for(unsigned int i = 0; i < players.size(); i++)
        if(players[i].id == id)
        {
            player = &players[i];
            break;
        }
    
    if(player == NULL)
    {
        print(format("ServerGame::processPlayerMessage: player id %1% not found\n") % id);
        return;
    }

    switch(msg->getType())
    {
        case MessageType::CLI_SHIP_CONTROL:
        {
            //print(format("ServerGame: received ship control data from player.id=%1%\n") % id);
            ShipControlMessage *control_msg = static_cast<ShipControlMessage*>(msg.get());
            //print(format("player->ship = %1%\n")%player->ship);
            if(player->ship)
                player->ship->setControlData(control_msg->getData());
        }
        break;

        default: break;
    }
}

void ServerGame::tick()
{
    tick_num++;

    /*
     * step physics
     */
    physics->Step(1.0f/60.0f, 10, 8);
    physics->ClearForces();

    /*
     * tick entities
     */
    for(unsigned int i = 0; i < entities.size(); i++)
        entities[i]->tick();

    /*
     * send updated entity state
     */
    for(unsigned int i = 0; i < ships.size(); i++)
    {
        ShipStateData d;
        ships[i]->getStateData(&d);
        MessagePtr msg(new ShipStateMessage(ships[i]->getID(), d));
        server->queueUnreliableMessageAll(msg);
    }

    for(unsigned int i = 0; i < players.size(); i++)
    {
        if(players[i].ship)
        {
            std::vector<AsteroidPtr> near;
            std::set<AsteroidPtr> updated_asteroids;

            getAsteroidsNear(players[i].ship->getPosition(), near);
            std::stable_sort(near.begin(), near.end(), LastUpdateSorter(players[i]));

            //print(format("%1% asteroids near player %2%\n") % near.size() % players[i].id);

            //number of asteroid updates to send per tick
            int updates_per_tick = 3;

            //asteroid updates sent
            int updates = 0;

            /*
             * send updates for any asteroids near the player
             */
            for(unsigned int j = 0; j < near.size(); j++)
            {
                //print(format("asteroid %1% is near player last_update=%2%   %3%\n") % near[j]->getID() % players[i].last_ent_updates[near[j]->getID()] % players[i].last_ent_updates.size());

                sendAsteroidUpdate(players[i], near[j]);
                updated_asteroids.insert(near[j]);

                if(++updates >= updates_per_tick)
                    break;
            }

            /*
             * send any remaining updates for asteroids updated the longest time ago
             */
            if(updates < updates_per_tick)
            {
                std::vector<AsteroidPtr> remaining = asteroids;
                std::stable_sort(remaining.begin(), remaining.end(), LastUpdateSorter(players[i]));

                for(unsigned int j = 0; j < remaining.size(); j++)
                {
                    if(updated_asteroids.find(remaining[j]) != updated_asteroids.end())
                        continue;

                    //print(format("asteroid %1% is remaining fpr player last_update=%2%\n") % remaining[j]->getID() % players[i].last_ent_updates[remaining[j]->getID()]);
                    sendAsteroidUpdate(players[i], remaining[j]);

                    if(++updates >= updates_per_tick)
                        break;
                }
            }
        }
    }

    /*
    for(unsigned int i = 0; i < asteroids.size(); i++)
    {
        AsteroidStateData d;
        asteroids[i]->getStateData(&d);
        MessagePtr msg(new AsteroidStateMessage(asteroids[i]->getID(), d));
        server->queueUnreliableMessageAll(msg);
    }
    */

    /*
     * kill entities that are out of bounds
     */
    for(unsigned int i = 0; i < asteroids.size(); i++)
        if(!asteroid_bounds.intersects(asteroids[i]->getBounds()))
            asteroids[i]->die();

    for(unsigned int i = 0; i < ships.size(); i++)
        if(!game_bounds.intersects(ships[i]->getBounds()))
        {
            givePlayerPoints(ships[i]->getPlayerID(), -2);
            ships[i]->die();
        }

    for(unsigned int i = 0; i < bullets.size(); i++)
        if(!game_bounds.intersects(bullets[i]->getBounds()))
            bullets[i]->die();

    /*
     * tell clients to delete dead entities.
     */
    for(unsigned int i = 0; i < entities.size(); i++)
    {
        if(entities[i]->isDead())
        {
            EntityDeleteData d;
            d.entity_id = entities[i]->getID();
            MessagePtr msg(new EntityDeleteMessage(d));
            server->queueReliableMessageAll(msg);
        }
    }

    /*
     * remove dead entities
     */
    //TODO replace with std::remove_if
    unsigned int i = 0;
    while(i < entities.size())
    {
        if(entities[i]->isDead())
            removeEntity(i);
        else
            i++;
    }

    i = 0;
    while(i < asteroids.size())
    {
        if(asteroids[i]->isDead())
            asteroids.erase(asteroids.begin()+i);
        else
            i++;
    }

    i = 0;
    while(i < ships.size())
    {
        if(ships[i]->isDead())
        {
            for(unsigned int j = 0; j < players.size(); j++)
                if(players[j].ship == ships[i])
                {
                    players[j].ship.reset();
                    players[j].dead_since = tick_num;
                    break;
                }

            ships.erase(ships.begin()+i);
        }
        else
            i++;
    }

    i = 0;
    while(i < bullets.size())
    {
        if(bullets[i]->isDead())
            bullets.erase(bullets.begin()+i);
        else
            i++;
    }


    /*
     * spawn asteroids
     */
    if(asteroids.size() < 60)
        spawnAsteroidRandom();
    

    /*
     * respawn dead player ships
     */
    for(unsigned int i = 0; i < players.size(); i++)
        if(!players[i].ship && tick_num - players[i].dead_since > 100)
            respawnPlayer(players[i]);


    /*
     * send scores
     */
    if(tick_num - last_send_score_tick > 60)
    {
        last_send_score_tick = tick_num;
        ScoreData sd;

        for(unsigned int i = 0; i < players.size(); i++)
        {
            sd.names.push_back(players[i].name);
            sd.scores.push_back(players[i].points);
        }

        MessagePtr msg(new ScoreMessage(sd));
        server->queueUnreliableMessageAll(msg);
    }
}

void ServerGame::getAsteroidsNear(const cml::vector2f &pos, std::vector<AsteroidPtr> &vec)
{
    cml::vector2f extent(125, 125);
    AABB bounds(pos-extent, pos+extent);

    for(unsigned int i = 0; i < asteroids.size(); i++)
        if(bounds.intersects(asteroids[i]->getBounds()))
            vec.push_back(asteroids[i]);
}

void ServerGame::sendAsteroidUpdate(ServerGamePlayer &p, AsteroidPtr a)
{
    AsteroidStateData d;
    a->getStateData(&d);
    MessagePtr msg(new AsteroidStateMessage(a->getID(), d));
    server->queueUnreliableMessage(p.id, msg);
    p.last_ent_updates[a->getID()] = tick_num;
}

void ServerGame::addPlayer(int id, const std::string &name)
{
    print(format("a player joined the game [id=%1%]\n") % id);

    players.push_back(ServerGamePlayer());
    ServerGamePlayer &player = players[players.size()-1];
    player.id = id;
    player.name = name;
    player.color = ship_colors[rand()%ship_colors.size()];

    for(unsigned int i = 0; i < entities.size(); i++)
        player.last_ent_updates[entities[i]->getID()] = tick_num;

    /*
     * tell the player to create a new game state
     */
    GameStateData gamestate_data;
    gamestate_data.game_bounds = game_bounds;
    MessagePtr gamestate_msg(new GameStateMessage(gamestate_data));
    server->queueReliableMessage(id, gamestate_msg);


    /*
     * tell player to spawn other players ships
     */
    for(unsigned int i = 0; i < ships.size(); i++)
    {
        ShipSpawnData d;
        ships[i]->getSpawnData(&d);
        print(format("ServerGame: sending ship[%3%] players[i].id=%1%   player.id=%2%\n") % players[i].id % player.id % d.id);
        MessagePtr msg(new ShipSpawnMessage(d));
        server->queueReliableMessage(id, msg);
    }

    /*
     * create the new player's ship
     */
    respawnPlayer(player);

    /*
     * tell the new player to spawn all the asteroids
     */
    for(unsigned int i = 0; i < asteroids.size(); i++)
    {
        AsteroidSpawnData sd;
        asteroids[i]->getSpawnData(&sd);
        MessagePtr msg(new AsteroidSpawnMessage(sd));
        server->queueReliableMessage(id, msg);
    }

    /*
     * tell the new player to spawn all the bullets
     */
    for(unsigned int i = 0; i < bullets.size(); i++)
    {
        BulletSpawnData sd;
        bullets[i]->getSpawnData(&sd);
        MessagePtr msg(new BulletSpawnMessage(sd));
        server->queueReliableMessage(id, msg);
    }

    //TODO stuff
}

void ServerGame::removePlayer(int id)
{
    for(std::vector<ServerGamePlayer>::iterator it = players.begin(); it != players.end(); it++)
        if(it->id == id)
        {
            print(format("a player left the game [id=%1%]\n") % id);
            //TODO kill their ship
            players.erase(it);
            break;
        }
}

void ServerGame::addEntity(EntityPtr e)
{
    entities.push_back(e);
    id_to_entity[e->getID()] = e;

    for(unsigned int i = 0; i < players.size(); i++)
        players[i].last_ent_updates[e->getID()] = tick_num;
}

void ServerGame::removeEntity(unsigned int index)
{
    for(unsigned int i = 0; i < players.size(); i++)
        players[i].last_ent_updates.erase(entities[index]->getID());

    id_to_entity.erase(entities[index]->getID());
    entities.erase(entities.begin()+index);

}

ShipPtr ServerGame::spawnShip(ShipSpawnData &data)
{
    data.id = genEntityID();

    ShipPtr ship(new Ship(this, data));
    ships.push_back(ship);
    addEntity(ship);

    MessagePtr msg(new ShipSpawnMessage(data));
    server->queueReliableMessageAll(msg);

    return ship;
}

BulletPtr ServerGame::spawnBullet(BulletSpawnData &data)
{
    data.id = genEntityID();

    BulletPtr bullet(new Bullet(this, data));
    bullets.push_back(bullet);
    addEntity(bullet);

    MessagePtr msg(new BulletSpawnMessage(data));
    server->queueReliableMessageAll(msg);

    return BulletPtr();
}

ShipPtr ServerGame::respawnPlayer(ServerGamePlayer &player)
{
    //TODO don't spawn player on an asteroid
    float x = game_bounds.getP0()[0] + (float(rand())/RAND_MAX * game_bounds.getWidth());
    float y = game_bounds.getP0()[1] + (float(rand())/RAND_MAX * game_bounds.getHeight());

    ShipSpawnData d;
    d.id = genEntityID();
    d.player_id = player.id;
    d.body_color = player.color;
    d.name = player.name;
    d.state.position.set(x, y);
    d.state.velocity.set(0.0, 0.0);
    d.state.rotation = 0;
    d.state.angular_velocity = 0;
    d.state.health = 100;

    ShipPtr ship = spawnShip(d);
    player.ship = ship;

    /*
     * tell the player they own this ship
     */
    ShipOwnerData ship_owner_data;
    ship_owner_data.ship_id = ship->getID();
    MessagePtr ship_owner_msg(new ShipOwnerMessage(ship_owner_data));
    print(format("ServerGame: sending ship[%2%] owner player.id=%1%\n") % player.id % d.id);
    server->queueReliableMessage(player.id, ship_owner_msg);

    return ship;
}

AsteroidPtr ServerGame::spawnAsteroidRandom()
{
    /*
     *                       The Void
     *
     *  -----------------------------------------------------
     *  |                Asteroid Bounds                    |
     *  |                  sector 0                         |
     *  |""""""--------------------------------------"""""""|
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |sector|                                    |sector |
     *  |  2   |            Game Bounds             |  3    |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |      |                                    |       |
     *  |""""""--------------------------------------"""""""|
     *  |                                                   |
     *  |                  sector 1                         |
     *  -----------------------------------------------------
     *
     */
    int sector = rand() % 4;
    float sec_x, sec_y, sec_w, sec_h;
    switch(sector)
    {
        case 0:
            sec_x = asteroid_bounds.getP0()[0];
            sec_y = game_bounds.getP1()[1];
            sec_w = asteroid_bounds.getWidth();
            sec_h = asteroid_bounds.getP1()[1] - game_bounds.getP1()[1];
        break;
        case 1:
            sec_x = asteroid_bounds.getP0()[0];
            sec_y = asteroid_bounds.getP0()[1];
            sec_w = asteroid_bounds.getWidth();
            sec_h = game_bounds.getP0()[1] - asteroid_bounds.getP0()[1];
        break;
        case 2:
            sec_x = asteroid_bounds.getP0()[0];
            sec_y = game_bounds.getP0()[1];
            sec_w = game_bounds.getP0()[0] - asteroid_bounds.getP0()[0];
            sec_h = game_bounds.getHeight();
        break;
        case 3:
            sec_x = game_bounds.getP1()[0];
            sec_y = game_bounds.getP0()[1];
            sec_w = asteroid_bounds.getP1()[0] - game_bounds.getP1()[0];
            sec_h = game_bounds.getHeight();
        break;
    }

    float x = sec_x + (float(rand())/RAND_MAX * sec_w);
    float y = sec_y + (float(rand())/RAND_MAX * sec_h);

    float target_x = game_bounds.getP0()[0] + (float(rand())/RAND_MAX * game_bounds.getWidth());
    float target_y = game_bounds.getP0()[1] + (float(rand())/RAND_MAX * game_bounds.getHeight());
    cml::vector2f velocity = cml::normalize(cml::vector2f(target_x, target_y) - cml::vector2f(x, y));

    float speed = (float(rand())/RAND_MAX * 20.0f) + 2.0f;
    velocity *= speed;

    AsteroidSpawnData d;
    d.asteroid_type = asteroid_data.getRandomType();
    d.state.position.set(x, y);
    d.state.velocity.set(velocity[0], velocity[1]);
    d.state.rotation = 0;
    d.state.angular_velocity = (float(rand())/RAND_MAX * 10.0f) - 5.0f;

    return spawnAsteroid(d);
}

AsteroidPtr ServerGame::spawnAsteroid(AsteroidSpawnData &data)
{
    data.id = genEntityID();

    AsteroidPtr ast(new Asteroid(this, data, asteroid_data));
    asteroids.push_back(ast);
    addEntity(ast);

    MessagePtr msg(new AsteroidSpawnMessage(data));
    server->queueReliableMessageAll(msg);

    return ast;
}

bool ServerGame::isServer() const
{
    return true;
}

b2WorldPtr ServerGame::getPhysics()
{
    return physics;
}

void ServerGame::queueReliableMessageAll(MessagePtr msg)
{
    for(unsigned int i = 0; i < players.size(); i++)
        server->queueReliableMessage(players[i].id, msg);
}

int ServerGame::genEntityID()
{
    return entity_id++;
}

void ServerGame::givePlayerPoints(int player_id, int pts)
{
    for(unsigned int i = 0; i < players.size(); i++)
        if(players[i].id == player_id)
        {
            players[i].points += pts;
            break;
        }
}


void ServerGame::addBodyToEntity(b2Body *body, int id)
{
    body_to_entity_id[body] = id;
}

void ServerGame::removeBodyToEntity(b2Body *body)
{
    body_to_entity_id.erase(body);
}

EntityPtr ServerGame::getBodyEntity(b2Body *body)
{
    std::map<b2Body*, int>::iterator it = body_to_entity_id.find(body);
    if(it == body_to_entity_id.end())
        return EntityPtr();
    return id_to_entity[it->second];
}

/*
 * Box2D will only call this during a step.
 */
void ServerGame::BeginContact(b2Contact *contact)
{
    EntityPtr e1 = getBodyEntity(contact->GetFixtureA()->GetBody());
    EntityPtr e2 = getBodyEntity(contact->GetFixtureB()->GetBody());

    /*
    print("BeginContact: ");
    print(format("e1.id=%1% ") % (e1 ? e1->getID() : -1));
    print(format("e2.id=%1% ") % (e2 ? e2->getID() : -1));
    print("\n");
    */

    if(e1 && e2 && !e1->isDead() && !e2->isDead())
    {
        e1->onHit(e2);
        e2->onHit(e1);
    }
}

/*
 * Box2D can call this inside and outside a timestep (e.g. when a body is destroyed).
 */
void ServerGame::EndContact(b2Contact *contact)
{
    /*
    b2Body *b1 = contact->GetFixtureA()->GetBody();
    b2Body *b2 = contact->GetFixtureB()->GetBody();

    if(b1 == world_bounds_body || b2 == world_bounds_body)
    {
        EntityPtr e = getBodyEntity((b1 == world_bounds_body) ? b2 : b1);
        if(e)
        {
            e->die();
            print(format("entity %1% went out of bounds\n") % e->getID());
        }
    }
    */
}

void ServerGame::PreSolve(b2Contact *contact, const b2Manifold *oldManifold){}
void ServerGame::PostSolve(const b2Contact *contact, const b2ContactImpulse *impulse){}

ParticleSystemPtr ServerGame::getParticleSystem()
{
    return ParticleSystemPtr();
}
