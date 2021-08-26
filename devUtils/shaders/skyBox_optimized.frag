#version 150
out vec4 a_outColor;
in vec3 v_texCoords;
uniform samplerCube u_skybox;
uniform float u_exposure;
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
  a_outColor.xyz = (vec3(1.0, 1.0, 1.0) - exp((
    -(a_outColor.xyz)
   * u_exposure)));
  a_outColor.xyz = pow (a_outColor.xyz, vec3(0.4545454, 0.4545454, 0.4545454));
}

