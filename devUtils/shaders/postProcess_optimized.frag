#version 150
out vec4 a_color;
in vec2 v_texCoords;
uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;
uniform sampler2D u_bloomNotBluredTexture;
uniform float u_bloomIntensity;
uniform float u_exposure;
uniform int u_useSSAO;
uniform float u_ssaoExponent;
uniform sampler2D u_ssao;
void main ()
{
  float ssaof_1;
  vec4 tmpvar_2;
  tmpvar_2 = texture (u_colorTexture, v_texCoords);
  ssaof_1 = 1.0;
  if ((u_useSSAO != 0)) {
    ssaof_1 = pow (texture (u_ssao, v_texCoords).x, u_ssaoExponent);
  } else {
    ssaof_1 = 1.0;
  };
  a_color.xyz = ((texture (u_bloomTexture, v_texCoords).xyz * u_bloomIntensity) + ((texture (u_bloomNotBluredTexture, v_texCoords).xyz + tmpvar_2.xyz) * ssaof_1));
  vec3 color_3;
  color_3 = (a_color.xyz * u_exposure);
  mat3 tmpvar_4;
  tmpvar_4[0].x = 0.59719;
  tmpvar_4[1].x = 0.35458;
  tmpvar_4[2].x = 0.04823;
  tmpvar_4[0].y = 0.076;
  tmpvar_4[1].y = 0.90834;
  tmpvar_4[2].y = 0.01566;
  tmpvar_4[0].z = 0.0284;
  tmpvar_4[1].z = 0.13383;
  tmpvar_4[2].z = 0.83777;
  color_3 = (tmpvar_4 * color_3);
  mat3 tmpvar_5;
  tmpvar_5[0].x = 1.60475;
  tmpvar_5[1].x = -0.53108;
  tmpvar_5[2].x = -0.07367;
  tmpvar_5[0].y = -0.10208;
  tmpvar_5[1].y = 1.10813;
  tmpvar_5[2].y = -0.00605;
  tmpvar_5[0].z = -0.00327;
  tmpvar_5[1].z = -0.07276;
  tmpvar_5[2].z = 1.07602;
  color_3 = (tmpvar_5 * ((
    (color_3 * (color_3 + 0.0245786))
   - 9.0537e-5) / (
    (color_3 * ((0.983729 * color_3) + 0.432951))
   + 0.238081)));
  vec3 tmpvar_6;
  tmpvar_6 = clamp (color_3, 0.0, 1.0);
  color_3 = tmpvar_6;
  a_color.xyz = pow (tmpvar_6, vec3(0.4545454, 0.4545454, 0.4545454));
  a_color.w = tmpvar_2.w;
}

