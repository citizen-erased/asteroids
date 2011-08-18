#include "precompiled.h"
#include "network/message.h"
#include "draw/particle_system.h"
#include "bullet.h"
#include "ship.h"

Ship::Ship(IEntityGameCallback *game, const ShipSpawnData &data)
: Entity(game, data.id, EntityType::SHIP)
{
    this->physics = game->getPhysics();
    player_id = data.player_id;
    body_color = data.body_color;
    name = data.name;
    tick_num = 0;
    shot_ticks = 15;
    last_shot_tick = 0;

    b2BodyDef b;
    b.type = b2_dynamicBody;
    b.position.Set(data.state.position[0], data.state.position[1]);
    b.angle = data.state.rotation;
    b.active = true;
    body = physics->CreateBody(&b);
    game->addBodyToEntity(body, getID());

    b2PolygonShape shape;
    b2Vec2 verts[3];
    verts[0].Set(-1, -1);
    verts[1].Set( 1, -1);
    verts[2].Set( 0, 2);
    shape.Set(verts, 3);
    body->CreateFixture(&shape, 0.1f);

    body->SetLinearVelocity(b2Vec2(data.state.velocity[0], data.state.velocity[1]));
    body->SetLinearDamping(1.5f);
    body->SetAngularDamping(5.0f);

    forward = reverse = right = left = shooting = false;

    updateBounds();

    if(!on_server)
    {
        flame1 = FlameEmitterPtr(new FlameEmitter(30));
        flame1->setColor(cml::vector3f(1.0f, 0.0f, 0.0f), cml::vector3f(0.3f, 0.3f, 0.0f));
        flame1->setParticleLife(20, 35);
        flame1->setParticleLife(10, 15);
        flame1->setSpeed(0.1f, 0.25f);
        flame1->setRespawnRate(4);

        flame2 = FlameEmitterPtr(new FlameEmitter(*flame1));

        game->getParticleSystem()->addEmitter(flame1);
        game->getParticleSystem()->addEmitter(flame2);
    }
}

Ship::~Ship()
{
    game->removeBodyToEntity(body);
    physics->DestroyBody(body);
}

void Ship::tick()
{
    tick_num++;

    updateBounds();

    if(right)
        //body->SetAngularVelocity(-0.5f);
        body->ApplyAngularImpulse(-1.0f);
        //body->ApplyTorque(100.1f);
    else if(left)
        body->ApplyAngularImpulse(1.0f);
        //body->SetAngularVelocity(0.5f);

    float max_ang_vel = 3.0f;
    if(body->GetAngularVelocity() > max_ang_vel)
        body->SetAngularVelocity(max_ang_vel);
    else if(body->GetAngularVelocity() < -max_ang_vel)
        body->SetAngularVelocity(-max_ang_vel);

    //print(format("angular vel= %1%\n") % body->GetAngularVelocity());

    if(forward)
    {
        cml::vector2f accel(0.0f, 2.0f);
        accel = cml::rotate_vector_2D(accel, body->GetAngle());
        body->ApplyLinearImpulse(b2Vec2(accel[0], accel[1]), body->GetWorldCenter());
    }
    else if(reverse)
    {
        cml::vector2f accel(0.0f, -1.0f);
        accel = cml::rotate_vector_2D(accel, body->GetAngle());
        body->ApplyLinearImpulse(b2Vec2(accel[0], accel[1]), body->GetWorldCenter());
    }

    if(body->GetLinearVelocity().LengthSquared() > 64.0f*64.0f)
    {
        b2Vec2 v = body->GetLinearVelocity();
        v.Normalize();
        v *= 64.0f;
        body->SetLinearVelocity(v);
    }

    if(on_server && shooting && last_shot_tick+shot_ticks < tick_num)
    {
        last_shot_tick = tick_num;
        b2Vec2 pos = body->GetPosition();

        BulletSpawnData d;
        d.shooter_id = player_id;
        d.state.position = cml::rotate_vector_2D(cml::vector2f(0.0f, 2.5f), body->GetAngle()) + cml::vector2f(pos.x, pos.y);
        d.state.velocity = cml::rotate_vector_2D(cml::vector2f(0.0f, 1000.0f), body->GetAngle());
        d.state.rotation = body->GetAngle();

        game->spawnBullet(d);
    }

    if(flame1 && flame2)
    {
        float angle = body->GetAngle();
        cml::vector2f dir_vec = cml::rotate_vector_2D(cml::vector2f(0.0f, -1.0f), angle);
        flame1->setDirection(dir_vec);
        flame2->setDirection(dir_vec);

        cml::vector2f offset1 = cml::rotate_vector_2D(cml::vector2f(0.5f, -2.0f), angle);
        cml::vector2f offset2 = cml::rotate_vector_2D(cml::vector2f(-0.5f, -2.0f), angle);
        flame1->setPosition(getPosition()+offset1);
        flame2->setPosition(getPosition()+offset2);
    }

    //print(format("%1%\n") % body->GetLinearVelocity().Length());
}

void Ship::updateBounds()
{
    const b2Vec2 &pos = body->GetPosition();
    bounds.set(cml::vector2f(pos.x-2.0f, pos.y-2.0f), cml::vector2f(pos.x+2.0f, pos.y+2.0f));
}

void Ship::draw()
{
    b2Vec2 pos = body->GetPosition();
    glPushMatrix();
    glTranslatef(pos.x, pos.y, 0.0f);
    glRotatef(cml::deg(body->GetAngle()), 0.0f, 0.0f, 1.0f);
    glColor4fv(body_color.data());
    glBegin(GL_LINE_LOOP);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(0.0f, 2.0f);
    glEnd();
    glPopMatrix();
}

void Ship::onDie()
{
    if(on_server)
    {
        //send die message
    }
    else
    {
        //spawn death effects
    }

    if(flame1) flame1->setRespawnRate(0);
    if(flame2) flame2->setRespawnRate(0);
}

void Ship::onHit(EntityPtr e)
{
    if(isDead()) return;

    if(e->getType() == EntityType::ASTEROID
        || e->getType() == EntityType::BULLET)
    {
        if(on_server)
        {
            die();
            game->givePlayerPoints(player_id, -1);
            if(e->getType() == EntityType::BULLET)
                game->givePlayerPoints(static_cast<Bullet*>(e.get())->getShooterID(), 5);
        }
    }
    //health -= something
    //if(health <= 0) die();
}

void Ship::processMessage(MessagePtr msg)
{
    switch(msg->getType())
    {
        //case SRV_ASTEROID_STATE: break;
        //case SRV_ASTEROID_ON_DIE: break;
        default: break;
    }
}

void Ship::getSpawnData(ShipSpawnData *data) const
{
    data->id = getID();
    data->player_id = player_id;
    data->body_color = body_color;
    data->name = name;
    getStateData(&data->state);
}

void Ship::setStateData(const ShipStateData &data)
{
    body->SetTransform(b2Vec2(data.position[0], data.position[1]), data.rotation);
    body->SetLinearVelocity(b2Vec2(data.velocity[0], data.velocity[1]));
    body->SetAngularVelocity(data.angular_velocity);
}

void Ship::getStateData(ShipStateData *data) const
{
    b2Vec2 pos = body->GetPosition();
    b2Vec2 vel = body->GetLinearVelocity();

    data->position.set(pos.x, pos.y);
    data->velocity.set(vel.x, vel.y);
    data->rotation = body->GetAngle();
    data->angular_velocity = body->GetAngularVelocity();
    data->health = 100;
}

void Ship::getControlData(ShipControlData *data) const
{
    data->forward = forward;
    data->reverse = reverse;
    data->left = left;
    data->right = right;
    data->shooting = shooting;
}

void Ship::setControlData(const ShipControlData &data)
{
    forward = data.forward;
    reverse = data.reverse;
    left = data.left;
    right = data.right;
    shooting = data.shooting;
}

cml::vector2f Ship::getPosition() const
{
    b2Vec2 pos = body->GetPosition();
    return cml::vector2f(pos.x, pos.y);
}

cml::vector4f Ship::getBodyColor() const
{
    return body_color;
}

const std::string& Ship::getName() const
{
    return name;
}

unsigned int Ship::getPlayerID() const
{
    return player_id;
}

void Ship::event(const sf::Event &e)
{
    if(e.Type == sf::Event::KeyPressed)
    {
        switch(e.Key.Code)
        {
            case sf::Key::W: forward = true; break;
            case sf::Key::S: reverse = true; break;
            case sf::Key::A: left = true; break;
            case sf::Key::D: right = true; break;
            case sf::Key::Space: shooting = true; break;
            default: break;
        }
    }
    else if(e.Type == sf::Event::KeyReleased)
    {
        switch(e.Key.Code)
        {
            case sf::Key::W: forward = false; break;
            case sf::Key::S: reverse = false; break;
            case sf::Key::A: left = false; break;
            case sf::Key::D: right = false; break;
            case sf::Key::Space: shooting = false; break;
            default: break;
        }
    }
}

