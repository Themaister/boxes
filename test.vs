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

uniform ModelTransform
{
   mat4 model;
};

in vec2 aVertex;

out vec3 vWorldPos;
out vec3 vNormal;

uniform sampler2D heightmap;
uniform sampler2D normalmap;

void main()
{
   float y = 40.0;
   float height = texelFetch(heightmap, ivec2(aVertex.x, aVertex.y), 0).r * y;

   vec2 normal = texelFetch(normalmap, ivec2(aVertex.x, aVertex.y), 0).rg * y;
   vNormal = normalize(vec3(-normal.x, 2.0, -normal.y));

   vec4 world = model * vec4(aVertex.x, height, aVertex.y, 1.0);
   gl_Position = vp * world;
   vWorldPos = world.xyz;
}

