#version 150
out vec3 a_color;
in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;
void main ()
{
  a_color = textureLod (u_texture, v_texCoords, float(u_mip)).xyz;
}

