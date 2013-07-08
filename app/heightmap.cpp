#include "../global.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"
#include <memory>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

class HeightGrid
{
   public:
      void init(unsigned size)
      {
         init_vertices(size);
         init_indices(size);
         vector<VertexArray::Array> arrays = {
            { Shader::VertexLocation, 2, GL_SHORT, GL_FALSE, 0, 0 },
         };
         array.setup(arrays, &vertex, &elems);
      }

      void render()
      {
         array.bind();
         glDrawElements(GL_TRIANGLE_STRIP, elements, GL_UNSIGNED_SHORT, nullptr);
         array.unbind();
      }

   private:
      VertexArray array;
      Buffer vertex;
      Buffer elems;
      unsigned elements = 0;

      void init_vertices(unsigned size)
      {
         vector<GLshort> vertices;
         vertices.reserve(2 * size * size);
         for (unsigned y = 0; y < size; y++)
         {
            for (unsigned x = 0; x < size; x++)
            {
               vertices.push_back(x);
               vertices.push_back(-int(size - 1 - y));
            }
         }

         vertex.init(GL_ARRAY_BUFFER, 2 * size * size * sizeof(GLshort), Buffer::None, vertices.data());
      }

      void init_indices(unsigned size)
      {
         elements = (size - 1) * (2 * size + 1);

         vector<GLushort> indices;
         indices.reserve(elements);

         int pos = 0;
         for (unsigned y = 0; y < size - 1; y++)
         {
            int dir_odd = -int(size) + ((y & 1) ? -1 : 1);
            int dir_even = size;

            for (unsigned x = 0; x < 2 * size - 1; x++)
            {
               indices.push_back(pos);
               pos += (x & 1) ? dir_odd : dir_even;
            }
            indices.push_back(pos);
            indices.push_back(pos);
         }

         elems.init(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(GLushort), Buffer::None, indices.data());
      }
};

class HeightmapApp : public LibretroGLApplication
{
   public:
      void get_system_info(retro_system_info& info) const override
      {
         info.library_name = "Heightmap";
         info.library_version = "v1";
         info.valid_extensions = nullptr;
         info.need_fullpath = false;
         info.block_extract = false;
      }

      void get_system_av_info(retro_system_av_info& info) const override
      {
         info.timing.fps = 60.0;
         info.timing.sample_rate = 30000.0;
         info.geometry.base_width = 320;
         info.geometry.base_height = 240;
         info.geometry.max_width = 1920;
         info.geometry.max_height = 1080;
         info.geometry.aspect_ratio = 16.0f / 9.0f;
      }

      string get_application_name() const override
      {
         return "Heightmap";
      }

      string get_application_name_short() const override
      {
         return "heightmap";
      }

      vector<Resolution> get_resolutions() const override
      {
         vector<Resolution> res;
         res.push_back({320, 240});
         res.push_back({640, 480});
         res.push_back({1280, 720});
         res.push_back({1920, 1080});
         return res;
      }

      void update_global_data()
      {
         global.proj = perspective(45.0f, float(width) / float(height), 0.1f, 500.0f);
         global.inv_proj = inverse(global.proj);
         global.view = lookAt(player_pos, player_pos + player_look_dir, vec3(0, 1, 0));
         global.inv_view = inverse(global.view);

         global.vp = global.proj * global.view;
         global.inv_vp = inverse(global.vp);

         GlobalTransforms *buf;
         if (global_buffer.map(buf))
         {
            *buf = global;
            global_buffer.unmap();
         }
      }

      void viewport_changed(const Resolution& res) override
      {
         width = res.width;
         height = res.height;

         update_global_data();
      }

      void update_input(const InputState::Analog& analog, const InputState::Buttons& buttons)
      {
         player_view_deg_y += analog.rx * -2.0f;
         player_view_deg_x += analog.ry * -1.5f;
         player_view_deg_x = clamp(player_view_deg_x, -80.0f, 80.0f);

         mat4 rotate_x = rotate(mat4(1.0), player_view_deg_x, vec3(1, 0, 0));
         mat4 rotate_y = rotate(mat4(1.0), player_view_deg_y, vec3(0, 1, 0));
         mat4 rotate_y_right = rotate(mat4(1.0), player_view_deg_y - 90.0f, vec3(0, 1, 0));

         player_look_dir = vec3(rotate_y * rotate_x * vec4(0, 0, -1, 1));
         vec3 right_walk_dir = vec3(rotate_y_right * vec4(0, 0, -1, 1));


         vec3 mod_speed = buttons.r ? vec3(2.0f) : vec3(1.0f);
         vec3 velocity = player_look_dir * vec3(analog.y * -0.25f) +
            right_walk_dir * vec3(analog.x * 0.25f);

         player_pos += velocity * mod_speed;
         update_global_data();
      }

      void run(const InputState& input, GLuint) override
      {
         auto analog = input.analog;
         if (fabsf(analog.x) < 0.3f)
            analog.x = 0.0f;
         if (fabsf(analog.y) < 0.3f)
            analog.y = 0.0f;
         if (fabsf(analog.rx) < 0.3f)
            analog.rx = 0.0f;
         if (fabsf(analog.ry) < 0.3f)
            analog.ry = 0.0f;
         update_input(analog, input.pressed);

         glViewport(0, 0, width, height);
         glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

         glEnable(GL_CULL_FACE);
         glEnable(GL_DEPTH_TEST);

         global_buffer.bind();
         global_frag_buffer.bind();
         shader.use();

         grid.render();

         global_buffer.unbind();
         global_frag_buffer.unbind();
         shader.unbind();
      }

      void get_context_version(unsigned& major, unsigned& minor) const override
      {
         major = 3;
         minor = 1;
      }

      void load() override
      {
         global_buffer.init(GL_UNIFORM_BUFFER, sizeof(global), Buffer::WriteOnly);

         vec4 global_color(0.8f, 0.6f, 0.2f, 1.0f);
         global_frag_buffer.init(GL_UNIFORM_BUFFER, sizeof(global_color), Buffer::None, value_ptr(global_color), 1);

         player_pos = vec3(0.0f);
         player_look_dir = vec3(0, 0, -1);
         player_view_deg_x = 0.0f;
         player_view_deg_y = 0.0f;

         shader.init(path("test.vs"), path("test.fs"));
         grid.init(128);
      }

   private:
      unsigned width = 0;
      unsigned height = 0;

      float player_view_deg_x = 0.0f;
      float player_view_deg_y = 0.0f;
      vec3 player_pos;
      vec3 player_look_dir{0, 0, -1};

      struct GlobalTransforms
      {
         mat4 vp;
         mat4 view;
         mat4 proj;
         mat4 inv_vp;
         mat4 inv_view;
         mat4 inv_proj;
      };

      GlobalTransforms global;
      Buffer global_buffer;
      Buffer global_frag_buffer;
      Shader shader;

      HeightGrid grid;
};

unique_ptr<LibretroGLApplication> libretro_gl_application_create()
{
   return unique_ptr<LibretroGLApplication>(new HeightmapApp);
}

