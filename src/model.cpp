#include "model.hpp"
#include "happly.h"
#include <glm/vec3.hpp>

using namespace gl;

// TODO: Il est fortement recommandé de définir quelques structs
//       pour représenter les attributs.
//       Faire de même pour représenter une vertex, qui est constitué d'attributs.
//       Cela facilitera l'utilisation et rendra votre code plus clair.
//       Un format entrelacé est recommandé (ordonné par vertex au lieu par attribut).
// struct ... { ... };
struct Vertex3D {
    glm::vec3 position;  // Position 3D (x, y, z)
    glm::vec3 color;     // Couleur RGB (normalisée 0-1)
};

void Model::load(const char* path)
{
    // Chargement des données du fichier .ply.
    // Ne modifier pas cette partie.
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");
    
    std::vector<unsigned char> colorRed   = vertex.getProperty<unsigned char>("red");
    std::vector<unsigned char> colorGreen = vertex.getProperty<unsigned char>("green");
    std::vector<unsigned char> colorBlue  = vertex.getProperty<unsigned char>("blue");

    // Tableau de faces, une face est un tableau d'indices.
    // Les faces sont toutes des triangles dans nos modèles (donc 3 indices par face).
    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();
    
    // TODO: Rassemblez les propriétés du fichier .ply pour correspondre au
    //       format de donnée souhaité (celui que vous avez défini dans la struct).

    size_t numVertices = positionX.size();
    std::vector<Vertex3D> vertices(numVertices);

    for (size_t i = 0; i < numVertices; ++i)
    {
        vertices[i].position = glm::vec3(positionX[i], positionY[i], positionZ[i]);
        // Normaliser les couleurs (0-255 -> 0.0-1.0)
        vertices[i].color = glm::vec3(
            colorRed[i] / 255.0f,
            colorGreen[i] / 255.0f,
            colorBlue[i] / 255.0f
        );
    }
    
    // TODO: Rassemblez les indices dans un seul tableau contigu.

    std::vector<GLuint> indices;
    for (const auto& face : facesIndices)
    {
        for (unsigned int index : face)
        {
            indices.push_back(index);
        }
    }
    // Sauvegarder le nombre d'indices à dessiner
    count_ = static_cast<GLsizei>(indices.size());
    
    // TODO: Allocation des ressources sur la carte graphique et envoyer les
    //       données traitées dans le vbo et ebo sur la carte graphique.

    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glGenVertexArrays(1, &vao_);

    // Configurer le VAO
    glBindVertexArray(vao_);

    // Remplir le VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), vertices.data(), GL_STATIC_DRAW);

    // Remplir le EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Spécifier le format des données
    // Attribut 0 : position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, position));
    glEnableVertexAttribArray(0);

    // Attribut 1 : couleur (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));
    glEnableVertexAttribArray(1);

    // Délier le VAO
    glBindVertexArray(0);
    
    // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
    //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
    //       du contexte pour empêcher des modifications sur celui-ci.
    
    // TODO: Initialisez count_, qui correspond au nombre d'indices à dessiner.
}

Model::~Model()
{
    // TODO: Libérez les ressources allouées.
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
}

void Model::draw() const
{
    // TODO: Dessin du modèle.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


