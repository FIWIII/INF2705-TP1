#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Car {
public:
  Car();

  void loadModels();

  void update(float deltaTime);

  void draw(glm::mat4 &projView);

  void setColorMod(const glm::vec3 &color);

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

  gl::GLint colorModUniformLocation;
  gl::GLint mvpUniformLocation;

private:
  void drawFrame(glm::mat4 &projView, glm::mat4 carModel);

  void drawWheel(glm::mat4 &projView, glm::mat4 wheelModel, bool isFrontWheel);
  void drawWheels(glm::mat4 &projView, glm::mat4 carModel);

  void drawBlinker(glm::mat4 &projView, glm::mat4 headlightModel,
                   bool isLeftHeadlight);
  void drawLight(glm::mat4 &projView, glm::mat4 headlightModel,
                 bool isFrontHeadlight);
  void drawHeadlight(glm::mat4 &projView, glm::mat4 headlightModel,
                     bool isFrontHeadlight, bool isLeftHeadlight);
  void drawHeadlights(glm::mat4 &projView, glm::mat4 frameModel);

private:
  Model frame_;
  Model wheel_;
  Model blinker_;
  Model light_;

  glm::vec3 lastColorMod_;
};
