#version 150
out vec4 a_outBloom;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float u_exposure;
uniform float u_tresshold;
void main ()
{
  vec3 color_1;
  color_1 = (texture (u_texture, v_texCoords).xyz * u_exposure);
  mat3 tmpvar_2;
  tmpvar_2[0].x = 0.59719;
  tmpvar_2[1].x = 0.35458;
  tmpvar_2[2].x = 0.04823;
  tmpvar_2[0].y = 0.076;
  tmpvar_2[1].y = 0.90834;
  tmpvar_2[2].y = 0.01566;
  tmpvar_2[0].z = 0.0284;
  tmpvar_2[1].z = 0.13383;
  tmpvar_2[2].z = 0.83777;
  color_1 = (tmpvar_2 * color_1);
  mat3 tmpvar_3;
  tmpvar_3[0].x = 1.60475;
  tmpvar_3[1].x = -0.53108;
  tmpvar_3[2].x = -0.07367;
  tmpvar_3[0].y = -0.10208;
  tmpvar_3[1].y = 1.10813;
  tmpvar_3[2].y = -0.00605;
  tmpvar_3[0].z = -0.00327;
  tmpvar_3[1].z = -0.07276;
  tmpvar_3[2].z = 1.07602;
  color_1 = (tmpvar_3 * ((
    (color_1 * (color_1 + 0.0245786))
   - 9.0537e-5) / (
    (color_1 * ((0.983729 * color_1) + 0.432951))
   + 0.238081)));
  vec3 tmpvar_4;
  tmpvar_4 = clamp (color_1, 0.0, 1.0);
  color_1 = tmpvar_4;
  float tmpvar_5;
  tmpvar_5 = dot (tmpvar_4, vec3(0.2126, 0.7152, 0.0722));
  if ((tmpvar_5 > u_tresshold)) {
    vec4 tmpvar_6;
    tmpvar_6.w = 1.0;
    tmpvar_6.xyz = tmpvar_4;
    a_outBloom = tmpvar_6;
  } else {
    a_outBloom = vec4(0.0, 0.0, 0.0, 1.0);
  };
  a_outBloom = clamp (a_outBloom, 0.0, 1000.0);
}

