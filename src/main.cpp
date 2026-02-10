#include "car.hpp"
#include "happly.h"
#include "model.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <inf2705/OpenGLApplication.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace gl;
using namespace glm;

struct Vertex {
  glm::vec2 position;
  glm::vec3 color;
};

struct App : public OpenGLApplication {
  App()
      : nSide_(5), oldNSide_(0), cameraPosition_(0.f, 10.f, 30.f),
        cameraOrientation_(glm::radians(-15.0f), 0.f), currentScene_(0),
        isMouseMotionEnabled_(false), isAutopilotEnabled_(true),
        trackDistance_(0.0f) {
    car_.position = glm::vec3(0.0f, 0.0f, 15.0f);
    car_.orientation.y = glm::radians(180.0f);
  }

  void init() override {
    // Le message expliquant les touches de clavier.
    setKeybindMessage("ESC : quitter l'application."
                      "\n"
                      "T : changer de scène."
                      "\n"
                      "W : déplacer la caméra vers l'avant."
                      "\n"
                      "S : déplacer la caméra vers l'arrière."
                      "\n"
                      "A : déplacer la caméra vers la gauche."
                      "\n"
                      "D : déplacer la caméra vers la droite."
                      "\n"
                      "Q : déplacer la caméra vers le bas."
                      "\n"
                      "E : déplacer la caméra vers le haut."
                      "\n"
                      "Flèches : tourner la caméra."
                      "\n"
                      "Souris : tourner la caméra"
                      "\n"
                      "Espace : activer/désactiver la souris."
                      "\n");

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gris moyen

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    loadShaderPrograms();

    // Partie 1
    initShapeData();

    // Partie 2
    loadModels();

    initStaticMatrices();
  }

  void checkShaderCompilingError(const char *name, GLuint id) {
    GLint success;
    GLchar infoLog[1024];

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, NULL, infoLog);
      glDeleteShader(id);
      std::cout << "Shader \"" << name << "\" compile error: " << infoLog
                << std::endl;
    }
  }

  void checkProgramLinkingError(const char *name, GLuint id) {
    GLint success;
    GLchar infoLog[1024];

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, NULL, infoLog);
      glDeleteProgram(id);
      std::cout << "Program \"" << name << "\" linking error: " << infoLog
                << std::endl;
    }
  }

  void drawFrame() override {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Scene Parameters");
    ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
    ImGui::End();

    switch (currentScene_) {
    case 0:
      sceneShape();
      break;
    case 1:
      sceneModels();
      break;
    }
  }

  void onClose() override {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteProgram(basicSP_);
    glDeleteProgram(transformSP_);
  }

  void onKeyPress(const sf::Event::KeyPressed &key) override {
    using enum sf::Keyboard::Key;
    switch (key.code) {
    case Escape:
      window_.close();
      break;
    case Space:
      isMouseMotionEnabled_ = !isMouseMotionEnabled_;
      if (isMouseMotionEnabled_) {
        window_.setMouseCursorGrabbed(true);
        window_.setMouseCursorVisible(false);
      } else {
        window_.setMouseCursorGrabbed(false);
        window_.setMouseCursorVisible(true);
      }
      break;
    case T:
      currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
      break;
    default:
      break;
    }
  }

  void onResize(const sf::Event::Resized &event) override {}

  void onMouseMove(const sf::Event::MouseMoved &mouseDelta) override {
    if (!isMouseMotionEnabled_)
      return;

    const float MOUSE_SENSITIVITY = 0.1;
    float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
    float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
    cameraOrientation_.x -= cameraMouvementX * deltaTime_;
  }

  void updateCameraInput() {
    if (!window_.hasFocus())
      return;

    if (isMouseMotionEnabled_) {
      sf::Vector2u windowSize = window_.getSize();
      sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
      sf::Mouse::setPosition(windowHalfSize, window_);
    }

    float cameraMouvementX = 0;
    float cameraMouvementY = 0;

    const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
      cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
      cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
      cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
      cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
    cameraOrientation_.x -= cameraMouvementX * deltaTime_;

    // Keyboard input
    glm::vec3 positionOffset = glm::vec3(0.0);
    const float SPEED = 10.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
      positionOffset.z -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
      positionOffset.z += SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
      positionOffset.x -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
      positionOffset.x += SPEED;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
      positionOffset.y -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
      positionOffset.y += SPEED;

    positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y,
                                 glm::vec3(0.0, 1.0, 0.0)) *
                     glm::vec4(positionOffset, 1);
    cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
  }

  void loadModels() {
    car_.loadModels();
    tree_.load("../models/pine.ply");
    streetlight_.load("../models/streetlight.ply");
    grass_.load("../models/grass.ply");
    street_.load("../models/street.ply");
    streetcorner_.load("../models/streetcorner.ply");
  }

  GLuint loadShaderObject(GLenum type, const char *path) {
    std::string shaderSource = readFile(
        "../src/shaders/" + std::filesystem::path(path).filename().string());

    if (shaderSource.empty()) {
      std::cerr << "CRITICAL ERROR: Shader file not found: " << path
                << std::endl;
      return 0;
    }

    const char *shaderSourceCStr = shaderSource.c_str();

    GLuint shaderID = glCreateShader(type);

    glShaderSource(shaderID, 1, &shaderSourceCStr, nullptr);

    glCompileShader(shaderID);

    checkShaderCompilingError(path, shaderID);

    return shaderID;
  }

  void loadShaderPrograms() {
    // Partie 1
    const char *BASIC_VERTEX_SRC_PATH = "./shaders/basic.vs.glsl";
    const char *BASIC_FRAGMENT_SRC_PATH = "./shaders/basic.fs.glsl";

    // Charger et compiler les shaders
    GLuint vertexShader =
        loadShaderObject(GL_VERTEX_SHADER, BASIC_VERTEX_SRC_PATH);
    GLuint fragmentShader =
        loadShaderObject(GL_FRAGMENT_SHADER, BASIC_FRAGMENT_SRC_PATH);

    // Créer le programme de shader
    basicSP_ = glCreateProgram();

    // Attacher les shaders au programme
    glAttachShader(basicSP_, vertexShader);
    glAttachShader(basicSP_, fragmentShader);

    // Lier le programme
    glLinkProgram(basicSP_);

    // Vérifier les erreurs de liaison
    checkProgramLinkingError("basic", basicSP_);

    // Détacher et supprimer les shaders (on n'en a plus besoin)
    glDetachShader(basicSP_, vertexShader);
    glDetachShader(basicSP_, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Partie 2
    const char *TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
    const char *TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

    vertexShader =
        loadShaderObject(GL_VERTEX_SHADER, TRANSFORM_VERTEX_SRC_PATH);
    fragmentShader =
        loadShaderObject(GL_FRAGMENT_SHADER, TRANSFORM_FRAGMENT_SRC_PATH);

    transformSP_ = glCreateProgram();
    glAttachShader(transformSP_, vertexShader);
    glAttachShader(transformSP_, fragmentShader);
    glLinkProgram(transformSP_);
    checkProgramLinkingError("transform", transformSP_);

    glDetachShader(transformSP_, vertexShader);
    glDetachShader(transformSP_, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Récupérer les locations des variables uniform
    mvpUniformLocation_ = glGetUniformLocation(transformSP_, "uMVP");
    colorModUniformLocation_ = glGetUniformLocation(transformSP_, "uColorMod");

    // Pour la voiture
    car_.mvpUniformLocation = mvpUniformLocation_;
    car_.colorModUniformLocation = colorModUniformLocation_;
  }

  void generateNgon() {
    const float RADIUS = 0.7f;
    vertices_[0] = {{0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}; // Centre blanc

    // Générer les points du périmètre avec couleurs
    for (int i = 0; i < nSide_; i++) {
      float angle = 2.0f * glm::pi<float>() * i / nSide_;
      float x = RADIUS * cos(angle);
      float y = RADIUS * sin(angle);

      // Couleur arc-en-ciel basée sur l'angle
      float hue = (float)i / nSide_;
      glm::vec3 color;
      color.r = 0.5f + 0.5f * cos(2.0f * glm::pi<float>() * hue);
      color.g = 0.5f + 0.5f * cos(2.0f * glm::pi<float>() * (hue + 0.333f));
      color.b = 0.5f + 0.5f * cos(2.0f * glm::pi<float>() * (hue + 0.666f));

      vertices_[i + 1] = {{x, y}, color}; // i+1 car vertices_[0] = centre
    }

    // Générer les indices : triangle fan depuis le centre
    for (int i = 0; i < nSide_; i++) {
      elements_[i * 3 + 0] = 0;                    // Centre
      elements_[i * 3 + 1] = i + 1;                // Point actuel
      elements_[i * 3 + 2] = (i + 1) % nSide_ + 1; // Point suivant (wraparound)
    }
  }

  void initShapeData() {
    // Générer les buffers
    glGenBuffers(1, &vbo_);      // Vertex Buffer Object
    glGenBuffers(1, &ebo_);      // Element Buffer Object
    glGenVertexArrays(1, &vao_); // Vertex Array Object

    // Configurer le VAO
    glBindVertexArray(vao_);

    // Remplir le VBO avec les données des sommets
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // Allouer le VBO avec une taille suffisante (GL_DYNAMIC_DRAW car les
    // données changeront)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), nullptr, GL_DYNAMIC_DRAW);

    // Remplir le EBO avec les indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements_), nullptr,
                 GL_DYNAMIC_DRAW);

    // Spécifier le format des données
    // Attribut 0 : position (2 floats, offset 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Attribut 1 : couleur (3 floats, offset après position)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Délier le VAO pour éviter des modifications accidentelles
    glBindVertexArray(0);
  }

  void sceneShape() {
    ImGui::Begin("Scene Parameters");
    ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
    ImGui::End();

    bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
    if (hasNumberOfSidesChanged) {
      oldNSide_ = nSide_;
      generateNgon();
      // generateNgon(vertices_, elements_, nSide_);

      // Mettre à jour le VBO avec les nouvelles données
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * (nSide_ + 1),
                      vertices_);

      // Mettre à jour le EBO avec les nouveaux indices
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * nSide_ * 3,
                      elements_);
    }

    // Utiliser le programme de shader
    glUseProgram(basicSP_);

    // Lier le VAO
    glBindVertexArray(vao_);

    // Dessiner TOUS les triangles du polygone
    glDrawElements(GL_TRIANGLES, nSide_ * 3, GL_UNSIGNED_INT, 0);

    // Délier le VAO
    glBindVertexArray(0);
  }

  void drawModel(const Model &model, const glm::mat4 &projView,
                 const glm::mat4 &modelMatrix) {
    glm::mat4 mvp = projView * modelMatrix;
    glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(mvp));
    model.draw();
  }

  void drawStreetlights(glm::mat4 &projView) {
    for (int i = 0; i < 8; ++i) {
      drawModel(streetlight_, projView, streetlightModelMatrices_[i]);
    }
  }

  void drawTree(glm::mat4 &projView) {
    glDisable(GL_CULL_FACE);
    drawModel(tree_, projView, treeModelMatrice_);
    glEnable(GL_CULL_FACE);
  }

  void drawGround(glm::mat4 &projView) {
    drawModel(grass_, projView, groundModelMatrice_);

    for (int i = 0; i < 4; ++i) {
      drawModel(streetcorner_, projView, streetPatchesModelMatrices_[i]);
    }

    for (int i = 4; i < N_STREET_PATCHES; ++i) {
      drawModel(street_, projView, streetPatchesModelMatrices_[i]);
    }
  }

  glm::mat4 getViewMatrix() {
    glm::mat4 view = glm::mat4(1.0f);
    view =
        glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
    view =
        glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, -cameraPosition_);
    return view;
  }

  glm::mat4 getPerspectiveProjectionMatrix() {
    // getWindowAspect();
    float fov = glm::radians(70.0f);  // Convertir 70° en radians
    float aspect = getWindowAspect(); // Ratio largeur/hauteur
    float nearPlane = 0.1f;
    float farPlane = 300.0f;

    return glm::perspective(fov, aspect, nearPlane, farPlane);

    return glm::mat4(1.0);
  }

  void sceneModels() {
    ImGui::Begin("Scene Parameters");
    ImGui::SliderFloat("Car speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
    ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f,
                       "%.2f°");
    if (ImGui::Button("Reset steering"))
      car_.steeringAngle = 0.f;
    ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
    ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
    ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
    ImGui::Checkbox("Brake", &car_.isBraking);
    ImGui::Checkbox("Auto drive", &isAutopilotEnabled_);
    ImGui::End();

    updateCameraInput();
    car_.update(deltaTime_);

    if (isAutopilotEnabled_)
      updateCarOnTrack(deltaTime_);

    glUseProgram(transformSP_);
    car_.setColorMod(glm::vec3(1.0f));

    glm::mat4 view = getViewMatrix();
    glm::mat4 projection = getPerspectiveProjectionMatrix();
    glm::mat4 projView = projection * view;

    drawGround(projView);
    drawTree(projView);
    drawStreetlights(projView);
    car_.draw(projView);
  }

  void initStaticMatrices() {
    const float ROAD_HALF_LENGTH = 15.0f;
    const float ROAD_WIDTH = 5.0f;
    const float SEGMENT_LENGTH = 30.0f / 7.0f;

    groundModelMatrice_ =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
    groundModelMatrice_ =
        glm::scale(groundModelMatrice_, glm::vec3(50.0f, 1.0f, 50.0f));

    treeModelMatrice_ =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    treeModelMatrice_ =
        glm::scale(treeModelMatrice_, glm::vec3(15.0f, 15.0f, 15.0f));

    const float lightOffset = ROAD_HALF_LENGTH - (ROAD_WIDTH * 0.5f) - 0.25f;
    const glm::vec3 positions[] = {
        {-7.5f, -0.15f, -lightOffset}, {7.5f, -0.15f, -lightOffset},
        {-7.5f, -0.15f, lightOffset},  {7.5f, -0.15f, lightOffset},
        {-lightOffset, -0.15f, -7.5f}, {-lightOffset, -0.15f, 7.5f},
        {lightOffset, -0.15f, -7.5f},  {lightOffset, -0.15f, 7.5f}};

    for (int i = 0; i < N_STREETLIGHTS; ++i) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), positions[i]);
      glm::vec3 toCenter = glm::normalize(glm::vec3(0.0f) - positions[i]);
      float rotation =
          std::atan2(-toCenter.x, -toCenter.z) + glm::radians(90.0f);
      streetlightModelMatrices_[i] =
          glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    int patchIndex = 0;
    const glm::vec3 corners[] = {{-ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH},
                                 {ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH},
                                 {-ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH},
                                 {ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH}};

    for (const auto &pos : corners) {
      glm::mat4 cornerModel = glm::translate(glm::mat4(1.0f), pos);
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(cornerModel, glm::vec3(ROAD_WIDTH, 1.0f, ROAD_WIDTH));
    }

    for (int i = 0; i < 7; ++i) {
      float x =
          -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

      glm::mat4 top = glm::translate(glm::mat4(1.0f),
                                     glm::vec3(x, 0.0f, -ROAD_HALF_LENGTH));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(top, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

      glm::mat4 bottom =
          glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, ROAD_HALF_LENGTH));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(bottom, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
    }

    for (int i = 0; i < 7; ++i) {
      float z =
          -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

      glm::mat4 left = glm::translate(glm::mat4(1.0f),
                                      glm::vec3(-ROAD_HALF_LENGTH, 0.0f, z));
      left =
          glm::rotate(left, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(left, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

      glm::mat4 right =
          glm::translate(glm::mat4(1.0f), glm::vec3(ROAD_HALF_LENGTH, 0.0f, z));
      right =
          glm::rotate(right, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(right, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
    }
  }

  void updateCarOnTrack(float deltaTime) {
    const float ROAD_HALF_LENGTH = 15.0f;
    const float SEGMENT_LENGTH = ROAD_HALF_LENGTH * 2.0f;
    const float PERIMETER = 4.0f * SEGMENT_LENGTH;

    trackDistance_ += car_.speed * deltaTime;
    trackDistance_ = std::fmod(trackDistance_, PERIMETER);
    if (trackDistance_ < 0.0f)
      trackDistance_ += PERIMETER;

    float d = trackDistance_;

    if (d < SEGMENT_LENGTH) {
      car_.position = glm::vec3(-ROAD_HALF_LENGTH + d, 0.0f, ROAD_HALF_LENGTH);
      car_.orientation.y = glm::radians(180.0f);
    } else if (d < 2.0f * SEGMENT_LENGTH) {
      d -= SEGMENT_LENGTH;
      car_.position = glm::vec3(ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH - d);
      car_.orientation.y = glm::radians(90.0f);
    } else if (d < 3.0f * SEGMENT_LENGTH) {
      d -= 2.0f * SEGMENT_LENGTH;
      car_.position = glm::vec3(ROAD_HALF_LENGTH - d, 0.0f, -ROAD_HALF_LENGTH);
      car_.orientation.y = 0.0f;
    } else {
      d -= 3.0f * SEGMENT_LENGTH;
      car_.position = glm::vec3(-ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH + d);
      car_.orientation.y = glm::radians(-90.0f);
    }
  }

private:
  // Shaders
  GLuint basicSP_;
  GLuint transformSP_;
  GLint colorModUniformLocation_;
  GLint mvpUniformLocation_;

  // Partie 1
  GLuint vbo_, ebo_, vao_;

  static constexpr unsigned int MIN_N_SIDES = 5;
  static constexpr unsigned int MAX_N_SIDES = 12;

  Vertex vertices_[MAX_N_SIDES + 1];
  GLuint elements_[(MAX_N_SIDES) * 3];

  int nSide_, oldNSide_;

  // Partie 2
  Model tree_;
  Model streetlight_;
  Model grass_;
  Model street_;
  Model streetcorner_;

  Car car_;

  glm::vec3 cameraPosition_;
  glm::vec2 cameraOrientation_;

  // Matrices statiques
  static constexpr unsigned int N_STREETLIGHTS = 2 * 4;
  static constexpr unsigned int N_STREET_PATCHES = 7 * 4 + 4;
  glm::mat4 treeModelMatrice_;
  glm::mat4 groundModelMatrice_;
  glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
  glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];

  // Imgui var
  const char *const SCENE_NAMES[2] = {
      "Introduction",
      "3D Model & transformation",
  };
  const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
  int currentScene_;

  bool isMouseMotionEnabled_;
  bool isAutopilotEnabled_;
  float trackDistance_;
};

int main(int argc, char *argv[]) {
  WindowSettings settings = {};
  settings.fps = 60;
  settings.context.depthBits = 24;
  settings.context.stencilBits = 8;
  settings.context.antiAliasingLevel = 4;
  settings.context.majorVersion = 3;
  settings.context.minorVersion = 3;
  settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

  App app;
  app.run(argc, argv, "Tp1", settings);
}
