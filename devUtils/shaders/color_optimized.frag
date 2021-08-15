#version 150
out vec4 outColor;
in vec3 v_colors;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = v_colors;
  outColor = tmpvar_1;
}

