/*
 * This file forward declares common classes and typedefs shared pointers for them.
 * Should be included by headers that can forward declare the types it needs rather
 * than including their headers. This helps speed up compilation and keeps things
 * a bit cleaner.
 */

#define type(t) class t; typedef boost::shared_ptr<t> t##Ptr;

/* client/server games */
type(Client);
type(Server);
type(ClientGame);
type(ServerGame);

/* entities */
type(Entity);
type(Asteroid);
type(Bullet);
type(Ship);

/* network */
type(Message);
type(MessageBuffer);
type(Packet);

/* game states */
type(GameState);
type(MenuState);
type(ClientState);
type(ClientServerState);

/* game */
type(Game);

/* particles */
type(Emitter);
type(ExplosionEmitter);
type(ExplosionEmitterQuads);
type(FlameEmitter);
type(ParticleSystem);

/* Menus */
type(MainMenu);

/* Physics */
type(b2World);

/* Camera */
type(ACamera)
type(FreeCamera)
type(ShipCamera)

/* Math */
type(AABB);

#undef type

namespace sf
{
class Event;
class RenderWindow;
class String;
class Font;
class View;
}

typedef boost::shared_ptr<sf::RenderWindow> RenderWindowPtr;

namespace gcn
{
class Gui;
class OpenGLGraphics;
class Container;
class Graphics;
}

