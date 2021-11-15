#version 150
out vec4 FragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
uniform float u_roughness;
uniform uint u_sampleCount;
void main ()
{
  float resolution_2;
  float totalWeight_3;
  vec3 prefilteredColor_4;
  vec3 V_5;
  vec3 N_6;
  vec3 tmpvar_7;
  tmpvar_7 = normalize(v_localPos);
  N_6 = tmpvar_7;
  V_5 = tmpvar_7;
  prefilteredColor_4 = vec3(0.0, 0.0, 0.0);
  totalWeight_3 = 0.0;
  resolution_2 = float(textureSize (u_environmentMap, 0).x);
  for (uint i_1 = uint(0); i_1 < u_sampleCount; i_1++) {
    float tmpvar_8;
    uint bits_9;
    bits_9 = ((i_1 << 16u) | (i_1 >> 16u));
    bits_9 = (((bits_9 & 1431655765u) << 1u) | ((bits_9 & 2863311530u) >> 1u));
    bits_9 = (((bits_9 & 858993459u) << 2u) | ((bits_9 & 3435973836u) >> 2u));
    bits_9 = (((bits_9 & 252645135u) << 4u) | ((bits_9 & 4042322160u) >> 4u));
    bits_9 = (((bits_9 & 16711935u) << 8u) | ((bits_9 & 4278255360u) >> 8u));
    tmpvar_8 = (float(bits_9) * 2.328306e-10);
    vec2 tmpvar_10;
    tmpvar_10.x = (float(i_1) / float(u_sampleCount));
    tmpvar_10.y = tmpvar_8;
    vec3 H_11;
    float tmpvar_12;
    tmpvar_12 = (u_roughness * u_roughness);
    float tmpvar_13;
    tmpvar_13 = (6.283185 * tmpvar_10.x);
    float tmpvar_14;
    tmpvar_14 = sqrt(((1.0 - tmpvar_8) / (1.0 + 
      (((tmpvar_12 * tmpvar_12) - 1.0) * tmpvar_8)
    )));
    float tmpvar_15;
    tmpvar_15 = sqrt((1.0 - (tmpvar_14 * tmpvar_14)));
    H_11.x = (cos(tmpvar_13) * tmpvar_15);
    H_11.y = (sin(tmpvar_13) * tmpvar_15);
    H_11.z = tmpvar_14;
    float tmpvar_16;
    tmpvar_16 = abs(N_6.z);
    vec3 tmpvar_17;
    if ((tmpvar_16 < 0.999)) {
      tmpvar_17 = vec3(0.0, 0.0, 1.0);
    } else {
      tmpvar_17 = vec3(1.0, 0.0, 0.0);
    };
    vec3 tmpvar_18;
    tmpvar_18 = normalize(((tmpvar_17.yzx * N_6.zxy) - (tmpvar_17.zxy * N_6.yzx)));
    vec3 tmpvar_19;
    tmpvar_19 = normalize(((
      (tmpvar_18 * H_11.x)
     + 
      (((N_6.yzx * tmpvar_18.zxy) - (N_6.zxy * tmpvar_18.yzx)) * H_11.y)
    ) + (N_6 * tmpvar_14)));
    vec3 tmpvar_20;
    tmpvar_20 = normalize(((
      (2.0 * dot (V_5, tmpvar_19))
     * tmpvar_19) - V_5));
    float tmpvar_21;
    tmpvar_21 = max (dot (N_6, tmpvar_20), 0.0);
    if ((tmpvar_21 > 0.0)) {
      float tmpvar_22;
      tmpvar_22 = (u_roughness * u_roughness);
      float tmpvar_23;
      tmpvar_23 = (tmpvar_22 * tmpvar_22);
      float tmpvar_24;
      tmpvar_24 = max (dot (N_6, tmpvar_19), 0.0);
      float tmpvar_25;
      tmpvar_25 = (((tmpvar_24 * tmpvar_24) * (tmpvar_23 - 1.0)) + 1.0);
      float tmpvar_26;
      tmpvar_26 = (12.56637 / ((6.0 * resolution_2) * resolution_2));
      float tmpvar_27;
      tmpvar_27 = (1.0/(((
        float(u_sampleCount)
       * 
        ((((tmpvar_23 / 
          ((3.141593 * tmpvar_25) * tmpvar_25)
        ) * max (
          dot (N_6, tmpvar_19)
        , 0.0)) / (4.0 * max (
          dot (tmpvar_19, V_5)
        , 0.0))) + 0.0001)
      ) + 0.0001)));
      float tmpvar_28;
      if ((u_roughness == 0.0)) {
        tmpvar_28 = 0.0;
      } else {
        tmpvar_28 = (0.5 * log2((tmpvar_27 / tmpvar_26)));
      };
      prefilteredColor_4 = (prefilteredColor_4 + (textureLod (u_environmentMap, tmpvar_20, tmpvar_28).xyz * tmpvar_21));
      totalWeight_3 = (totalWeight_3 + tmpvar_21);
    };
  };
  prefilteredColor_4 = (prefilteredColor_4 / totalWeight_3);
  vec4 tmpvar_29;
  tmpvar_29.w = 1.0;
  tmpvar_29.xyz = prefilteredColor_4;
  FragColor = tmpvar_29;
}

