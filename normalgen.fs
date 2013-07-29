in vec2 vCoord;
uniform sampler2D heightmap;

out vec4 FragColor;

void main()
{
   ivec2 coord = ivec2(vCoord);
   ivec3 offset = ivec3(0, -1, 1);
   float h0 = texelFetch(heightmap, coord + offset.yx, 0).r;
   float h1 = texelFetch(heightmap, coord + offset.zx, 0).r;
   float h2 = texelFetch(heightmap, coord + offset.xy, 0).r;
   float h3 = texelFetch(heightmap, coord + offset.xz, 0).r;

   float dx = h1 - h0;
   float dz = h3 - h2;
   FragColor = vec4(dx * 0.5 + 0.5, dz * 0.5 + 0.5, 0.0, 0.0);
}

