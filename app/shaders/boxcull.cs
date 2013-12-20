layout(local_size_x = 64) in;

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

layout(binding = 0, offset = 4) uniform atomic_uint lod0_cnt; // Outputs to instance variable.

layout(binding = 0) buffer SourceData
{
   vec4 pos[];
} source_data;

layout(binding = 1) buffer DestData
{
   vec4 pos[];
} culled;

void main()
{
   uint invocation = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationID.x;
   vec4 point = source_data.pos[invocation];
   vec4 pos = vec4(point.xyz, 1.0);
   for (int i = 0; i < 6; i++)
      if (dot(pos, global_vert.frustum[i]) < -point.w) // Culled
         return;

   uint counter = atomicCounterIncrement(lod0_cnt);
   culled.pos[counter] = point;
}

