#version 330 core

in vec3 vColor;
out vec4 fragColor;

// Variable uniform pour modifier la couleur (teinter)
uniform vec3 uColorMod;

void main()
{

    fragColor = vec4(vColor * uColorMod, 1.0);
}
