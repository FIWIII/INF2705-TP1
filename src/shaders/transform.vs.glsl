#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.
//       Les vertices sont transformées à l'aide d'une matrice mvp
//       pour les placer à l'écran.

// Entrées : position 3D et couleur
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

// Sortie vers le fragment shader
out vec3 vColor;

// Variable uniform pour la matrice MVP (Model-View-Projection)
uniform mat4 uMVP;

void main()
{
    // Transformer la position 3D avec la matrice MVP
    gl_Position = uMVP * vec4(aPosition, 1.0);
    
    // Passer la couleur au fragment shader
    vColor = aColor;
    
}
