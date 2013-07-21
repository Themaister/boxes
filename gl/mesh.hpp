#ifndef MESH_HPP__
#define MESH_HPP__

#include "global.hpp"
#include "vertex_array.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace GL
{
   struct Mesh
   {
      std::vector<float> vbo;
      std::vector<uint32_t> ibo;
      std::vector<VertexArray::Array> arrays;
      bool has_vertex = false;
      bool has_normal = false;
      bool has_texcoord = false;
   };

   std::vector<Mesh> load_scene(const std::string& path);
}

#endif

