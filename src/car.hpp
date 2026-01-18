#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Car
{   
public:
    Car();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView);
    
private:
    // TODO: Une division en méthodes a déjà été réalisée pour vous.
    //       Libre à vous de modifier (ou ajouter) les définitions (et surtout
    //       d'ajouter des paramètres d'entrées). Procédez comme bon vous semble.
    //       Il sera important de penser à comment réutiliser le plus possible les matrices.
    //       Utilisez le passage par référence ou par copie pour vous aider.
    
    void drawFrame();
    
    void drawWheel();
    void drawWheels();
    
    void drawBlinker();
    void drawLight();    
    void drawHeadlight();
    void drawHeadlights();
    
private:    
    Model frame_;
    Model wheel_;
    Model blinker_;
    Model light_;
    
public:
    glm::vec3 position;
    glm::vec2 orientation;    
    
    float speed;
    float wheelsRollAngle;
    float steeringAngle;
    bool isHeadlightOn;
    bool isBraking;
    bool isLeftBlinkerActivated;
    bool isRightBlinkerActivated;
    
    bool isBlinkerOn;
    float blinkerTimer;
    
    GLuint colorModUniformLocation;
    GLuint mvpUniformLocation;
};


