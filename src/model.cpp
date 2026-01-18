#include "model.hpp"

#include "happly.h"

using namespace gl;

// TODO: Il est fortement recommandé de définir quelques structs
//       pour représenter les attributs.
//       Faire de même pour représenter une vertex, qui est constitué d'attributs.
//       Cela facilitera l'utilisation et rendra votre code plus clair.
//       Un format entrelacé est recommandé (ordonné par vertex au lieu par attribut).
// struct ... { ... };

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
    
    // TODO: Rassemblez les indices dans un seul tableau contigu.
    
    // TODO: Allocation des ressources sur la carte graphique et envoyer les
    //       données traitées dans le vbo et ebo sur la carte graphique.
    
    // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
    //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
    //       du contexte pour empêcher des modifications sur celui-ci.
    
    // TODO: Initialisez count_, qui correspond au nombre d'indices à dessiner.
}

Model::~Model()
{
    // TODO: Libérez les ressources allouées.
}

void Model::draw()
{
    // TODO: Dessin du modèle.
}

