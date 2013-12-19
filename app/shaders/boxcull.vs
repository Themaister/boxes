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
   vec4 frustum[6];
} global_vert;

layout(location = VERTEX) in vec4 aPoint;

layout(binding = 0, offset = 4) uniform atomic_uint lod0_cnt; // Outputs to instance variable.

layout(binding = 0) buffer InstanceData
{
   vec4 pos[];
} culled;

void main()
{
   gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
   vec4 pos = vec4(aPoint.xyz, 1.0);
   for (int i = 0; i < 6; i++)
      if (dot(pos, global_vert.frustum[i]) < -aPoint.w) // Culled
         return;

   uint counter = atomicCounterIncrement(lod0_cnt);
   culled.pos[counter] = aPoint;
}

