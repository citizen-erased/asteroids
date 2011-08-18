#include "precompiled.h"
#include "network/message.h"
#include "bullet.h"

Bullet::Bullet(IEntityGameCallback *game, const BulletSpawnData &data)
: Entity(game, data.id, EntityType::BULLET)
{
    this->physics = game->getPhysics();
    shooter_id = data.shooter_id;

    b2BodyDef b;
    b.type = b2_dynamicBody;
    b.position.Set(data.state.position[0], data.state.position[1]);
    b.angle = data.state.rotation;
    b.active = true;
    body = physics->CreateBody(&b);
    game->addBodyToEntity(body, getID());

    body->SetLinearVelocity(b2Vec2(data.state.velocity[0], data.state.velocity[1]));

    b2PolygonShape shape;
    shape.SetAsEdge(b2Vec2(0.0f, 0.0f), b2Vec2(0.0f, 1.0f));
    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 0.0f;
    fixture.friction = 0.0f;
    fixture.isSensor = true;
    body->CreateFixture(&fixture);

    updateBounds();
}

Bullet::~Bullet()
{
    game->removeBodyToEntity(body);
    physics->DestroyBody(body);
}

void Bullet::tick()
{
    updateBounds();
}

void Bullet::updateBounds()
{
    const b2Vec2 &pos = body->GetPosition();
    bounds.set(cml::vector2f(pos.x-0.1f, pos.y), cml::vector2f(pos.x+0.1f, pos.y));
}

void Bullet::draw()
{
    b2Vec2 pos = body->GetPosition();
    glPushMatrix();
    glTranslatef(pos.x, pos.y, 0.0f);
    glRotatef(cml::deg(body->GetAngle()), 0.0f, 0.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f,  0.0f);
        glVertex2f(0.0f,  1.0f);
    glEnd();
    glPopMatrix();
}

void Bullet::onDie()
{

}
void Bullet::onHit(EntityPtr e)
{
    if(isDead())
        return;

    if(e->getType() == EntityType::ASTEROID
        || e->getType() == EntityType::SHIP)
    {
        die();
    }
}

void Bullet::getSpawnData(BulletSpawnData *data) const
{
    data->id = getID();
    data->shooter_id = shooter_id;
    getStateData(&data->state);
}

void Bullet::getStateData(BulletStateData *data) const
{
    b2Vec2 pos = body->GetPosition();
    b2Vec2 vel = body->GetLinearVelocity();

    data->position.set(pos.x, pos.y);
    data->velocity.set(vel.x, vel.y);
    data->rotation = body->GetAngle();
}

void Bullet::setStateData(const BulletStateData &data)
{
    body->SetTransform(b2Vec2(data.position[0], data.position[1]), data.rotation);
    body->SetLinearVelocity(b2Vec2(data.velocity[0], data.velocity[1]));
}

int Bullet::getShooterID() const
{
    return shooter_id;
}

