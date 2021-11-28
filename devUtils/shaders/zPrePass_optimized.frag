#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;
void main ()
{
  if ((u_hasTexture != 0)) {
    vec4 tmpvar_1;
    tmpvar_1 = texture (u_albedoSampler, v_texCoord);
    if (((tmpvar_1.w * 255.0) < 1.0)) {
      discard;
    };
  };
}

