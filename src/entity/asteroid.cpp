#include "precompiled.h"
#include "network/message.h"
#include "draw/draw.h"
#include "draw/particle_system.h"
#include "bullet.h"
#include "asteroid.h"

void AsteroidDataCache::load(const std::string &filename)
{
    std::ifstream in(filename.c_str());

    int num = 0;
    in >> num;

    for(int i = 0; i < num; i++)
        loadAsteroid(in);

    if(in.fail())
        warning(format("error loading asteroid data from \"%1%\"\n") % filename);

    in.close();

    print(format("%1% asteroid types loaded\n") % data.size());
}

void AsteroidDataCache::loadAsteroid(std::istream &in)
{
    AsteroidTypeData td;
    int num_polys = 0;

    in >> td.health;
    in >> num_polys;

    for(int j = 0; j < num_polys; j++)
    {
        int num_verts = 0;
        in >> num_verts;

        td.polys.push_back(std::vector<cml::vector2f>());
        std::vector<cml::vector2f> &verts = td.polys.back();

        for(int k = 0; k < num_verts; k++)
        {
            float x, y;
            in >> x;
            in >> y;
            verts.push_back(cml::vector2f(x, y));                
        }
    }

    data.push_back(td);
}

const AsteroidTypeData& AsteroidDataCache::getTypeData(int type) const
{
    if(type < 0 || type >= data.size())
        fatal(format("AsteroidTypeData %1% not found in cache\n") % type);
    return data[type];
}

int AsteroidDataCache::getRandomType() const
{
    return rand() % data.size();
}




Asteroid::Asteroid(IEntityGameCallback *game, const AsteroidSpawnData &data, const AsteroidDataCache &cache)
: Entity(game, data.id, EntityType::ASTEROID)
{
    this->physics = game->getPhysics();
    const AsteroidTypeData &td = cache.getTypeData(data.asteroid_type);

    health = td.health;

    asteroid_type = data.asteroid_type;
    polys = td.polys;

    b2BodyDef b;
    b.type = b2_dynamicBody;
    b.position.Set(data.state.position[0], data.state.position[1]);
    b.angle = data.state.rotation;
    b.active = true;
    body = physics->CreateBody(&b);
    game->addBodyToEntity(body, getID());

    body->SetLinearVelocity(b2Vec2(data.state.velocity[0], data.state.velocity[1]));
    body->SetAngularVelocity(data.state.angular_velocity);

    max_vert_dist = 0.0f;
    for(unsigned int i = 0; i < polys.size(); i++)
    {
        b2PolygonShape shape;
        std::vector<b2Vec2> shape_verts;
        std::vector<cml::vector2f> &verts = polys[i];

        if(verts.empty())
            continue;

        for(unsigned int j = 0; j < verts.size(); j++)
        {
            float x = verts[j][0];
            float y = verts[j][1];
            shape_verts.push_back(b2Vec2(x, y));
            max_vert_dist = std::max(max_vert_dist, verts[j].length());
        }

        shape.Set(&shape_verts[0], shape_verts.size());
        body->CreateFixture(&shape, 0.1f);
    }

    updateBounds();

    if(!on_server)
    {

    }
}

Asteroid::~Asteroid()
{
    game->removeBodyToEntity(body);
    physics->DestroyBody(body);
}

void Asteroid::tick()
{
    updateBounds();
}

void Asteroid::updateBounds()
{
    const b2Vec2 &pos = body->GetPosition();
    bounds.set(cml::vector2f(pos.x-max_vert_dist, pos.y-max_vert_dist), cml::vector2f(pos.x+max_vert_dist, pos.y+max_vert_dist));
}

void Asteroid::draw()
{
    b2Vec2 pos = body->GetPosition();
    glPushMatrix();
    glTranslatef(pos.x, pos.y, 0.0f);
    glRotatef(cml::deg(body->GetAngle()), 0.0f, 0.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for(unsigned int i = 0; i < polys.size(); i++)
        Draw::drawPoly(polys[i]);
    glPopMatrix();
    //Draw::drawAABB(bounds);
}

void Asteroid::onDie()
{
}

void Asteroid::onHit(EntityPtr e)
{
    if(isDead())
        return;

    if(e->getType() == EntityType::BULLET)
    {
        //print("Asteroid hit by Bullet\n");
        health--; 
        print(format("%1%\n") % health);

        if(on_server)
        {
            if(health <= 0)
            {
                die();
                game->givePlayerPoints(static_cast<Bullet*>(e.get())->getShooterID(), 1);
            }
        }
    }
}

void Asteroid::processMessage(MessagePtr msg)
{
    switch(msg->getType())
    {
        //case SRV_ASTEROID_STATE: break;
        //case SRV_ASTEROID_ON_DIE: break;
        default: break;
    }
}

void Asteroid::getSpawnData(AsteroidSpawnData *data) const
{
    data->id = getID();
    data->asteroid_type = asteroid_type;
    getStateData(&data->state);
}

void Asteroid::setStateData(const AsteroidStateData &data)
{
    body->SetTransform(b2Vec2(data.position[0], data.position[1]), data.rotation);
    body->SetLinearVelocity(b2Vec2(data.velocity[0], data.velocity[1]));
    body->SetAngularVelocity(data.angular_velocity);

    health = data.health;
}

void Asteroid::getStateData(AsteroidStateData *data) const
{
    b2Vec2 pos = body->GetPosition();
    b2Vec2 vel = body->GetLinearVelocity();

    data->position.set(pos.x, pos.y);
    data->velocity.set(vel.x, vel.y);
    data->rotation = body->GetAngle();
    data->angular_velocity = body->GetAngularVelocity();
    data->health = health;
}

cml::vector2f Asteroid::getPosition() const
{
    b2Vec2 pos = body->GetPosition();
    return cml::vector2f(pos.x, pos.y);
}

