#include "precompiled.h"
#include "particle_system.h"

Emitter::Emitter(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color)
{
    this->position = position;
    this->start_color = start_color;
    this->end_color = end_color;

    respawn_per_tick = 0;
    num_dead = 0;
    min_life = max_life = 50.0f;
}

void Emitter::setPosition(const cml::vector2f &pos)
{
    position = pos;
}

void Emitter::setColor(const cml::vector3f &start_color, const cml::vector3f &end_color)
{
    this->start_color = start_color;
    this->end_color = end_color;
}

void Emitter::setParticleLife(float min_life, float max_life)
{
    this->min_life = min_life;
    this->max_life = max_life;
}

void Emitter::setRespawnRate(int rate)
{
    respawn_per_tick = rate;
}

bool Emitter::isDead() const
{
    return respawn_per_tick <= 0 && num_dead == particles.size();
}

void Emitter::tick()
{
    int num_respawned = 0;
    num_dead = 0;

    for(unsigned int i = 0; i < particles.size(); i++)
    {
        Particle &p = particles[i];

        p.age += 1.0f;

        if(p.age > p.max_age)
            p.dead = true;

        if(p.dead && num_respawned < respawn_per_tick)
        {
            respawn(p);
            num_respawned++;
        }
        else
        {
            p.position += p.velocity;
            float n_life = p.age/p.max_age;
            p.color = cml::lerp(start_color, end_color, n_life);
        }

        if(p.dead)
            num_dead++;
    }
}



ExplosionEmitter::ExplosionEmitter(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color, unsigned int num_pieces, float life, float speed)
:Emitter(position, start_color, end_color)
{
    particles.resize(num_pieces);
    float rot_step = (2.0f*M_PI) / particles.size();
    float rot = 0.0f;

    for(unsigned int i = 0; i < particles.size(); i++)
    {
        Particle &p = particles[i];
        p.position = position;
        p.velocity = cml::rotate_vector_2D(cml::vector2f(speed, 0.0f), rot);

        p.age = 0.0f;
        p.max_age = life;
        p.dead = false;

        rot += rot_step;
    }
}

void ExplosionEmitter::draw()
{
    glBegin(GL_LINES);
    for(unsigned int i = 0; i < particles.size(); i++)
    {
        Particle &p = particles[i];
        if(p.dead) continue;
        cml::vector2f pos2 = p.position + p.velocity;

        glColor3fv(p.color.data());
        glVertex2fv(p.position.data());
        glVertex2fv(pos2.data());
    }
    glEnd();
}

ExplosionEmitterQuads::ExplosionEmitterQuads(const cml::vector2f &position, const cml::vector3f &start_color, const cml::vector3f &end_color, unsigned int num_pieces, float life, float speed)
: ExplosionEmitter(position, start_color, end_color, num_pieces, life, speed)
{
}

void ExplosionEmitterQuads::draw()
{
    glBegin(GL_QUADS);
    for(unsigned int i = 0; i < particles.size(); i++)
    {
        Particle &p = particles[i];
        if(p.dead) continue;
        const cml::vector2f &pos = p.position;

        float size = 1.2f;
        glColor3fv(p.color.data());
        glVertex2f(pos[0]-size, pos[1]-size);
        glVertex2f(pos[0]+size, pos[1]-size);
        glVertex2f(pos[0]+size, pos[1]+size);
        glVertex2f(pos[0]-size, pos[1]+size);
    }
    glEnd();
}



FlameEmitter::FlameEmitter(unsigned int num_particles)
:Emitter(cml::vector2f(0.0f, 0.0f), cml::vector3f(0.0f, 0.0f, 0.0f), cml::vector3f(0.0f, 0.0f, 0.0f))
{
    particles.resize(num_particles);
    respawn_per_tick = 1;
    angle = M_PI/4.0f;
    min_speed = max_speed = 1.0f;
    direction.set(1.0f, 0.0f);
}

void FlameEmitter::respawn(Particle &p)
{
    p.position = position;
    float rot = float(rand())/RAND_MAX * angle * 2.0f;
    rot -= angle;
    float speed = cml::lerp(min_speed, max_speed, float(rand())/RAND_MAX);
    p.velocity = cml::rotate_vector_2D(direction, rot) * speed;
    p.age = 0.0f;
    float l = float(rand())/RAND_MAX * (max_life-min_life);
    p.max_age = min_life + l;
    p.dead = false;
}

void FlameEmitter::draw()
{
    glBegin(GL_QUADS);
    for(unsigned int i = 0; i < particles.size(); i++)
    {
        Particle &p = particles[i];
        if(p.dead) continue;
        cml::vector2f pos2 = p.position + p.velocity;

        float size = 0.2f;
        const cml::vector2f &pos = p.position;
        glColor3fv(p.color.data());
        glVertex2f(pos[0]-size, pos[1]-size);
        glVertex2f(pos[0]+size, pos[1]-size);
        glVertex2f(pos[0]+size, pos[1]+size);
        glVertex2f(pos[0]-size, pos[1]+size);
    }
    glEnd();
}

void FlameEmitter::setDirection(const cml::vector2f dir)
{
    direction = dir;
    direction.normalize();
}

void FlameEmitter::setAngle(float angle)
{
    this->angle = angle;
}

void FlameEmitter::setSpeed(float min_speed, float max_speed)
{
    this->min_speed = min_speed;
    this->max_speed = max_speed;
}




void ParticleSystem::addEmitter(EmitterPtr e)
{
    emitters.push_back(e);
}

void ParticleSystem::tick()
{
    for(unsigned int i = 0; i < emitters.size(); i++)
        emitters[i]->tick();

    unsigned int j = 0;
    while(j < emitters.size())
    {
        if(emitters[j]->isDead())
            emitters.erase(emitters.begin()+j);
        else
            j++;
    }
}

void ParticleSystem::draw()
{
    for(unsigned int i = 0; i < emitters.size(); i++)
        emitters[i]->draw();
}

