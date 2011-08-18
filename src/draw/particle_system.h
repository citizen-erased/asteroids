#ifndef AST_PARTICLE_SYSTEM_H
#define AST_PARTICLE_SYSTEM_H

#include <cml/cml.h>
#include "types.h"

struct Particle
{
    cml::vector2f position;
    cml::vector2f velocity;
    cml::vector3f color;
    float age, max_age;
    bool dead;
};

class Emitter
{
protected:
    cml::vector2f position;
    std::vector<Particle> particles;
    cml::vector3f start_color, end_color;
    int respawn_per_tick;
    unsigned int num_dead;
    float min_life, max_life;

public:
    Emitter(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color);

    void setPosition(const cml::vector2f &pos);
    void setColor(const cml::vector3f &start_color, const cml::vector3f &end_color);
    void setParticleLife(float min_life, float max_life);
    void setRespawnRate(int rate);

    bool isDead() const;

    virtual void tick();
    virtual void draw() = 0;

protected:
    virtual void respawn(Particle &){};
};



class ExplosionEmitter : public Emitter
{ 
public:
    ExplosionEmitter(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color, unsigned int num_pieces, float life, float speed);

    virtual void draw();
};

class ExplosionEmitterQuads : public ExplosionEmitter
{ 
public:
    ExplosionEmitterQuads(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color, unsigned int num_pieces, float life, float speed);

    virtual void draw();
};


class FlameEmitter : public Emitter
{ 
private:
    cml::vector2f direction;
    float angle;
    float min_speed, max_speed;

public:
    FlameEmitter(unsigned int num_particles);

    void setDirection(const cml::vector2f dir);
    void setAngle(float angle);
    void setSpeed(float min_speed, float max_speed);

    virtual void draw();

protected:
    void respawn(Particle &);
};


class ParticleSystem
{
private:
    std::vector<EmitterPtr> emitters;

public:
    void addEmitter(EmitterPtr e);

    void tick();
    void draw();
};

#endif /* AST_PARTICLE_SYSTEM_H */

