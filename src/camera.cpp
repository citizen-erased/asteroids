#include "precompiled.h"
#include "entity/ship.h"
#include "camera.h"

ACamera::ACamera()
{

}

ACamera::ACamera(float x, float y)
{
    position.set(x, y);
}

void ACamera::setPosition(const cml::vector2f &pos)
{
    position = pos;
}

const cml::vector2f& ACamera::getPosition() const
{
    return position;
}


FreeCamera::FreeCamera(float x, float y)
: ACamera(x, y)
{
    up = down = left = right = in = out = false;    
    speed = 4.0f;
    scale_speed = 0.05f;
    scale = 1.0f;
}

void FreeCamera::setScale(float scale)
{
    this->scale = scale;
}

float FreeCamera::getScale() const
{
    return scale;
}

void FreeCamera::tick()
{
    if(up)
        position[1] += speed;
    else if(down)
        position[1] -= speed;

    if(left)
        position[0] -= speed;
    else if(right)
        position[0] += speed;

    if(in)
        scale += scale_speed;
    else if(out)
        scale -= scale_speed;
}

void FreeCamera::event(const sf::Event &e)
{
    if(e.Type == sf::Event::KeyPressed)
    {
        switch(e.Key.Code)
        {
            case sf::Key::Up:    up    = true; break;
            case sf::Key::Down:  down  = true; break;
            case sf::Key::Left:  left  = true; break;
            case sf::Key::Right: right = true; break;
            case sf::Key::Add:      in  = true; break;
            case sf::Key::Subtract: out = true; break;
            default: break;
        }
    }
    else if(e.Type == sf::Event::KeyReleased)
    {
        switch(e.Key.Code)
        {
            case sf::Key::Up:    up    = false; break;
            case sf::Key::Down:  down  = false; break;
            case sf::Key::Left:  left  = false; break;
            case sf::Key::Right: right = false; break;
            case sf::Key::Add:      in  = false; break;
            case sf::Key::Subtract: out = false; break;
            default: break;
        }
    }
}

void ShipCamera::tick()
{
    if(!ship) return;

    if(ship->isDead())
        ship.reset();
    else
    {
        position = ship->getPosition();
    }

}

void ShipCamera::setShip(ShipPtr ship)
{
    this->ship = ship;
}

