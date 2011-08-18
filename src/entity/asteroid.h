#ifndef AST_ASTEROID_H
#define AST_ASTEROID_H

#include "types.h"
#include "entity.h"

struct AsteroidTypeData
{
    std::vector< std::vector<cml::vector2f> > polys;
    int health;
};

class AsteroidDataCache
{
private:
    std::vector<AsteroidTypeData> data;

public:
    void load(const std::string &filename);
    void loadAsteroid(std::istream &in);
    const AsteroidTypeData& getTypeData(int type) const;
    int getRandomType() const;
};


class Asteroid : public Entity
{
private:
    b2WorldPtr physics;
    b2Body  *body;
    int asteroid_type;
    int health;

    std::vector< std::vector<cml::vector2f> > polys;
    float max_vert_dist;

public:
    Asteroid(IEntityGameCallback *game, const AsteroidSpawnData &data, const AsteroidDataCache &cache);
    ~Asteroid();

    void tick();
    void draw();

    void onDie();

    void onHit(EntityPtr e);

    void processMessage(MessagePtr msg);

    void getSpawnData(AsteroidSpawnData *data) const;
    void getStateData(AsteroidStateData *data) const;
    void setStateData(const AsteroidStateData &data);

    cml::vector2f getPosition() const;

private:
    void updateBounds();
};

#endif /* AST_ASTEROID_H */

