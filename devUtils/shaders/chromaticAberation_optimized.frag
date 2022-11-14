#version 150
out vec4 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_finalColorTexture;
uniform sampler2D u_DepthTexture;
uniform ivec2 u_windowSize;
uniform float u_strength;
uniform float u_near;
uniform float u_far;
uniform float u_unfocusDistance;
void main ()
{
  vec3 finalColor_1;
  vec2 fragC_2;
  vec2 tmpvar_3;
  vec2 tmpvar_4;
  tmpvar_4 = vec2(u_windowSize);
  tmpvar_3 = (u_strength / tmpvar_4);
  fragC_2 = (gl_FragCoord.xy / tmpvar_4);
  fragC_2 = (fragC_2 * 2.0);
  fragC_2 = (fragC_2 - 1.0);
  vec2 tmpvar_5;
  tmpvar_5 = -(fragC_2);
  vec2 tmpvar_6;
  tmpvar_6 = (tmpvar_5 * tmpvar_3);
  vec2 tmpvar_7;
  tmpvar_7 = ((tmpvar_5 * tmpvar_3) * 0.5);
  vec4 tmpvar_8;
  tmpvar_8 = texture (u_DepthTexture, v_texCoords);
  float tmpvar_9;
  float tmpvar_10;
  tmpvar_10 = (2.0 * u_near);
  float tmpvar_11;
  tmpvar_11 = (u_far - u_near);
  float tmpvar_12;
  tmpvar_12 = (u_far + u_near);
  tmpvar_9 = ((tmpvar_10 * u_far) / (tmpvar_12 - (
    ((tmpvar_8.x * 2.0) - 1.0)
   * tmpvar_11)));
  float tmpvar_13;
  tmpvar_13 = ((tmpvar_10 * u_far) / (tmpvar_12 - (
    ((tmpvar_8.x * 2.0) - 1.0)
   * tmpvar_11)));
  float tmpvar_14;
  tmpvar_14 = ((tmpvar_10 * u_far) / (tmpvar_12 - (
    ((tmpvar_8.x * 2.0) - 1.0)
   * tmpvar_11)));
  if (((tmpvar_9 < u_unfocusDistance) && (tmpvar_14 < tmpvar_9))) {
    finalColor_1.x = texture (u_finalColorTexture, v_texCoords).x;
  } else {
    finalColor_1.x = texture (u_finalColorTexture, (v_texCoords + tmpvar_6)).x;
  };
  if (((tmpvar_13 < u_unfocusDistance) && (tmpvar_14 < tmpvar_13))) {
    finalColor_1.y = texture (u_finalColorTexture, v_texCoords).y;
  } else {
    finalColor_1.y = texture (u_finalColorTexture, (v_texCoords + tmpvar_7)).y;
  };
  finalColor_1.z = texture (u_finalColorTexture, v_texCoords).z;
  vec4 tmpvar_15;
  tmpvar_15.w = 1.0;
  tmpvar_15.xyz = finalColor_1;
  a_color = tmpvar_15;
}

