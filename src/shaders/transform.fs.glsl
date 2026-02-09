#version 330 core

// TODO: La couleur des fragments est donnée à partir de la couleur
//       des vertices passée en entrée.
//       De plus, une variable uniform permet de multiplier la couleur
//       par une autre pour coloriser les fragments.

// Entrée depuis le vertex shader
in vec3 vColor;

// Sortie : couleur du fragment
out vec4 fragColor;

// Variable uniform pour modifier la couleur (teinter)
uniform vec3 uColorMod;

void main()
{
     // Multiplier la couleur par le modificateur de couleur
    fragColor = vec4(vColor * uColorMod, 1.0);
}
