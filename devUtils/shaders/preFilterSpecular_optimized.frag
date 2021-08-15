#version 150
out vec4 FragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
uniform float u_roughness;
void main ()
{
  float totalWeight_2;
  vec3 prefilteredColor_3;
  vec3 V_4;
  vec3 N_5;
  vec3 tmpvar_6;
  tmpvar_6 = normalize(v_localPos);
  N_5 = tmpvar_6;
  V_4 = tmpvar_6;
  prefilteredColor_3 = vec3(0.0, 0.0, 0.0);
  totalWeight_2 = 0.0;
  for (uint i_1 = uint(0); i_1 < 1024u; i_1++) {
    float tmpvar_7;
    uint bits_8;
    bits_8 = ((i_1 << 16u) | (i_1 >> 16u));
    bits_8 = (((bits_8 & 1431655765u) << 1u) | ((bits_8 & 2863311530u) >> 1u));
    bits_8 = (((bits_8 & 858993459u) << 2u) | ((bits_8 & 3435973836u) >> 2u));
    bits_8 = (((bits_8 & 252645135u) << 4u) | ((bits_8 & 4042322160u) >> 4u));
    bits_8 = (((bits_8 & 16711935u) << 8u) | ((bits_8 & 4278255360u) >> 8u));
    tmpvar_7 = (float(bits_8) * 2.328306e-10);
    vec2 tmpvar_9;
    tmpvar_9.x = (float(i_1) / 1024.0);
    tmpvar_9.y = tmpvar_7;
    vec3 H_10;
    float tmpvar_11;
    tmpvar_11 = (u_roughness * u_roughness);
    float tmpvar_12;
    tmpvar_12 = (6.283185 * tmpvar_9.x);
    float tmpvar_13;
    tmpvar_13 = sqrt(((1.0 - tmpvar_7) / (1.0 + 
      (((tmpvar_11 * tmpvar_11) - 1.0) * tmpvar_7)
    )));
    float tmpvar_14;
    tmpvar_14 = sqrt((1.0 - (tmpvar_13 * tmpvar_13)));
    H_10.x = (cos(tmpvar_12) * tmpvar_14);
    H_10.y = (sin(tmpvar_12) * tmpvar_14);
    H_10.z = tmpvar_13;
    float tmpvar_15;
    tmpvar_15 = abs(N_5.z);
    vec3 tmpvar_16;
    if ((tmpvar_15 < 0.999)) {
      tmpvar_16 = vec3(0.0, 0.0, 1.0);
    } else {
      tmpvar_16 = vec3(1.0, 0.0, 0.0);
    };
    vec3 tmpvar_17;
    tmpvar_17 = normalize(((tmpvar_16.yzx * N_5.zxy) - (tmpvar_16.zxy * N_5.yzx)));
    vec3 tmpvar_18;
    tmpvar_18 = normalize(((
      (tmpvar_17 * H_10.x)
     + 
      (((N_5.yzx * tmpvar_17.zxy) - (N_5.zxy * tmpvar_17.yzx)) * H_10.y)
    ) + (N_5 * tmpvar_13)));
    vec3 tmpvar_19;
    tmpvar_19 = normalize(((
      (2.0 * dot (V_4, tmpvar_18))
     * tmpvar_18) - V_4));
    float tmpvar_20;
    tmpvar_20 = max (dot (N_5, tmpvar_19), 0.0);
    if ((tmpvar_20 > 0.0)) {
      float tmpvar_21;
      tmpvar_21 = (u_roughness * u_roughness);
      float tmpvar_22;
      tmpvar_22 = (tmpvar_21 * tmpvar_21);
      float tmpvar_23;
      tmpvar_23 = max (dot (N_5, tmpvar_18), 0.0);
      float tmpvar_24;
      tmpvar_24 = (((tmpvar_23 * tmpvar_23) * (tmpvar_22 - 1.0)) + 1.0);
      float tmpvar_25;
      tmpvar_25 = (1.0/(((1024.0 * 
        ((((tmpvar_22 / 
          ((3.141593 * tmpvar_24) * tmpvar_24)
        ) * max (
          dot (N_5, tmpvar_18)
        , 0.0)) / (4.0 * max (
          dot (tmpvar_18, V_4)
        , 0.0))) + 0.0001)
      ) + 0.0001)));
      float tmpvar_26;
      if ((u_roughness == 0.0)) {
        tmpvar_26 = 0.0;
      } else {
        tmpvar_26 = (0.5 * log2((tmpvar_25 / 7.989483e-6)));
      };
      prefilteredColor_3 = (prefilteredColor_3 + (textureLod (u_environmentMap, tmpvar_19, tmpvar_26).xyz * tmpvar_20));
      totalWeight_2 = (totalWeight_2 + tmpvar_20);
    };
  };
  prefilteredColor_3 = (prefilteredColor_3 / totalWeight_2);
  vec4 tmpvar_27;
  tmpvar_27.w = 1.0;
  tmpvar_27.xyz = prefilteredColor_3;
  FragColor = tmpvar_27;
}

