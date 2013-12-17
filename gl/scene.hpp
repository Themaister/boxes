#ifndef SCENE_HPP__
#define SCENE_HPP__

#include "global.hpp"
#include "mesh.hpp"
#include <functional>
#include <cstdint>
#include <vector>

namespace GL
{
   class Shader;
   struct Drawable
   {
      Shader *shader;
      std::uintptr_t shader_key;

      AABB aabb;
      glm::mat4 model_transform;
      std::function<void ()> draw;

      float depth; // Set by render_draw_list before sorting elements.
   };

   using DrawList = std::vector<Drawable>;

   void render_draw_list(const glm::mat4& view_proj, const DrawList& list,
         std::function<void (Shader*, std::uintptr_t shader_key)> start_shader);
}

#endif

