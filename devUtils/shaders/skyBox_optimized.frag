#version 150
out vec4 a_outColor;
in vec3 v_texCoords;
uniform samplerCube u_skybox;
uniform vec3 u_ambient;
uniform int u_skyBoxPresent;
void main ()
{
  vec3 tmpvar_1;
  tmpvar_1 = pow (u_ambient, vec3(2.2, 2.2, 2.2));
  if ((u_skyBoxPresent != 0)) {
    vec4 tmpvar_2;
    tmpvar_2 = textureLod (u_skybox, v_texCoords, 2.0);
    a_outColor.w = tmpvar_2.w;
    a_outColor.xyz = (tmpvar_2.xyz * tmpvar_1);
  } else {
    a_outColor.xyz = tmpvar_1;
  };
}

