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

in vec2 aVertex;
in vec2 aTexCoord;
in vec3 aNormal;

out vec2 vTex;
out vec3 vWorldPos;

void main()
{
   vec4 world = vec4(aVertex.x - 64.0, aVertex.y - 64.0, -10.0, 1.0);
   gl_Position = vp * world;
   vTex = vec2(1.0, -1.0) * aVertex / 128.0;
   vWorldPos = world.xyz;
}

