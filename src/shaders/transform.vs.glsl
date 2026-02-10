#version 330 core

// Entrées : position 3D et couleur par sommet.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 uMVP;
out vec3 vColor;

void main()
{
    vColor = aColor;
    // Transformation des sommets à l'aide de la matrice MVP.
    gl_Position = uMVP * vec4(aPosition, 1.0);
}
