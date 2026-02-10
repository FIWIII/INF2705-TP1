#version 330 core

// Entrée : couleur interpolée depuis le vertex shader.
in vec3 vColor;

// Modulation de couleur globale (pour les clignotants et phares).
uniform vec3 uColorMod;

out vec4 fragColor;

void main()
{
    // Multiplication par uColorMod pour coloriser les fragments selon l'état du composant.
    fragColor = vec4(vColor * uColorMod, 1.0);
}
