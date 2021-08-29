#version 150
out vec4 outColor;
noperspective in vec2 v_texCoords;
uniform sampler2D u_depth;
void main ()
{
  float tmpvar_1;
  tmpvar_1 = texture (u_depth, v_texCoords).x;
  vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.x = tmpvar_1;
  tmpvar_2.y = tmpvar_1;
  tmpvar_2.z = tmpvar_1;
  outColor = tmpvar_2;
}

