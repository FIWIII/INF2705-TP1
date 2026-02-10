#include "model.hpp"

#include "happly.h"
#include <vector>

#include <glm/glm.hpp>

using namespace gl;

struct Vertex3D {
  glm::vec3 position;
  glm::vec3 color;
};

void Model::load(const char *path) {
  // Chargement des données du fichier .ply.
  // Ne modifiez pas cette partie.
  happly::PLYData plyIn(path);

  happly::Element &vertex = plyIn.getElement("vertex");
  std::vector<float> positionX = vertex.getProperty<float>("x");
  std::vector<float> positionY = vertex.getProperty<float>("y");
  std::vector<float> positionZ = vertex.getProperty<float>("z");

  std::vector<unsigned char> colorRed =
      vertex.getProperty<unsigned char>("red");
  std::vector<unsigned char> colorGreen =
      vertex.getProperty<unsigned char>("green");
  std::vector<unsigned char> colorBlue =
      vertex.getProperty<unsigned char>("blue");

  // Tableau de faces, une face est un tableau d'indices.
  // Les faces sont toutes des triangles dans nos modèles (3 indices par face).
  std::vector<std::vector<unsigned int>> facesIndices =
      plyIn.getFaceIndices<unsigned int>();

  // Rassembler les propriétés
  std::vector<Vertex3D> vertices(positionX.size());
  for (size_t i = 0; i < positionX.size(); ++i) {
    vertices[i] = {
        {positionX[i], positionY[i], positionZ[i]},
        {colorRed[i] / 255.0f, colorGreen[i] / 255.0f, colorBlue[i] / 255.0f}};
  }

  // Rassembler les indices pour le EBO.
  std::vector<GLuint> indices;
  for (const auto &face : facesIndices) {
    for (unsigned int idx : face) {
      indices.push_back(idx);
    }
  }

  // Initialisation du nombre d'indices à dessiner.
  count_ = static_cast<GLsizei>(indices.size());

  // Allocation des ressources sur la carte graphique (VBO, EBO, VAO).
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
  glGenVertexArrays(1, &vao_);

  // Liaison du VAO
  glBindVertexArray(vao_);

  // Remplissage du VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D),
               vertices.data(), GL_STATIC_DRAW);

  // Remplissage du EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)sizeof(glm::vec3));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Model::~Model() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteBuffers(1, &ebo_);
}

void Model::draw() const {
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
