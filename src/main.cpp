#include "car.hpp"
#include "model.hpp"
#include <cmath>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <inf2705/OpenGLApplication.hpp>
#include <iostream>
#include <string>

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// Struct représentant un sommet avec position 2D et couleur RGB.
struct Vertex {
  glm::vec2 position; // Position 2D (x, y)
  glm::vec3 color;    // Couleur (r, g, b)
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
    setKeybindMessage("ESC : quitter l'application.\n"
                      "T : changer de scène.\n"
                      "W : déplacer la caméra vers l'avant.\n"
                      "S : déplacer la caméra vers l'arrière.\n"
                      "A : déplacer la caméra vers la gauche.\n"
                      "D : déplacer la caméra vers la droite.\n"
                      "Q : déplacer la caméra vers le bas.\n"
                      "E : déplacer la caméra vers le haut.\n"
                      "Flèches : tourner la caméra.\n"
                      "Souris : tourner la caméra\n"
                      "Espace : activer/désactiver la souris.\n");

    // Config de base.
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

  // Appelée à chaque trame. Le buffer swap est fait juste après.
  void drawFrame() override {
    // Nettoyage de la surface de dessin.
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

  // Appelée lorsque la fenêtre se ferme.
  void onClose() override {
    // Libère les ressources allouées
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteProgram(basicSP_);
    glDeleteProgram(transformSP_);
  }

  // Appelée lors d'une touche de clavier.
  void onKeyPress(const sf::Event::KeyPressed &key) override {
    using enum sf::Keyboard::Key;
    switch (key.code) {
    case Escape:
      window_.close();
      break;
    case Space:
      isMouseMotionEnabled_ = !isMouseMotionEnabled_;
      window_.setMouseCursorGrabbed(isMouseMotionEnabled_);
      window_.setMouseCursorVisible(!isMouseMotionEnabled_);
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
    cameraOrientation_.y -=
        mouseDelta.position.x * MOUSE_SENSITIVITY * deltaTime_;
    cameraOrientation_.x -=
        mouseDelta.position.y * MOUSE_SENSITIVITY * deltaTime_;
  }

  void updateCameraInput() {
    if (!window_.hasFocus())
      return;
    if (isMouseMotionEnabled_) {
      sf::Vector2u size = window_.getSize();
      sf::Mouse::setPosition(sf::Vector2i(size.x / 2, size.y / 2), window_);
    }

    const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
      cameraOrientation_.x -= KEYBOARD_MOUSE_SENSITIVITY * deltaTime_;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
      cameraOrientation_.x += KEYBOARD_MOUSE_SENSITIVITY * deltaTime_;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
      cameraOrientation_.y -= KEYBOARD_MOUSE_SENSITIVITY * deltaTime_;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
      cameraOrientation_.y += KEYBOARD_MOUSE_SENSITIVITY * deltaTime_;

    glm::vec3 positionOffset(0.0f);
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

    cameraPosition_ += glm::vec3(glm::rotate(glm::mat4(1.0f),
                                             cameraOrientation_.y, {0, 1, 0}) *
                                 glm::vec4(positionOffset, 1)) *
                       deltaTime_;
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
    std::string src = readFile("../src/shaders/" +
                               std::filesystem::path(path).filename().string());
    if (src.empty())
      return 0;
    const char *ptr = src.c_str();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &ptr, nullptr);
    glCompileShader(id);
    checkShaderCompilingError(path, id);
    return id;
  }

  // Création d'un programme de shader (Vertex + Fragment).
  GLuint createProgram(const char *vsPath, const char *fsPath,
                       const char *name) {
    // 1. Charge et compile les vertex et fragment shaders.
    GLuint vs = loadShaderObject(GL_VERTEX_SHADER, vsPath);
    GLuint fs = loadShaderObject(GL_FRAGMENT_SHADER, fsPath);

    // 2. Crée le programme OpenGL.
    GLuint p = glCreateProgram();

    // 3. Attache les shaders au programme.
    glAttachShader(p, vs);
    glAttachShader(p, fs);

    // 4. Lie (link) le programme pour créer l'exécutable GPU.
    glLinkProgram(p);

    // 5. Vérifie s'il y a eu des erreurs lors de la liaison.
    checkProgramLinkingError(name, p);

    // 6. Détache et supprime les shaders objects (plus besoin après le link).
    glDetachShader(p, vs);
    glDetachShader(p, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return p;
  }

  void loadShaderPrograms() {
    // Partie 1
    basicSP_ = createProgram("basic.vs.glsl", "basic.fs.glsl", "basic");

    // Partie 2
    transformSP_ =
        createProgram("transform.vs.glsl", "transform.fs.glsl", "transform");

    // Récupération des emplacements des variables uniformes.
    mvpUniformLocation_ = glGetUniformLocation(transformSP_, "uMVP");
    colorModUniformLocation_ = glGetUniformLocation(transformSP_, "uColorMod");

    // Transmission des locations à l'objet Car pour son rendu
    car_.mvpUniformLocation = mvpUniformLocation_;
    car_.colorModUniformLocation = colorModUniformLocation_;
  }

  // Génération d'un polygone régulier avec une triangulation en éventail.
  void generateNgon() {
    const float RADIUS = 0.7f;
    for (int i = 0; i < nSide_; i++) {
      float a = 2.0f * glm::pi<float>() * i / nSide_, h = (float)i / nSide_;
      vertices_[i] = {
          {RADIUS * cos(a), RADIUS * sin(a)},
          {0.5f + 0.5f * cos(2.f * glm::pi<float>() * h),
           0.5f + 0.5f * cos(2.f * glm::pi<float>() * (h + 0.333f)),
           0.5f + 0.5f * cos(2.f * glm::pi<float>() * (h + 0.666f))}};
    }
    for (int i = 0; i < nSide_ - 2; i++) {
      elements_[i * 3 + 0] = 0;
      elements_[i * 3 + 1] = i + 1;
      elements_[i * 3 + 2] = i + 2;
    }
  }

  void initShapeData() {
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements_), nullptr,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
  }

  void sceneShape() {
    ImGui::Begin("Scene Parameters");
    ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
    ImGui::End();
    if (nSide_ != oldNSide_) {
      oldNSide_ = nSide_;
      generateNgon();
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * nSide_, vertices_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                      sizeof(GLuint) * (nSide_ - 2) * 3, elements_);
    }
    glUseProgram(basicSP_);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, (nSide_ - 2) * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  // Rendu d'un modèle PLY avec modulation de couleur et transformation MVP.
  void drawModel(const Model &m, const glm::mat4 &pv, const glm::mat4 &mm) {
    glm::mat4 mvp = pv * mm;
    glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(mvp));
    m.draw();
  }

  void drawStreetlights(glm::mat4 &pv) {
    for (int i = 0; i < N_STREETLIGHTS; ++i)
      drawModel(streetlight_, pv, streetlightModelMatrices_[i]);
  }

  void drawTree(glm::mat4 &pv) {
    // L'arbre nécessite de désactiver le face culling pour voir l'intérieur des
    // branches
    glDisable(GL_CULL_FACE);
    drawModel(tree_, pv, treeModelMatrice_);
    glEnable(GL_CULL_FACE);
  }

  void drawGround(glm::mat4 &pv) {
    drawModel(grass_, pv, groundModelMatrice_);
    for (int i = 0; i < 4; ++i)
      drawModel(streetcorner_, pv, streetPatchesModelMatrices_[i]);
    for (int i = 4; i < N_STREET_PATCHES; ++i)
      drawModel(street_, pv, streetPatchesModelMatrices_[i]);
  }

  glm::mat4 getViewMatrix() {
    glm::mat4 v = glm::mat4(1.0f);
    v = glm::rotate(v, -cameraOrientation_.x, {1, 0, 0});
    v = glm::rotate(v, -cameraOrientation_.y, {0, 1, 0});
    return glm::translate(v, -cameraPosition_);
  }

  glm::mat4 getPerspectiveProjectionMatrix() {
    // FOV de 70 degrés, near à 0.1 et far à 300.
    return glm::perspective(glm::radians(70.0f), getWindowAspect(), 0.1f,
                            300.0f);
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

    // Calcul de la matrice de Projection-Vue (PV)
    glm::mat4 pv = getPerspectiveProjectionMatrix() * getViewMatrix();

    // Rendu des différents composants de la scène
    drawGround(pv);
    drawTree(pv);
    drawStreetlights(pv);

    // Rendu de l'automobile
    car_.draw(pv);
  }

  // Calcul unique des transformations pour les objets de décor immobiles.
  void initStaticMatrices() {
    const float RL = 15.0f, RW = 5.0f, SL = 30.0f / 7.0f;
    groundModelMatrice_ =
        glm::scale(glm::translate(glm::mat4(1.0f), {0, -0.1f, 0}), {50, 1, 50});
    treeModelMatrice_ = glm::scale(glm::mat4(1.0f), {15, 15, 15});
    const float lo = RL - RW * 0.5f - 0.5f;
    glm::vec3 lp[] = {{-7.5f, -0.15f, -lo}, {7.5f, -0.15f, -lo},
                      {-7.5f, -0.15f, lo},  {7.5f, -0.15f, lo},
                      {-lo, -0.15f, -7.5f}, {-lo, -0.15f, 7.5f},
                      {lo, -0.15f, -7.5f},  {lo, -0.15f, 7.5f}};
    for (int i = 0; i < 8; ++i)
      streetlightModelMatrices_[i] = glm::rotate(
          glm::translate(glm::mat4(1.0f), lp[i]),
          std::atan2(lp[i].x, lp[i].z) + glm::radians(90.0f), {0, 1, 0});
    int ix = 0;
    glm::vec3 c[] = {{-RL, 0, -RL}, {RL, 0, -RL}, {-RL, 0, RL}, {RL, 0, RL}};
    for (auto &p : c)
      streetPatchesModelMatrices_[ix++] =
          glm::scale(glm::translate(glm::mat4(1.0f), p), {RW, 1, RW});
    for (int i = 0; i < 7; ++i) {
      float x = -RL + SL * 0.5f + i * SL, z = -RL + SL * 0.5f + i * SL;
      streetPatchesModelMatrices_[ix++] =
          glm::scale(glm::translate(glm::mat4(1.0f), {x, 0, -RL}), {SL, 1, RW});
      streetPatchesModelMatrices_[ix++] =
          glm::scale(glm::translate(glm::mat4(1.0f), {x, 0, RL}), {SL, 1, RW});
      streetPatchesModelMatrices_[ix++] =
          glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), {-RL, 0, z}),
                                 glm::radians(90.0f), {0, 1, 0}),
                     {SL, 1, RW});
      streetPatchesModelMatrices_[ix++] =
          glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), {RL, 0, z}),
                                 glm::radians(90.0f), {0, 1, 0}),
                     {SL, 1, RW});
    }
  }

  // Calcul du pilotage automatique pour suivre le circuit rectangulaire.
  void updateCarOnTrack(float dt) {
    const float RL = 15.0f, SL = RL * 2.0f;
    trackDistance_ = std::fmod(trackDistance_ + car_.speed * dt, 4.0f * SL);
    if (trackDistance_ < 0.0f)
      trackDistance_ += 4.0f * SL;
    float d = trackDistance_;
    if (d < SL) {
      car_.position = {-RL + d, 0, RL};
      car_.orientation.y = glm::radians(180.0f);
    } else if (d < 2 * SL) {
      d -= SL;
      car_.position = {RL, 0, RL - d};
      car_.orientation.y = glm::radians(90.0f);
    } else if (d < 3 * SL) {
      d -= 2 * SL;
      car_.position = {RL - d, 0, -RL};
      car_.orientation.y = 0;
    } else {
      d -= 3 * SL;
      car_.position = {-RL, 0, -RL + d};
      car_.orientation.y = glm::radians(-90.0f);
    }
  }

private:
  GLuint basicSP_, transformSP_;
  GLint colorModUniformLocation_, mvpUniformLocation_;
  GLuint vbo_, ebo_, vao_;
  static constexpr unsigned int MIN_N_SIDES = 5, MAX_N_SIDES = 12;
  Vertex vertices_[MAX_N_SIDES + 1];
  GLuint elements_[MAX_N_SIDES * 3];
  int nSide_, oldNSide_;
  Model tree_, streetlight_, grass_, street_, streetcorner_;
  Car car_;
  glm::vec3 cameraPosition_;
  glm::vec2 cameraOrientation_;
  static constexpr unsigned int N_STREETLIGHTS = 8, N_STREET_PATCHES = 32;
  glm::mat4 treeModelMatrice_, groundModelMatrice_,
      streetlightModelMatrices_[N_STREETLIGHTS],
      streetPatchesModelMatrices_[N_STREET_PATCHES];
  const char *const SCENE_NAMES[2] = {"Introduction",
                                      "3D Model & transformation"};
  const int N_SCENE_NAMES = 2;
  int currentScene_;
  bool isMouseMotionEnabled_, isAutopilotEnabled_;
  float trackDistance_;
};

int main(int argc, char *argv[]) {
  WindowSettings s = {};
  s.fps = 60;
  s.context.depthBits = 24;
  s.context.stencilBits = 8;
  s.context.antiAliasingLevel = 4;
  s.context.majorVersion = 3;
  s.context.minorVersion = 3;
  s.context.attributeFlags = sf::ContextSettings::Attribute::Core;
  App().run(argc, argv, "Tp1", s);
}