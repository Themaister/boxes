#ifndef MESH_HPP__
#define MESH_HPP__

#include "global.hpp"
#include "vertex_array.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace GL
{
   struct AABB
   {
      glm::vec3 base;
      glm::vec3 offset;

      inline glm::vec3 center() const { return base + glm::vec3(0.5f) * offset; }

      // Assumes we're not transforming to clip space or anything ...
      inline AABB transform(const glm::mat4& mat) const
      {
         AABB aabb;
         glm::vec3 corners[8];
         for (unsigned i = 0; i < 8; i++)
         {
            glm::vec3 c = corner(i);
            glm::vec4 c_trans = mat * glm::vec4(c.x, c.y, c.z, 1.0f);
            corners[8] = glm::vec3(c_trans.x, c_trans.y, c_trans.z) / glm::vec3(c_trans.w);
         }

         glm::vec3 minimum = corners[0];
         glm::vec3 maximum = minimum;
         for (auto& v : corners)
         {
            minimum = glm::min(minimum, v);
            maximum = glm::max(maximum, v);
         }

         aabb.base = minimum;
         aabb.offset = maximum - minimum;
         return aabb;
      }

      inline bool intersects_clip_space(const glm::mat4& view_proj) const
      {
         glm::vec4 corners[8];
         for (unsigned i = 0; i < 8; i++)
         {
            glm::vec3 c = corner(i);
            corners[i] = view_proj * glm::vec4(c.x, c.y, c.z, 1.0f);
         }

         // Plane equations for clip space.
         const glm::vec4 eqs[] = {
            glm::vec4{ 1,  1,  1,  1}, // Left
            glm::vec4{-1,  0,  0,  1}, // Right
            glm::vec4{ 0, -1,  0,  1}, // Top
            glm::vec4{ 0,  1,  0,  1}, // Bottom
            glm::vec4{ 0,  0,  1,  1}, // Near
            glm::vec4{ 0,  0, -1,  1}, // Far
         };

         for (auto& eq : eqs)
         {
            bool culled = true;
            for (auto& c : corners)
            {
               if (glm::dot(c, eq) > 0.0f)
               {
                  culled = false;
                  break;
               }
            }

            if (culled)
               return true;
         }

         return false;
      }

      inline glm::vec3 corner(unsigned corner) const
      {
         glm::vec3 b = base;
         if (corner & 4)
            b.z += offset.z;
         if (corner & 2)
            b.y += offset.y;
         if (corner & 1)
            b.x += offset.x;

         return b;
      }
   };

   struct Material
   {
      glm::vec3 ambient = glm::vec3(0.2f);
      glm::vec3 diffuse = glm::vec3(0.8f);
      glm::vec3 specular = glm::vec3(0.0f);
      float specular_power = 0.0f;

      std::string diffuse_map;
   };

   struct MaterialBuffer
   {
      glm::vec4 ambient;
      glm::vec4 diffuse;
      glm::vec4 specular;
      float specular_power;

      MaterialBuffer& operator=(const Material& mat)
      {
         ambient = glm::vec4(mat.ambient.x, mat.ambient.y, mat.ambient.z, 0.0f);
         diffuse = glm::vec4(mat.diffuse.x, mat.diffuse.y, mat.diffuse.z, 0.0f);
         specular = glm::vec4(mat.specular.x, mat.specular.y, mat.specular.z, 0.0f);
         specular_power = mat.specular_power;
         return *this;
      }

      explicit MaterialBuffer(const Material& mat)
      {
         *this = mat;
      }
   };

   struct Mesh
   {
      std::vector<float> vbo;
      std::vector<GLuint> ibo;
      std::vector<VertexArray::Array> arrays;
      AABB aabb;
      bool has_vertex = false;
      bool has_normal = false;
      bool has_texcoord = false;

      Material material;

      void finalize();
   };

   std::vector<Mesh> load_meshes_obj(const std::string& path);
}

#endif

