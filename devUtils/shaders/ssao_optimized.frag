#version 150
uniform u_SSAODATA ssaoDATA;
out float fragCoord;
in vec2 v_texCoords;
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_texNoise;
uniform vec3 samples[64];
uniform mat4 u_projection;
uniform mat4 u_view;
void main ()
{
  int begin_2;
  float occlusion_3;
  mat3 TBN_4;
  vec3 fragPos_5;
  fragPos_5 = texture (u_gPosition, v_texCoords).xyz;
  vec3 tmpvar_6;
  vec3 tmpvar_7;
  vec3 tmpvar_8;
  tmpvar_6 = u_view[uint(0)].xyz;
  tmpvar_7 = u_view[1u].xyz;
  tmpvar_8 = u_view[2u].xyz;
  mat3 tmpvar_9;
  float tmpvar_10;
  float tmpvar_11;
  float tmpvar_12;
  tmpvar_10 = ((tmpvar_7.y * tmpvar_8.z) - (tmpvar_8.y * tmpvar_7.z));
  tmpvar_11 = ((tmpvar_7.x * tmpvar_8.z) - (tmpvar_8.x * tmpvar_7.z));
  tmpvar_12 = ((tmpvar_7.x * tmpvar_8.y) - (tmpvar_8.x * tmpvar_7.y));
  mat3 tmpvar_13;
  tmpvar_13[0].x = tmpvar_10;
  tmpvar_13[1].x = -(tmpvar_11);
  tmpvar_13[2].x = tmpvar_12;
  tmpvar_13[0].y = ((tmpvar_8.y * tmpvar_6.z) - (tmpvar_6.y * tmpvar_8.z));
  tmpvar_13[1].y = ((tmpvar_6.x * tmpvar_8.z) - (tmpvar_8.x * tmpvar_6.z));
  tmpvar_13[2].y = ((tmpvar_8.x * tmpvar_6.y) - (tmpvar_6.x * tmpvar_8.y));
  tmpvar_13[0].z = ((tmpvar_6.y * tmpvar_7.z) - (tmpvar_7.y * tmpvar_6.z));
  tmpvar_13[1].z = ((tmpvar_7.x * tmpvar_6.z) - (tmpvar_6.x * tmpvar_7.z));
  tmpvar_13[2].z = ((tmpvar_6.x * tmpvar_7.y) - (tmpvar_7.x * tmpvar_6.y));
  tmpvar_9 = (tmpvar_13 / ((
    (tmpvar_6.x * tmpvar_10)
   - 
    (tmpvar_6.y * tmpvar_11)
  ) + (tmpvar_6.z * tmpvar_12)));
  mat3 tmpvar_14;
  tmpvar_14[0].x = tmpvar_9[0].x;
  tmpvar_14[1].x = tmpvar_9[0].y;
  tmpvar_14[2].x = tmpvar_9[0].z;
  tmpvar_14[0].y = tmpvar_9[1].x;
  tmpvar_14[1].y = tmpvar_9[1].y;
  tmpvar_14[2].y = tmpvar_9[1].z;
  tmpvar_14[0].z = tmpvar_9[2].x;
  tmpvar_14[1].z = tmpvar_9[2].y;
  tmpvar_14[2].z = tmpvar_9[2].z;
  vec3 tmpvar_15;
  tmpvar_15 = (tmpvar_14 * texture (u_gNormal, v_texCoords).xyz);
  vec4 tmpvar_16;
  tmpvar_16 = texture (u_texNoise, (v_texCoords * (vec2(textureSize (u_gPosition, 0)) / vec2(4.0, 4.0))));
  vec3 tmpvar_17;
  tmpvar_17 = normalize((tmpvar_16.xyz - (tmpvar_15 * 
    dot (tmpvar_16.xyz, tmpvar_15)
  )));
  mat3 tmpvar_18;
  tmpvar_18[uint(0)] = tmpvar_17;
  tmpvar_18[1u] = ((tmpvar_15.yzx * tmpvar_17.zxy) - (tmpvar_15.zxy * tmpvar_17.yzx));
  tmpvar_18[2u] = tmpvar_15;
  TBN_4 = tmpvar_18;
  occlusion_3 = 0.0;
  int tmpvar_19;
  tmpvar_19 = int((float(
    (64 - ssaoDATA.samplesTestSize)
  ) * abs(tmpvar_16.x)));
  begin_2 = tmpvar_19;
  for (int i_1 = tmpvar_19; i_1 < (begin_2 + ssaoDATA.samplesTestSize); i_1++) {
    vec4 offset_20;
    vec3 samplePos_21;
    samplePos_21 = (fragPos_5 + ((TBN_4 * samples[i_1]) * ssaoDATA.radius));
    vec4 tmpvar_22;
    tmpvar_22.w = 1.0;
    tmpvar_22.xyz = samplePos_21;
    offset_20 = (u_projection * tmpvar_22);
    offset_20.xyz = (offset_20.xyz / offset_20.w);
    offset_20.xyz = ((offset_20.xyz * 0.5) + 0.5);
    vec4 tmpvar_23;
    tmpvar_23 = texture (u_gPosition, offset_20.xy);
    float tmpvar_24;
    float tmpvar_25;
    tmpvar_25 = clamp ((ssaoDATA.radius / abs(
      (fragPos_5.z - tmpvar_23.z)
    )), 0.0, 1.0);
    tmpvar_24 = (tmpvar_25 * (tmpvar_25 * (3.0 - 
      (2.0 * tmpvar_25)
    )));
    float tmpvar_26;
    if ((tmpvar_23.z >= (samplePos_21.z + ssaoDATA.bias))) {
      tmpvar_26 = 1.0;
    } else {
      tmpvar_26 = 0.0;
    };
    occlusion_3 = (occlusion_3 + (tmpvar_26 * tmpvar_24));
  };
  occlusion_3 = (1.0 - (occlusion_3 / 64.0));
  fragCoord = occlusion_3;
}

