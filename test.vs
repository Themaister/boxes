uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_proj;
};

in vec4 aVertex;
in vec2 aTexCoord;
in vec3 aNormal;

out vec2 vTex;
out vec3 vNormal;

void main()
{
   gl_Position = vp * aVertex;

   vTex = aTexCoord;
   vec4 norm = vp * vec4(aNormal, 0.0);
   vNormal = norm.xyz;
}

