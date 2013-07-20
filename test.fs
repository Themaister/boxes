uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 view_nt;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_view_nt;
   mat4 inv_proj;
   vec4 camera_pos;
};

in vec2 vTex;
in vec3 vWorldPos;

uniform sampler2D uSampler0;
uniform samplerCube uSampler1;

out vec4 FragColor;

void main()
{
   vec3 to_plane = normalize(vWorldPos - camera_pos.xyz);
   vec3 normal = vec3(0.02 * sin(vWorldPos.x * 0.15), 0.01 * cos(vWorldPos.y * 0.29), 1.0);
   normal = normalize(normal);
   vec3 reflect_dir = reflect(to_plane, normal);
   vec4 col_reflect = texture(uSampler1, reflect_dir);

   float fres = 1.0;
   vec3 refract_dir = refract(to_plane, normal, 1.4);
   vec4 col_refract = vec4(0.0);
   if (refract_dir != vec3(0.0))
   {
      col_refract = texture(uSampler1, refract_dir); 
      fres = pow(1.0 - dot(refract_dir, -normal), 5.0);
   }

   FragColor = mix(col_refract, col_reflect, fres);
}
