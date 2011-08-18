#include "precompiled.h"
#include "audio/audio.h"
#include "entity/asteroid.h"
#include "entity/bullet.h"
#include "entity/ship.h"
#include "draw/draw.h"
#include "draw/particle_system.h"
#include "camera.h"
#include "client.h"
#include "client_game.h"

ClientGame::ClientGame(RenderWindowPtr win, GameStateData gamestate_data, ClientPtr client)
{
    this->client = client;
    this->window = win;

    physics = b2WorldPtr(new b2World(b2Vec2(0.0f, 0.0f), true));
    game_bounds = gamestate_data.game_bounds;

    ship_cam = ShipCameraPtr(new ShipCamera());
    free_cam = FreeCameraPtr(new FreeCamera(0, 0));
    cam = ship_cam;

    asteroid_data.load("asteroids.dat");

    asteroid_explosions.push_back("sounds/explosion1.wav");
    asteroid_explosions.push_back("sounds/explosion2.wav");
    asteroid_explosions.push_back("sounds/explosion3.wav");

    musics.push_back("sounds/stitchesinthecorpse_mymountaininstrumentalversion.ogg");
    musics.push_back("sounds/alone_to_die.ogg");

    stars.resize(256);
    for(int i = 0; i < stars.size(); i++)
        stars[i].set(float(rand())/RAND_MAX * game_bounds.getWidth(), float(rand())/RAND_MAX * game_bounds.getHeight());

    particle_system = ParticleSystemPtr(new ParticleSystem());

    if(sf::PostFX::CanUsePostFX())
    {
        if(glow_fx.LoadFromFile("glow.sfx"))
        {
            has_shaders = true;

            //glow_fx.SetTexture("framebuffer", NULL);
        }
        else
        {
            warning("failed to load glow.sfx\n");
            has_shaders = false;
        }

        int w = window->GetWidth();
        int h = window->GetHeight();
        initFBO(&main_fb, &main_fb_texture, w, h);
        initFBO(&framebuffers[0], &framebuffer_textures[0], w, h);
        initFBO(&framebuffers[1], &framebuffer_textures[1], w, h);
    }
    else
    {
        has_shaders = false;
    }

    draw_names = false;
    draw_scores = false;
    draw_glow = true;
}

void ClientGame::tick()
{
    msgs_out.clear();

    physics->Step(1.0f/60.0f, 10, 8);
    physics->ClearForces();

    for(unsigned int i = 0; i < entities.size(); i++)
        entities[i]->tick();

    if(player_ship)
    {
        //print("ClientGame: sending ship control data\n");
        ShipControlData control_data;
        player_ship->getControlData(&control_data);
        MessagePtr msg(new ShipControlMessage(control_data));
        client->queueUnreliableMessage(msg);
    }

    cam->tick();
    const cml::vector2f &cam_pos = cam->getPosition();
    sf::Listener::SetPosition(cam_pos[0], cam_pos[1], 0.0f);
    particle_system->tick();

    if(!Audio::isMusicPlaying() && !musics.empty())
        Audio::playMusic(musics[rand()%musics.size()]);
}

void ClientGame::processMessage(MessagePtr msg)
{
    switch(msg->getType())
    {
        case MessageType::SRV_ASTEROID_SPAWN:
        {
            print("ClientGame: spawning asteroid\n"); 
            AsteroidSpawnMessage *spawn_msg = static_cast<AsteroidSpawnMessage*>(msg.get());
            AsteroidPtr ast(new Asteroid(this, spawn_msg->getData(), asteroid_data));
            entities.push_back(ast);
            asteroids.push_back(ast);
        }
        break;
        case MessageType::SRV_SHIP_SPAWN:
        {
            ShipSpawnMessage *spawn_msg = static_cast<ShipSpawnMessage*>(msg.get());
            const ShipSpawnData &d = spawn_msg->getData();

            ShipPtr ship(new Ship(this, d));
            entities.push_back(ship);
            ships.push_back(ship);

            print(format("ClientGame: spawning ship id=%1% name=%2%\n") % ship->getID() % ship->getName());
        }
        break;
        case MessageType::SRV_BULLET_SPAWN:
        {
            BulletSpawnMessage *spawn_msg = static_cast<BulletSpawnMessage*>(msg.get());
            const BulletSpawnData &d = spawn_msg->getData();

            BulletPtr bullet(new Bullet(this, d));
            entities.push_back(bullet);
            if(Audio::distToListener(d.state.position) < 200.0f)
                Audio::playSFX("sounds/laser0.wav", false, d.state.position, 0.5f, 10.0f);
            print(format("ClientGame: spawning bullet id=%1%\n") % bullet->getID());
        }
        break;
        case MessageType::SRV_SHIP_STATE:
        {
            ShipStateMessage *state_msg = static_cast<ShipStateMessage*>(msg.get());
            int id = state_msg->getEntityID();

            //print(format("ClientGame: received ship state data for ship id=%1%\n") % id);

            //TODO use a map?
            for(unsigned int i = 0; i < ships.size(); i++)
                if(ships[i]->getID() == id)
                {
                    ships[i]->setStateData(state_msg->getData());
                    break;
                }
        }
        break;
        case MessageType::SRV_SHIP_OWNER:
        {
            ShipOwnerMessage *owner_msg = static_cast<ShipOwnerMessage*>(msg.get());
                    print("owner message\n");

            for(unsigned int i = 0; i < ships.size(); i++)
                if(ships[i]->getID() == owner_msg->getData().ship_id)
                {
                    print("owner message\n");
                    player_ship = ships[i];
                    ship_cam->setShip(player_ship);
                    break;
                }
        }
        case MessageType::SRV_ASTEROID_STATE:
        {
            AsteroidStateMessage *state_msg = static_cast<AsteroidStateMessage*>(msg.get());
            int id = state_msg->getEntityID();

            //print(format("ClientGame: received asteroid state data for asteroid id=%1%\n") % id);

            for(unsigned int i = 0; i < asteroids.size(); i++)
                if(asteroids[i]->getID() == id)
                {
                    asteroids[i]->setStateData(state_msg->getData());
                    break;
                }
        }
        break;
        case MessageType::SRV_ENTITY_DELETE:
        {
            print("ClientGame: deleting entity\n"); 
            EntityDeleteMessage *delete_msg = static_cast<EntityDeleteMessage*>(msg.get());
            int id = delete_msg->getData().entity_id;

            for(unsigned int i = 0; i < entities.size(); i++)
                if(entities[i]->getID() == id)
                {
                    entities[i]->die();
                    entities.erase(entities.begin()+i);
                    break;
                }

            if(player_ship && id == player_ship->getID())
                player_ship.reset();

            //TODO do this properly 
            for(unsigned int i = 0; i < ships.size(); i++)
                if(ships[i]->getID() == id)
                {
                    cml::vector2f pos = ships[i]->getPosition();
                    cml::vector3f col1(ships[i]->getBodyColor().data());
                    cml::vector3f col2 = cml::vector3f(0.0f, 0.0f, 0.0f);
                    ExplosionEmitterPtr e(new ExplosionEmitter(pos, col1, col2, 40, 50.0f, 1.6f));
                    particle_system->addEmitter(e);
                    if(Audio::distToListener(pos) < 200.0f)
                        Audio::playSFX("sounds/explosion0.wav", false, pos, 0.1f, 10.0f);

                    ships.erase(ships.begin()+i);
                    break;
                }

            for(unsigned int i = 0; i < asteroids.size(); i++)
                if(asteroids[i]->getID() == id)
                {
                    AsteroidPtr a = asteroids[i];
                    if(a->getBounds().intersects(game_bounds))
                    {
                        cml::vector2f pos = a->getPosition();
                        cml::vector3f col1 = cml::vector3f(1.0f, 0.0f, 0.0f);
                        cml::vector3f col2 = cml::vector3f(0.1f, 0.0f, 0.0f);
                        ExplosionEmitterPtr e(new ExplosionEmitter(pos, col1, col2, 50, 30.0f, 1.6f));
                        particle_system->addEmitter(e);

                        col1.set(0.2f, 0.2f, 0.0f);
                        col2.set(0.0f, 0.0f, 0.0f);
                        ExplosionEmitterQuadsPtr e2(new ExplosionEmitterQuads(pos, col1, col2, 30, 20.0f, 1.6f));
                        particle_system->addEmitter(e2);

                        if(Audio::distToListener(pos) < 300.0f)
                            Audio::playSFX(asteroid_explosions[rand()%asteroid_explosions.size()], false, pos, 0.01f, 1.0f);
                    }
                    asteroids.erase(asteroids.begin()+i);
                    break;
                }

            /*
            for(unsigned int i = 0; i < bullets.size(); i++)
                if(bullets[i]->getID() == id)
                {
                    bullets.erase(bullets.begin()+i);
                    break;
                }
            */
        }
        break;
        case MessageType::SRV_SCORE:
        {
            ScoreMessage *score_msg = static_cast<ScoreMessage*>(msg.get());
            scores = score_msg->getData();
        }
        break;
        default: break;
    }
}

void ClientGame::getOutgoingMessages(std::vector<MessagePtr> *msgs)
{
    msgs->insert(msgs->end(), msgs_out.begin(), msgs_out.end());
}

void ClientGame::draw()
{
    //print("begin draw\n");
    if(has_shaders && draw_glow)
    {
        //print("drawing with shaders\n");
        //print(format("%1% %2% %3%\n") % main_fb% framebuffers[0]% framebuffers[1]);

        glBindFramebuffer(GL_FRAMEBUFFER, main_fb);
        drawScene();

        //glBindTexture(GL_TEXTURE_2D, main_fb_texture);

        //bind fb1
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
        glBindTexture(GL_TEXTURE_2D, main_fb_texture);
        glow_fx.SetParameter("framebuffer", main_fb_texture);
        glow_fx.SetParameter("factor", 0.5f);
        glow_fx.SetParameter("step", 1.0f/window->GetWidth(), 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        window->Draw(glow_fx);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
        glBindTexture(GL_TEXTURE_2D, framebuffer_textures[0]);
        glow_fx.SetParameter("framebuffer", framebuffer_textures[0]);
        glow_fx.SetParameter("factor", 0.5f);
        glow_fx.SetParameter("step", 0.0f, 1.0f/window->GetHeight());
        glClear(GL_COLOR_BUFFER_BIT);
        window->Draw(glow_fx);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBindTexture(GL_TEXTURE_2D, main_fb_texture);
        Draw::drawTexturedRect(0, 0, window->GetWidth(), window->GetHeight());
        //glBindTexture(GL_TEXTURE_2D, framebuffer_textures[0]);
        //Draw::drawTexturedRect(0, 0, window->GetWidth(), window->GetHeight());
        glBindTexture(GL_TEXTURE_2D, framebuffer_textures[1]);
        Draw::drawTexturedRect(0, 0, window->GetWidth(), window->GetHeight());
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
    else
    {
        //print("drawing without shaders\n");
        drawScene();
    }



    if(draw_names) drawShipNames();
    if(draw_scores) drawScores();
    //print("end draw\n");
}

void ClientGame::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT);

    pushCameraTransform(); 
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    Draw::drawAABB(game_bounds);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for(int i = 0; i < stars.size(); i++)
    {
        glBegin(GL_POINTS);
        glVertex2fv(stars[i].data());
        glEnd();
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for(unsigned int i = 0; i < entities.size(); i++)
        entities[i]->draw();

    particle_system->draw();
    popCameraTransform();
}

void ClientGame::drawShipNames()
{
    float scale = 5.0f;
    if(cam == free_cam)
        scale = free_cam->getScale();
    const cml::vector2f &cam_pos = cam->getPosition();
    float hw = window->GetWidth() / 2.0f / scale;
    float hh = window->GetHeight() / 2.0f / scale;

    pushCameraTransform();
    for(unsigned int i = 0; i < ships.size(); i++)
    {
        sf::String str(ships[i]->getName(), sf::Font::GetDefaultFont(), 20);
        cml::vector2f pos = ships[i]->getPosition();
        pos -= cam_pos;
        pos[0] += hw; pos[1] += hh;
        str.SetPosition(pos[0]*scale, window->GetHeight() - pos[1]*scale);
        window->Draw(str);
    }
    popCameraTransform();
}

void ClientGame::drawScores()
{
    std::stringstream ss_names;
    std::stringstream ss_scores;

    for(unsigned int i = 0; i < scores.names.size(); i++)
    {
        ss_names << scores.names[i] << "\n";
        ss_scores << scores.scores[i] << "\n";
    }

    sf::String str_names(ss_names.str(), sf::Font::GetDefaultFont(), 20);
    sf::String str_scores(ss_scores.str(), sf::Font::GetDefaultFont(), 20);

    int spacing = 10;
    int name_w = str_names.GetRect().GetWidth();
    int score_w = str_scores.GetRect().GetWidth();
    int cw = window->GetWidth()/2;

    str_names.SetX(cw - name_w - spacing);
    str_scores.SetX(cw + spacing);

    int h = window->GetHeight();
    int name_h = str_names.GetRect().GetHeight();

    str_names.SetY(h/2 - name_h/2);
    str_scores.SetY(h/2 - name_h/2);

    sf::FloatRect r1 = str_names.GetRect();
    sf::FloatRect r2 = str_scores.GetRect();
    float padding = 15.0f;
    float x1 = std::min(r1.Left, r2.Left) - padding;
    float y1 = std::min(r1.Top, r2.Top) - padding;
    float x2 = std::max(r1.Right, r2.Right) + padding;
    float y2 = std::max(r1.Bottom, r2.Bottom) + padding;
    sf::Shape rect = sf::Shape::Rectangle(x1, y1, x2, y2, sf::Color(0, 0, 0, 180), 3.0f, sf::Color(100, 100, 100, 255)); 

    window->Draw(rect);
    window->Draw(str_names);
    window->Draw(str_scores);
}

void ClientGame::pushCameraTransform()
{
    glPushMatrix();
    float scale = 5.0f;
    if(cam == free_cam)
        scale = free_cam->getScale();

    glScalef(scale, scale, scale);

    const cml::vector2f &cam_pos = cam->getPosition();
    float hw = window->GetWidth() / 2.0f / scale;
    float hh = window->GetHeight() / 2.0f / scale;
    glTranslatef(-cam_pos[0]+hw, -cam_pos[1]+hh, 0.0f);
}

void ClientGame::popCameraTransform()
{
    glPopMatrix();
}

void ClientGame::initFBO(GLuint *fb_id, GLuint *tex_id, int tex_w, int tex_h)
{
    glGenTextures(1, tex_id);
    glBindTexture(GL_TEXTURE_2D, *tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);

    glGenFramebuffers(1, fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *fb_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex_id, 0);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        warning("incomplete framebuffer\n");
        has_shaders = false;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ClientGame::event(const sf::Event &e)
{
    //print(format("player_ship=%1%\n") % player_ship);
    if(player_ship)
        player_ship->event(e);

    cam->event(e);

    if(e.Type == sf::Event::KeyPressed)
    {
        if(e.Key.Code == sf::Key::C)
        {
            if(cam == ship_cam)
            {
                cam = free_cam;
                free_cam->setPosition(ship_cam->getPosition());
                free_cam->setScale(5.0f);
            }
            else
                cam = ship_cam;
        }
        else if(e.Key.Code == sf::Key::Num1)
            draw_names = !draw_names;
        else if(e.Key.Code == sf::Key::Num2)
            draw_glow = !draw_glow;
        else if(e.Key.Code == sf::Key::Tab)
            draw_scores = true;
    }
    else if(e.Type == sf::Event::KeyReleased)
    {
        if(e.Key.Code == sf::Key::Tab)
            draw_scores = false;
    }
    else if(e.Type == sf::Event::Resized)
    {
        if(has_shaders)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, main_fb);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

            glDeleteTextures(1, &main_fb_texture);
            glDeleteTextures(2, framebuffer_textures);
            glDeleteFramebuffers(1, &main_fb);
            glDeleteFramebuffers(2, framebuffers);

            int w = e.Size.Width;
            int h = e.Size.Height;
            initFBO(&main_fb, &main_fb_texture, w, h);
            initFBO(&framebuffers[0], &framebuffer_textures[0], w, h);
            initFBO(&framebuffers[1], &framebuffer_textures[1], w, h);
        }
    }
}

bool ClientGame::isServer() const
{
    return false;
}

b2WorldPtr ClientGame::getPhysics()
{
    return physics;
}

BulletPtr ClientGame::spawnBullet(BulletSpawnData &)
{
    return BulletPtr();
}

void ClientGame::addBodyToEntity(b2Body *body, int id)
{
    //TODO
}

void ClientGame::removeBodyToEntity(b2Body *body)
{
    //TODO
}

ParticleSystemPtr ClientGame::getParticleSystem()
{
    return particle_system;
}

