
uniform samplerCube uSampler1;

in vec3 vDirection;
out vec4 FragColor;

void main()
{
   FragColor = texture(uSampler1, normalize(vDirection));
}
