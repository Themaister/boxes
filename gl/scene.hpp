#ifndef SCENE_HPP__
#define SCENE_HPP__

#include "global.hpp"
#include "mesh.hpp"
#include <functional>
#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>

namespace GL
{
   struct Renderable
   {
      virtual void set_cache_depth(float depth) = 0;
      virtual const AABB& get_aabb() const = 0;
      virtual glm::mat4 get_model_transform() const = 0;
      virtual bool compare_less(const Renderable& other) const = 0;
      virtual void render() = 0;
   };

   class RenderQueue
   {
      public:
         using DrawList = std::vector<Renderable*>;

         inline void set_view_proj(const glm::mat4& vp) { view_proj = vp; }

         inline void begin()
         {
            draw_list.clear();
         }

         inline void end()
         {
            draw_list.erase(std::remove_if(std::begin(draw_list), std::end(draw_list), [this](Renderable* draw) -> bool {
                     auto aabb = draw->get_aabb();
                     auto model = draw->get_model_transform();
                     auto transformed_aabb = aabb.transform(model);

                     // Distance in clip space from near plane, plane eq (0, 0, 1, 1).
                     auto c = transformed_aabb.center();
                     auto clip = view_proj * glm::vec4(c.x, c.y, c.z, 1.0f);
                     draw->set_cache_depth(clip.z + clip.w);

                     return !transformed_aabb.intersects_clip_space(view_proj);
                  }), std::end(draw_list));

            // Sort based on various criteria.
            std::sort(std::begin(draw_list), std::end(draw_list),
                  [](Renderable* a, Renderable* b) -> bool {
                     return a->compare_less(*b);
                  });
         }

         inline void push(Renderable *elem)
         {
            draw_list.push_back(elem);
         }

         inline void render()
         {
            for (auto& elem : draw_list)
               elem->render();
         }

      private:
         DrawList draw_list;
         glm::mat4 view_proj;
   };
}

#endif

