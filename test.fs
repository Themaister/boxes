uniform GlobalFragmentData
{
   vec4 color_mod;
};

in vec2 vTex;
in vec3 vNormal;
in float vHeight;

uniform sampler2D uSampler0;

out vec4 FragColor;

void main()
{
   vec4 col = texture(uSampler0, vTex) * color_mod;
   float height = vHeight * 0.5 + 0.5;
#if FOO == 1
   FragColor = 2.0 * col * height;
#else
   FragColor = col * height;
#endif
}

