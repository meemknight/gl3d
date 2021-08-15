#version 150
out vec4 a_outColor;
uniform vec3 u_color = vec3(0.7, 0.7, 0.1);
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = u_color;
  a_outColor = tmpvar_1;
}

