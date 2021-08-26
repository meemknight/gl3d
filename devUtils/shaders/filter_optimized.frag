#version 150
out vec4 a_outBloom;
in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float u_exposure;
uniform float u_tresshold;
void main ()
{
  vec3 hdrCorrectedColor_1;
  vec3 tmpvar_2;
  tmpvar_2 = texture (u_texture, v_texCoords).xyz;
  hdrCorrectedColor_1 = (vec3(1.0, 1.0, 1.0) - exp((
    -(tmpvar_2)
   * u_exposure)));
  hdrCorrectedColor_1 = pow (hdrCorrectedColor_1, vec3(0.4545454, 0.4545454, 0.4545454));
  float tmpvar_3;
  tmpvar_3 = dot (hdrCorrectedColor_1, vec3(0.2126, 0.7152, 0.0722));
  if ((tmpvar_3 > u_tresshold)) {
    vec4 tmpvar_4;
    tmpvar_4.w = 1.0;
    tmpvar_4.xyz = tmpvar_2;
    a_outBloom = tmpvar_4;
  } else {
    a_outBloom = vec4(0.0, 0.0, 0.0, 1.0);
  };
  a_outBloom = clamp (a_outBloom, 0.0, 1000.0);
}

