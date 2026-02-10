#include "car.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>


using namespace gl;
using namespace glm;

    
Car::Car()
: position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f), speed(0.f)
, wheelsRollAngle(0.f), steeringAngle(0.f)
, isHeadlightOn(false), isBraking(false)
, isLeftBlinkerActivated(false), isRightBlinkerActivated(false)
, isBlinkerOn(false), blinkerTimer(0.f)
 , lastColorMod_(-1.0f, -1.0f, -1.0f)
{}

void Car::setColorMod(const glm::vec3& color)
{
    if (glm::all(glm::equal(color, lastColorMod_)))
        return;

    glUniform3f(colorModUniformLocation, color.r, color.g, color.b);
    lastColorMod_ = color;
}

void Car::loadModels()
{
    frame_.load("../models/frame.ply");
    wheel_.load("../models/wheel.ply");
    blinker_.load("../models/blinker.ply");
    light_.load("../models/light.ply");
}

void Car::update(float deltaTime)
{
    if (isBraking)
    {
        const float LOW_SPEED_THRESHOLD = 0.1f;
        const float BRAKE_APPLIED_SPEED_THRESHOLD = 0.01f;
        const float BRAKING_FORCE = 4.f;
    
        if (fabs(speed) < LOW_SPEED_THRESHOLD)
            speed = 0.f;
            
        if (speed > BRAKE_APPLIED_SPEED_THRESHOLD)
            speed -= BRAKING_FORCE * deltaTime;
        else if (speed < -BRAKE_APPLIED_SPEED_THRESHOLD)
            speed += BRAKING_FORCE * deltaTime;
    }
    
    const float WHEELBASE = 2.7f;
    float angularSpeed = speed * sin(-glm::radians(steeringAngle)) / WHEELBASE;
    orientation.y += angularSpeed * deltaTime;
    
    glm::vec3 positionMod = glm::rotate(glm::mat4(1.0f), orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(-speed, 0.f, 0.f, 1.f);
    position += positionMod * deltaTime;
    
    const float WHEEL_RADIUS = 0.2f;
    const float PI = glm::pi<float>();
    wheelsRollAngle += speed / (2.f * PI * WHEEL_RADIUS) * deltaTime;
    
    if (wheelsRollAngle > PI)
        wheelsRollAngle -= 2.f * PI;
    else if (wheelsRollAngle < -PI)
        wheelsRollAngle += 2.f * PI;
        
    if (isRightBlinkerActivated || isLeftBlinkerActivated)
    {
        const float BLINKER_PERIOD_SEC = 0.5f;
        blinkerTimer += deltaTime;
        if (blinkerTimer > BLINKER_PERIOD_SEC)
        {
            blinkerTimer = 0.f;
            isBlinkerOn = !isBlinkerOn;
        }
    }
    else
    {
        isBlinkerOn = true;
        blinkerTimer = 0.f;
    }  
}

void Car::draw(glm::mat4& projView)
{
    glm::mat4 carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position);
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));

    drawFrame(projView, carModel);
    drawWheels(projView, carModel);
}
    
void Car::drawFrame(glm::mat4& projView, glm::mat4 carModel)
{

    glm::mat4 frameModel = glm::translate(carModel, glm::vec3(0.0f, 0.25f, 0.0f));
    
    glm::mat4 mvp = projView * frameModel;
    
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    
    setColorMod(glm::vec3(1.0f));
    
    frame_.draw();
    
    drawHeadlights(projView, frameModel);
}

void Car::drawWheel(glm::mat4& projView, glm::mat4 wheelModel, bool isFrontWheel)
{
    const float WHEEL_ORIGIN_OFFSET = 0.10124f;
    wheelModel = glm::translate(wheelModel, glm::vec3(0.0f, 0.0f, WHEEL_ORIGIN_OFFSET));
  
    if (isFrontWheel)
    {
        wheelModel = glm::rotate(wheelModel, glm::radians(steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    wheelModel = glm::rotate(wheelModel, wheelsRollAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    wheelModel = glm::translate(wheelModel, glm::vec3(0.0f, 0.0f, -WHEEL_ORIGIN_OFFSET));
  
    glm::mat4 mvp = projView * wheelModel;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    setColorMod(glm::vec3(1.0f));
    
    wheel_.draw();
}

void Car::drawWheels(glm::mat4& projView, glm::mat4 carModel)
{
    const glm::vec3 WHEEL_POSITIONS[] =
    {
        glm::vec3(-1.29f, 0.245f, -0.57f),  // Avant gauche
        glm::vec3(-1.29f, 0.245f,  0.57f),  // Avant droite
        glm::vec3( 1.4f , 0.245f, -0.57f),  // Arrière gauche
        glm::vec3( 1.4f , 0.245f,  0.57f)   // Arrière droite
    };
    
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 wheelModel = glm::translate(carModel, WHEEL_POSITIONS[i]);
        bool isFrontWheel = (i < 2);  // Les 2 premières sont les roues avant
        drawWheel(projView, wheelModel, isFrontWheel);
    }
}

void Car::drawBlinker(glm::mat4& projView, glm::mat4 headlightModel, bool isLeftHeadlight)
{
    // Positionner le clignotant
    glm::mat4 blinkerModel = glm::translate(headlightModel, glm::vec3(0.0f, 0.0f, -0.06065f));
    
    bool isBlinkerActivated = (isLeftHeadlight  && isLeftBlinkerActivated) ||
                              (!isLeftHeadlight && isRightBlinkerActivated);

    const glm::vec3 ON_COLOR (1.0f, 0.7f , 0.3f );
    const glm::vec3 OFF_COLOR(0.5f, 0.35f, 0.15f);
    
    glm::vec3 color = (isBlinkerOn && isBlinkerActivated) ? ON_COLOR : OFF_COLOR;
    
    glm::mat4 mvp = projView * blinkerModel;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    setColorMod(color);
    
    blinker_.draw();
}

void Car::drawLight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight)
{
    glm::mat4 lightModel = glm::translate(headlightModel, glm::vec3(0.0f, 0.0f, 0.029f));
    const glm::vec3 FRONT_ON_COLOR (1.0f, 1.0f, 1.0f);
    const glm::vec3 FRONT_OFF_COLOR(0.5f, 0.5f, 0.5f);
    const glm::vec3 REAR_ON_COLOR  (1.0f, 0.1f, 0.1f);
    const glm::vec3 REAR_OFF_COLOR (0.5f, 0.1f, 0.1f);
    
    glm::vec3 color;
    if (isFrontHeadlight)
    {
        color = isHeadlightOn ? FRONT_ON_COLOR : FRONT_OFF_COLOR;
    }
    else
    {
        color = isBraking ? REAR_ON_COLOR : REAR_OFF_COLOR;
    }
    
    glm::mat4 mvp = projView * lightModel;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    setColorMod(color);
    
    light_.draw();
}

void Car::drawHeadlight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, bool isLeftHeadlight)
{
    // Rotation de 5 degrés pour les phares avant
    if (isFrontHeadlight)
    {
        headlightModel = glm::rotate(headlightModel, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    drawLight(projView, headlightModel, isFrontHeadlight);
    drawBlinker(projView, headlightModel, isLeftHeadlight);
}

void Car::drawHeadlights(glm::mat4& projView, glm::mat4 frameModel)
{
    const glm::vec3 HEADLIGHT_POSITIONS[] =
    {
        glm::vec3(-1.9650f, 0.38f, -0.45f),  // Avant gauche
        glm::vec3(-1.9650f, 0.38f,  0.45f),  // Avant droite
        glm::vec3( 2.0019f, 0.38f, -0.45f),  // Arrière gauche
        glm::vec3( 2.0019f, 0.38f,  0.45f)   // Arrière droite
    };
    
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 headlightModel = glm::translate(frameModel, HEADLIGHT_POSITIONS[i]);
        bool isFrontHeadlight = (i < 2);      // Les 2 premiers sont à l'avant
        bool isLeftHeadlight = (i % 2 == 0);  // Indices pairs = gauche
        
        drawHeadlight(projView, headlightModel, isFrontHeadlight, isLeftHeadlight);
    }
}

