#version 150
out vec4 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = texture (u_texture, v_texCoords).xyz;
  a_color = tmpvar_1;
}

