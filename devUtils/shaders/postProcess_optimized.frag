#version 150
out vec4 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_colorTexture;
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
  a_color.xyz = vec3(ssaof_1);
  a_color.w = tmpvar_2.w;
}

