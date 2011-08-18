#ifndef AST_CAMERA_H
#define AST_CAMERA_H

class ACamera
{
protected:
    cml::vector2f position;

public:
    ACamera();
    ACamera(float x, float y);

    virtual void tick(){};
    virtual void event(const sf::Event &){};

    void setPosition(const cml::vector2f &pos);
    const cml::vector2f& getPosition() const;
};

class FreeCamera : public ACamera
{
private:
    bool up, down, left, right, in, out;
    float speed, scale_speed;
    float scale;

public:
    FreeCamera(float x, float y);

    void setScale(float scale);
    float getScale() const;
    void tick();
    void event(const sf::Event &e);
};

class ShipCamera : public ACamera
{
private:
    ShipPtr ship;

public:
    void tick();
    void setShip(ShipPtr ship);
};

#endif /* AST_CAMERA_H */

