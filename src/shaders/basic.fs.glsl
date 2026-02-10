#version 330 core

//Entrée : couleur depuis le vertex shader
in vec3 vColor;

//Sortie : couleur du fragment
out vec4 fragColor;

void main()
{
    // Assigner la couleur avec alpha = 1.0 (opaque)
    fragColor = vec4(vColor, 1.0);
}
