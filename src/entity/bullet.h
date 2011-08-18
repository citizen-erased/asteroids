#ifndef AST_BULLET_H
#define AST_BUTTLET_H

#include "types.h"
#include "entity.h"

class Bullet : public Entity
{
private:
    b2WorldPtr physics;
    b2Body  *body;
    int shooter_id;

public:
    Bullet(IEntityGameCallback *game, const BulletSpawnData &data);
    ~Bullet();

    void tick();
    void draw();

    void onDie();
    void onHit(EntityPtr e);

    void getSpawnData(BulletSpawnData *data) const;
    void getStateData(BulletStateData *data) const;
    void setStateData(const BulletStateData &data);

    int getShooterID() const;

private:
    void updateBounds();
};

#endif /* AST_BULLET_H */

