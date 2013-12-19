#include <gl/global.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>
#include <gl/vertex_array.hpp>
#include <gl/texture.hpp>
#include <gl/mesh.hpp>
#include <gl/aabb.hpp>
#include <gl/framebuffer.hpp>
#include <gl/scene.hpp>
#include <memory>
#include <cstdint>

using namespace std;
using namespace glm;
using namespace GL;
using namespace Util;

class Scene
{
   public:
      void init()
      {
         auto mesh = create_mesh_box();

         mesh.arrays.push_back({3, 4, GL_FLOAT, GL_FALSE, 0, 0, 1, 1});
         render_array.setup(mesh.arrays, { &vert, &culled_buffer }, &elem);

         vert.init(GL_ARRAY_BUFFER, mesh.vbo, Buffer::None);
         elem.init(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo, Buffer::None);
         drawable.indices = mesh.ibo.size();

         MaterialBuffer material(mesh.material);
         drawable.material.init(GL_UNIFORM_BUFFER, sizeof(material),
               Buffer::None, &material, Shader::Material);

         if (!mesh.material.diffuse_map.empty())
         {
            drawable.use_diffuse = true;
            drawable.tex.load_texture({Texture::Texture2D,
                  { mesh.material.diffuse_map },
                  true });
         }
         else
            drawable.use_diffuse = false;

         cull_shader.init("app/shaders/boxcull.vs", "app/shaders/boxcull.fs");
         render_shader.reserve_define("DIFFUSE_MAP", 1);
         render_shader.init("app/shaders/boxrender.vs", "app/shaders/boxrender.fs");

         culled_buffer.init(GL_ARRAY_BUFFER, 16 * 1024 * 1024, Buffer::Copy);

         drawable.culled_buffer = &culled_buffer;
         drawable.cull_shader = &cull_shader;
         drawable.render_shader = &render_shader;
         drawable.render_array = &render_array;
      }

      void render(const mat4& view_proj)
      {
         queue.set_frustum(Frustum(view_proj));
         queue.begin();
         queue.push(&drawable);
         queue.end();
         queue.render();
      }

   private:
      struct Drawable : Renderable
      {
         Shader *cull_shader;
         Shader *render_shader;
         VertexArray *render_array;
         VertexArray cull_array;
         size_t indices;

         Buffer model;
         Buffer material;
         Buffer indirect;
         Buffer *culled_buffer;

         Texture tex;
         bool use_diffuse;
         float cache_depth;

         vector<vec4> blocks;
         AABB aabb;

         Drawable()
         {
            for (int z = -1000; z <= 1000; z += 16)
               for (int y = -1000; y <= 1000; y += 16)
                  for (int x = -1000; x <= 1000; x += 16)
                     blocks.push_back(vec4(vec3(x, y, z), 1.4143f));
            aabb = AABB(vec3(-1001), vec3(1001));

            model.init(GL_ARRAY_BUFFER, blocks.size() * sizeof(vec4), Buffer::None, blocks.data());
            cull_array.setup({{ Shader::VertexLocation, 4, GL_FLOAT, GL_FALSE }}, { &model }, nullptr);
         }

         inline void set_cache_depth(float depth) override { cache_depth = depth; }
         inline const AABB& get_aabb() const override { return aabb; }
         inline bool compare_less(const Renderable& o_tmp) const override
         {
            const Drawable& o = static_cast<const Drawable&>(o_tmp);
            if (&o == this)
               return false;

            if (use_diffuse && !o.use_diffuse)
               return true;
            if (cache_depth < o.cache_depth)
               return true;

            return false;
         }

         inline void render()
         {
            // Reset indirect draw buffer.
            struct IndirectCommand
            {
               GLuint count;
               GLuint primCount;
               GLuint firstIndex;
               GLuint baseVertex;
               GLuint baseInstance;
            };
            IndirectCommand command = { GLuint(indices) }; // primCount is incremented by shaders.
            indirect.init(GL_DRAW_INDIRECT_BUFFER, sizeof(command), Buffer::Copy, &command);

            cull_shader->use();
            cull_array.bind();

            // Frustum cull instanced cubes (points) and update indirect draw buffer.
            culled_buffer->bind_indexed(GL_SHADER_STORAGE_BUFFER, 0);
            indirect.bind_indexed(GL_ATOMIC_COUNTER_BUFFER, 0); // Instance count is written here.
            glEnable(GL_RASTERIZER_DISCARD); // Only run vertex shader stage.
            glDrawArrays(GL_POINTS, 0, blocks.size());
            glDisable(GL_RASTERIZER_DISCARD);
            indirect.unbind_indexed(GL_ATOMIC_COUNTER_BUFFER, 0);
            culled_buffer->unbind_indexed(GL_SHADER_STORAGE_BUFFER, 0);

            // GL must wait until previous shader has updated data.
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

#if 0
            const IndirectCommand* cmd;
            if (indirect.map(cmd))
            {
               Log::log("Count: %u, PrimCount: %u.", cmd->count, cmd->primCount); 
               indirect.unmap();
            }
#endif

            // Render instanced data.
            render_shader->use();
            render_array->bind();
            Sampler::bind(0, Sampler::TrilinearClamp);

            material.bind();
            if (use_diffuse)
            {
               tex.bind(0);
               render_shader->set_define("DIFFUSE_MAP", 1);
            }
            else
               render_shader->set_define("DIFFUSE_MAP", 0);

            indirect.bind();
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
            indirect.unbind();

            if (use_diffuse)
               tex.unbind(0);

            Sampler::unbind(0, Sampler::TrilinearClamp);
            render_shader->unbind();
            render_array->unbind();
            material.unbind();
         }
      };

      Drawable drawable;
      Shader cull_shader;
      Shader render_shader;
      RenderQueue queue;

      Buffer vert;
      Buffer elem;
      Buffer culled_buffer;
      VertexArray render_array;
};

class BoxesApp : public LibretroGLApplication
{
   public:
      void get_system_info(retro_system_info& info) const override
      {
         info.library_name = "Boxes";
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
         info.geometry.base_height = 180;
         info.geometry.max_width = 1920;
         info.geometry.max_height = 1080;
         info.geometry.aspect_ratio = 16.0f / 9.0f;
      }

      string get_application_name() const override
      {
         return "Boxes";
      }

      string get_application_name_short() const override
      {
         return "boxes";
      }

      vector<Resolution> get_resolutions() const override
      {
         vector<Resolution> res;
         res.push_back({320, 180});
         res.push_back({640, 360});
         res.push_back({1280, 720});
         res.push_back({1920, 1080});
         return res;
      }

      void update_global_data()
      {
         global.proj = perspective(45.0f, float(width) / float(height), 0.1f, 1000.0f);
         global.inv_proj = inverse(global.proj);
         global.view = lookAt(player_pos, player_pos + player_look_dir, vec3(0, 1, 0));
         global.view_nt = lookAt(vec3(0.0f), player_look_dir, vec3(0, 1, 0));
         global.inv_view = inverse(global.view);
         global.inv_view_nt = inverse(global.view_nt);

         global.vp = global.proj * global.view;
         global.inv_vp = inverse(global.vp);

         global.camera_pos = vec4(player_pos.x, player_pos.y, player_pos.z, 0.0);
         global.frustum = Frustum(global.vp);

         global_fragment.camera_pos = global.camera_pos;
         global_fragment.light_pos = vec4(50.0, 50.0, 0.0, 1.0);
         global_fragment.light_color = vec4(1.0);
         global_fragment.light_ambient = vec4(0.2);

         GlobalTransforms *buf;
         if (global_buffer.map(buf))
         {
            *buf = global;
            global_buffer.unmap();
         }

         GlobalFragmentData *frag_buf;
         if (global_fragment_buffer.map(frag_buf))
         {
            *frag_buf = global_fragment;
            global_fragment_buffer.unmap();
         }
      }

      void viewport_changed(const Resolution& res) override
      {
         width = res.width;
         height = res.height;

         update_global_data();
      }

      void update_input(float delta, const InputState::Analog& analog, const InputState::Buttons& buttons)
      {
         player_view_deg_y += analog.rx * -120.0f * delta;
         player_view_deg_x += analog.ry * -90.0f * delta;
         player_view_deg_x = clamp(player_view_deg_x, -80.0f, 80.0f);

         mat4 rotate_x = rotate(mat4(1.0), player_view_deg_x, vec3(1, 0, 0));
         mat4 rotate_y = rotate(mat4(1.0), player_view_deg_y, vec3(0, 1, 0));
         mat4 rotate_y_right = rotate(mat4(1.0), player_view_deg_y - 90.0f, vec3(0, 1, 0));

         player_look_dir = vec3(rotate_y * rotate_x * vec4(0, 0, -1, 1));
         vec3 right_walk_dir = vec3(rotate_y_right * vec4(0, 0, -1, 1));


         vec3 mod_speed = buttons.r ? vec3(240.0f) : vec3(120.0f);
         vec3 velocity = player_look_dir * vec3(analog.y * -0.25f) +
            right_walk_dir * vec3(analog.x * 0.25f);

         player_pos += velocity * mod_speed * delta;
         update_global_data();
      }

      void run(float delta, const InputState& input) override
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
         update_input(delta, analog, input.pressed);

         glViewport(0, 0, width, height);
         glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

         glEnable(GL_CULL_FACE);
         glEnable(GL_DEPTH_TEST);
         glDepthFunc(GL_LEQUAL);

         global_buffer.bind();
         global_fragment_buffer.bind();

         scene.render(global.vp);

         skybox.tex.bind(0);
         Sampler::bind(0, Sampler::TrilinearClamp);
         skybox.shader.use();
         skybox.arrays.bind();
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
         skybox.arrays.unbind();
         skybox.shader.unbind();

         global_buffer.unbind();
         global_fragment_buffer.unbind();
         skybox.tex.unbind(0);
         Sampler::unbind(0, Sampler::TrilinearClamp);
      }

      void get_context_version(unsigned& major, unsigned& minor) const override
      {
         major = 4;
         minor = 3;
      }

      void load() override
      {
         global_buffer.init(GL_UNIFORM_BUFFER, sizeof(global), Buffer::WriteOnly, nullptr, Shader::GlobalVertexData);
         global_fragment_buffer.init(GL_UNIFORM_BUFFER,
               sizeof(global_fragment), Buffer::WriteOnly, nullptr, Shader::GlobalFragmentData);

         player_pos = vec3(0.0f);
         player_look_dir = vec3(0, 0, -1);
         player_view_deg_x = 0.0f;
         player_view_deg_y = 0.0f;

         scene.init();

         skybox.tex.load_texture({Texture::TextureCube, {
                  "app/xpos.png",
                  "app/xneg.png",
                  "app/ypos.png",
                  "app/yneg.png",
                  "app/zpos.png",
                  "app/zneg.png",
               }, true});
         skybox.shader.init("app/shaders/skybox.vs", "app/shaders/skybox.fs");
         vector<int8_t> vertices = { -1, -1, 1, -1, -1, 1, 1, 1 };
         skybox.vertex.init(GL_ARRAY_BUFFER, 8, Buffer::None, vertices.data());
         skybox.arrays.setup({{Shader::VertexLocation, 2, GL_BYTE, GL_FALSE, 0, 0}}, { &skybox.vertex }, nullptr);
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
         mat4 view_nt;
         mat4 proj;
         mat4 inv_vp;
         mat4 inv_view;
         mat4 inv_view_nt;
         mat4 inv_proj;
         vec4 camera_pos;
         Frustum frustum;
      };

      struct GlobalFragmentData
      {
         vec4 camera_pos;
         vec4 light_pos;
         vec4 light_color;
         vec4 light_ambient;
      };

      GlobalTransforms global;
      GlobalFragmentData global_fragment;
      Buffer global_buffer;
      Buffer global_fragment_buffer;

      Scene scene;

      struct
      {
         Texture tex;
         Shader shader;
         VertexArray arrays;
         Buffer vertex;
      } skybox;
};

unique_ptr<LibretroGLApplication> libretro_gl_application_create()
{
   return Util::make_unique<BoxesApp>();
}

