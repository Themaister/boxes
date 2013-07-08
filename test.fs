uniform GlobalFragmentData
{
   vec4 color_mod;
};

in vec2 vTex;
in vec3 vNormal;
in float vHeight;

out vec4 FragColor;

void main()
{
   float height = vHeight * 0.5 + 0.5;
   FragColor = color_mod * height * vec4(1.0);
}

