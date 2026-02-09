#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // AJOUTEZ CETTE LIGNE
#include <glm/gtc/type_ptr.hpp>           // AJOUTEZ CETTE LIGNE

#include "model.hpp"

class Car
{   
public:
    Car();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView);

    void setColorMod(const glm::vec3& color);
    
private:
    // TODO: Une division en méthodes a déjà été réalisée pour vous.
    //       Libre à vous de modifier (ou ajouter) les définitions (et surtout
    //       d'ajouter des paramètres d'entrées). Procédez comme bon vous semble.
    //       Il sera important de penser à comment réutiliser le plus possible les matrices.
    //       Utilisez le passage par référence ou par copie pour vous aider.
    
    void drawFrame(glm::mat4& projView, glm::mat4 carModel);

    void drawWheel(glm::mat4& projView, glm::mat4 wheelModel, bool isFrontWheel);
    void drawWheels(glm::mat4& projView, glm::mat4 carModel);

    void drawBlinker(glm::mat4& projView, glm::mat4 headlightModel, bool isLeftHeadlight);
    void drawLight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight);    
    void drawHeadlight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, bool isLeftHeadlight);
    void drawHeadlights(glm::mat4& projView, glm::mat4 frameModel);

    glm::vec3 lastColorMod_;
    
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
    
    GLuint colorModUniformLocation{0};
    GLuint mvpUniformLocation{0};
};


