#include "car.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace glm;

Car::Car()
    : position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f), speed(0.f),
      wheelsRollAngle(0.f), steeringAngle(0.f), isHeadlightOn(false),
      isBraking(false), isLeftBlinkerActivated(false),
      isRightBlinkerActivated(false), isBlinkerOn(false), blinkerTimer(0.f),
      lastColorMod_(-1.0f, -1.0f, -1.0f) {}

void Car::setColorMod(const glm::vec3 &color) {
  if (glm::all(glm::equal(color, lastColorMod_)))
    return;
  glUniform3f(colorModUniformLocation, color.r, color.g, color.b);
  lastColorMod_ = color;
}

void Car::loadModels() {
  frame_.load("../models/frame.ply");
  wheel_.load("../models/wheel.ply");
  blinker_.load("../models/blinker.ply");
  light_.load("../models/light.ply");
}

void Car::update(float deltaTime) {
  if (isBraking) {
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

  glm::vec3 positionMod =
      glm::rotate(glm::mat4(1.0f), orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::vec4(-speed, 0.f, 0.f, 1.f);
  position += positionMod * deltaTime;

  const float WHEEL_RADIUS = 0.2f;
  wheelsRollAngle +=
      speed / (2.f * glm::pi<float>() * WHEEL_RADIUS) * deltaTime;

  if (wheelsRollAngle > glm::pi<float>())
    wheelsRollAngle -= 2.f * glm::pi<float>();
  else if (wheelsRollAngle < -glm::pi<float>())
    wheelsRollAngle += 2.f * glm::pi<float>();

  if (isRightBlinkerActivated || isLeftBlinkerActivated) {
    const float BLINKER_PERIOD_SEC = 0.5f;
    blinkerTimer += deltaTime;
    if (blinkerTimer > BLINKER_PERIOD_SEC) {
      blinkerTimer = 0.f;
      isBlinkerOn = !isBlinkerOn;
    }
  } else {
    isBlinkerOn = true;
    blinkerTimer = 0.f;
  }
}

void Car::draw(glm::mat4 &projView) {
  glm::mat4 carModel = glm::rotate(glm::translate(glm::mat4(1.0f), position),
                                   orientation.y, {0, 1, 0});
  drawFrame(projView, carModel);
  drawWheels(projView, carModel);
}

void Car::drawFrame(glm::mat4 &projView, glm::mat4 carModel) {
  glm::mat4 frameModel = glm::translate(carModel, {0, 0.25f, 0});
  glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE,
                     glm::value_ptr(projView * frameModel));
  setColorMod({1, 1, 1});
  frame_.draw();
  drawHeadlights(projView, frameModel);
}

void Car::drawWheel(glm::mat4 &projView, glm::mat4 wheelModel,
                    bool isFrontWheel) {
  bool isLeft = wheelModel[3][2] < 0;
  // Rotation pour que la jante soit vers l'extérieur pour les roues de droite.
  if (!isLeft)
    wheelModel = glm::rotate(wheelModel, glm::pi<float>(), {0, 1, 0});

  // Changement de l'axe de rotation (0.10124 unité vers l'intérieur).
  wheelModel = glm::translate(wheelModel, {0, 0, 0.10124f});
  if (isFrontWheel)
    wheelModel = glm::rotate(
        wheelModel, (isLeft ? 1.0f : -1.0f) * glm::radians(steeringAngle),
        {0, 1, 0});
  wheelModel = glm::rotate(
      wheelModel, (isLeft ? 1.0f : -1.0f) * wheelsRollAngle, {0, 0, 1});
  wheelModel = glm::translate(wheelModel, {0, 0, -0.10124f});

  glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE,
                     glm::value_ptr(projView * wheelModel));
  setColorMod({1, 1, 1});
  wheel_.draw();
}

void Car::drawWheels(glm::mat4 &projView, glm::mat4 carModel) {
  const glm::vec3 WHEEL_POSITIONS[] = {
      glm::vec3(-1.29f, 0.245f, -0.57f), glm::vec3(-1.29f, 0.245f, 0.57f),
      glm::vec3(1.4f, 0.245f, -0.57f), glm::vec3(1.4f, 0.245f, 0.57f)};
  for (int i = 0; i < 4; ++i)
    drawWheel(projView, glm::translate(carModel, WHEEL_POSITIONS[i]), i < 2);
}

void Car::drawBlinker(glm::mat4 &projView, glm::mat4 headlightModel,
                      bool isLeftHeadlight) {
  // Positionnement à z=0.06065 du côté extérieur.
  glm::mat4 blinkerModel = glm::translate(
      headlightModel, {0, 0, isLeftHeadlight ? -0.06065f : 0.06065f});
  bool isBlinkerActivated = (isLeftHeadlight && isLeftBlinkerActivated) ||
                            (!isLeftHeadlight && isRightBlinkerActivated);

  glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE,
                     glm::value_ptr(projView * blinkerModel));
  setColorMod(isBlinkerOn && isBlinkerActivated
                  ? glm::vec3(1.0f, 0.7f, 0.3f)
                  : glm::vec3(0.5f, 0.35f, 0.15f));
  blinker_.draw();
}

void Car::drawLight(glm::mat4 &projView, glm::mat4 headlightModel,
                    bool isFrontHeadlight) {
  // Positionnement à z=0.029.
  glm::mat4 lightModel = glm::translate(headlightModel, {0, 0, 0.029f});
  glm::vec3 color =
      isFrontHeadlight
          ? (isHeadlightOn ? glm::vec3(1, 1, 1) : glm::vec3(0.5, 0.5, 0.5))
          : (isBraking ? glm::vec3(1, 0.1, 0.1) : glm::vec3(0.5, 0.1, 0.1));
  glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE,
                     glm::value_ptr(projView * lightModel));
  setColorMod(color);
  light_.draw();
}

void Car::drawHeadlight(glm::mat4 &projView, glm::mat4 headlightModel,
                        bool isFrontHeadlight, bool isLeftHeadlight) {
  // Rotation de 5 degrés pour épouser le châssis à l'avant.
  if (isFrontHeadlight)
    headlightModel = glm::rotate(headlightModel, glm::radians(5.0f), {0, 0, 1});
  drawLight(projView, headlightModel, isFrontHeadlight);
  drawBlinker(projView, headlightModel, isLeftHeadlight);
}

void Car::drawHeadlights(glm::mat4 &projView, glm::mat4 frameModel) {
  const glm::vec3 HEADLIGHT_POSITIONS[] = {
      glm::vec3(-1.9650f, 0.38f, -0.45f), glm::vec3(-1.9650f, 0.38f, 0.45f),
      glm::vec3(2.0019f, 0.38f, -0.45f), glm::vec3(2.0019f, 0.38f, 0.45f)};
  for (int i = 0; i < 4; ++i)
    drawHeadlight(projView, glm::translate(frameModel, HEADLIGHT_POSITIONS[i]),
                  i < 2, i % 2 == 0);
}
