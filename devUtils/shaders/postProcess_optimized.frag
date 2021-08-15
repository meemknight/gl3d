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
  a_color.xyz = (vec3(1.0, 1.0, 1.0) - exp((
    -(a_color.xyz)
   * u_exposure)));
  a_color.xyz = pow (a_color.xyz, vec3(0.4545454, 0.4545454, 0.4545454));
  a_color.w = tmpvar_2.w;
}

