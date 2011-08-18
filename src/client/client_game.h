#ifndef AST_CLIENT_GAME_H
#define AST_CLIENT_GAME_H

#include "types.h"
#include "entity/asteroid.h"
#include "entity/entity.h"
#include "network/message.h"

class ClientGame : public IEntityGameCallback
{
private:
    ClientPtr client;
    RenderWindowPtr window;

    std::vector<EntityPtr> entities;
    std::vector<ShipPtr> ships;
    std::vector<AsteroidPtr> asteroids;
    ShipPtr player_ship;
    std::vector<MessagePtr> msgs_out;
    b2WorldPtr physics;

    AsteroidDataCache asteroid_data;

    AABB game_bounds;

    ACameraPtr cam;
    ShipCameraPtr ship_cam;
    FreeCameraPtr free_cam;

    std::vector<cml::vector2f> stars;

    bool has_shaders;
    sf::PostFX glow_fx;
    GLuint main_fb;
    GLuint main_fb_texture;
    GLuint framebuffers[2];
    GLuint framebuffer_textures[2];

    ParticleSystemPtr particle_system;

    std::vector<std::string> asteroid_explosions;
    std::vector<std::string> musics;

    ScoreData scores;

    bool draw_names;
    bool draw_scores;
    bool draw_glow;

public:
    ClientGame(RenderWindowPtr win, GameStateData gamestate_data, ClientPtr client);

    void tick();
    void draw();
    void drawScene();
    void drawShipNames();
    void drawScores();
    void pushCameraTransform();
    void popCameraTransform();
    void initFBO(GLuint *fb_id, GLuint *tex_id, int tex_w, int tex_h);
    void event(const sf::Event &e);

    void processMessage(MessagePtr msg);
    void getOutgoingMessages(std::vector<MessagePtr> *out);


    bool isServer() const;
    b2WorldPtr getPhysics();
    void queueReliableMessageAll(MessagePtr msg);

    BulletPtr spawnBullet(BulletSpawnData &);

    void addBodyToEntity(b2Body *body, int id);
    void removeBodyToEntity(b2Body *body);

    ParticleSystemPtr getParticleSystem();
    void givePlayerPoints(int player_id, int pts){}
};

#endif /* AST_CLIENT_GAME_H */

