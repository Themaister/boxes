uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
};

uniform samplerCube skybox;

in vec3 vDirection;
out vec4 FragColor;

float saturate(float a)
{
   return clamp(a, 0.0, 1.0);
}

void main()
{
   vec3 dir = normalize(vDirection);
   vec4 col = texture(skybox, dir);
   vec3 to_light = light_pos.xyz - camera_pos.xyz;
   col += pow(saturate(dot(dir, normalize(to_light))), 200.0 * length(to_light));
   FragColor = col;
}
