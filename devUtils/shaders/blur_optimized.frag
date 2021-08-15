#version 150
in vec2 v_texCoords;
uniform sampler2D u_ssaoInput;
out float fragColor;
void main ()
{
  float result_1;
  vec2 texelSize_2;
  texelSize_2 = (1.0/(vec2(textureSize (u_ssaoInput, 0))));
  result_1 = texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, -2.0) * texelSize_2))).x;
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, -1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, 0.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, 1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, -2.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords - texelSize_2)).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, 0.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, 1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, -2.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, -1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, v_texCoords).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, 1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, -2.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, -1.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, 0.0) * texelSize_2))).x);
  result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + texelSize_2)).x);
  fragColor = (result_1 / 16.0);
}

