#version 150
out vec4 a_outColor;
in vec3 v_texCoords;
uniform samplerCube u_skybox;
uniform float u_exposure;
uniform vec3 u_ambient;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1 = textureLod (u_skybox, v_texCoords, 2.0);
  a_outColor.w = tmpvar_1.w;
  a_outColor.xyz = (tmpvar_1.xyz * u_ambient);
  a_outColor.xyz = (vec3(1.0, 1.0, 1.0) - exp((
    -(a_outColor.xyz)
   * u_exposure)));
  a_outColor.xyz = pow (a_outColor.xyz, vec3(0.4545454, 0.4545454, 0.4545454));
}

