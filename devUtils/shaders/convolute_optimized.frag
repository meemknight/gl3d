#version 150
out vec4 fragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
uniform float u_sampleQuality;
void main ()
{
  float nrSamples_2;
  float sampleDelta_3;
  vec3 right_4;
  vec3 up_5;
  vec3 irradiance_6;
  vec3 normal_7;
  vec3 tmpvar_8;
  tmpvar_8 = normalize(v_localPos);
  normal_7 = tmpvar_8;
  irradiance_6 = vec3(0.0, 0.0, 0.0);
  vec3 tmpvar_9;
  tmpvar_9 = normalize(((vec3(1.0, 0.0, 0.0) * tmpvar_8.zxy) - (vec3(0.0, 0.0, 1.0) * tmpvar_8.yzx)));
  right_4 = tmpvar_9;
  up_5 = normalize(((tmpvar_8.yzx * tmpvar_9.zxy) - (tmpvar_8.zxy * tmpvar_9.yzx)));
  sampleDelta_3 = u_sampleQuality;
  nrSamples_2 = 0.0;
  for (float phi_1 = 0.0; phi_1 < 6.283185; phi_1 = (phi_1 + sampleDelta_3)) {
    for (float theta_10 = 0.0; theta_10 < 1.570796; theta_10 = (theta_10 + sampleDelta_3)) {
      float tmpvar_11;
      tmpvar_11 = cos(theta_10);
      vec3 tmpvar_12;
      tmpvar_12.x = (sin(theta_10) * cos(phi_1));
      tmpvar_12.y = (sin(theta_10) * sin(phi_1));
      tmpvar_12.z = tmpvar_11;
      irradiance_6 = (irradiance_6 + ((texture (u_environmentMap, 
        (((tmpvar_12.x * right_4) + (tmpvar_12.y * up_5)) + (tmpvar_11 * normal_7))
      ).xyz * 
        cos(theta_10)
      ) * sin(theta_10)));
      nrSamples_2 += 1.0;
    };
  };
  irradiance_6 = (irradiance_6 * (3.141593 / nrSamples_2));
  vec4 tmpvar_13;
  tmpvar_13.w = 1.0;
  tmpvar_13.xyz = irradiance_6;
  fragColor = tmpvar_13;
}

