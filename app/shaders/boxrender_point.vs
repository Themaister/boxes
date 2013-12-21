layout(binding = GLOBAL_VERTEX_DATA) uniform GlobalVertexData
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
   vec4 camera_vel;
   vec4 resolution;
} global_vert;

layout(location = VERTEX) in vec4 aVertex;

out VertexData
{
   vec3 normal;
   vec3 world;
} vout;

void main()
{
   vec4 world = vec4(aVertex.xyz, 1.0);

   gl_Position = global_vert.vp * world;
   gl_PointSize = 1000.0 / length(global_vert.camera_pos.xyz - world.xyz);

   vout.normal = normalize(global_vert.camera_pos.xyz - world.xyz);
   vout.world = world.xyz;
}

