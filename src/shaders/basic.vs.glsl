#version 330 core

//Emtrée : position et couleur sommets
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

//Sortie : DONÉES VERS LE FRAGMENT SHADER
out vec3 vColor;    //Couleur à passer au fragment shader

void main()
{
    // Transformer la position 2D en position 4D pour OpenGL
    // (x, y, z=0, w=1)
    gl_Position = vec4(aPosition, 0.0, 1.0);
    
    // Passer la couleur au fragment shader
    vColor = aColor;
}
