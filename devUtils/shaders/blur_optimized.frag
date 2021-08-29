#version 150
noperspective in vec2 v_texCoords;
uniform sampler2D u_ssaoInput;
out float fragColor;
void main ()
{
  fragColor = texture (u_ssaoInput, v_texCoords).x;
}

