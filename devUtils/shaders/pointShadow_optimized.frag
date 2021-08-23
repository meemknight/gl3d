#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
uniform vec3 u_lightPos;
uniform float u_farPlane;
in vec2 v_texCoord;
in vec4 v_fragPos;
void main ()
{
  if ((u_hasTexture != 0)) {
    vec4 tmpvar_1;
    tmpvar_1 = texture (u_albedoSampler, v_texCoord);
    if ((tmpvar_1.w <= 0.1)) {
      discard;
    };
  };
  vec3 x_2;
  x_2 = (v_fragPos.xyz - u_lightPos);
  gl_FragDepth = (sqrt(dot (x_2, x_2)) / u_farPlane);
}

