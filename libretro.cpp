#include "libretro.h"
#include "util.hpp"
#include <gl/global.hpp>
#include <cstring>

using namespace std;
using namespace Log;
using namespace GL;

static struct retro_hw_render_callback hw_render;
static string libretro_dir;

static unique_ptr<LibretroGLApplication> app;
static LibretroGLApplication::InputState last_input_state;

static unsigned width;
static unsigned height;

static bool use_frame_time_cb;
static float frame_delta;

void retro_init(void)
{
   if (!app)
      app = libretro_gl_application_create();
}

void retro_deinit(void)
{
   app.reset();
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned, unsigned)
{}

void retro_get_system_info(struct retro_system_info *info)
{
   retro_init();
   app->get_system_info(*info);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   app->get_system_av_info(*info);
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   bool tmp = true;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &tmp);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

static void update_variables()
{
   retro_variable var = {0};
   var.key = "heightmap_resolution";

   if (!environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) || !var.value)
      return;

   auto list = String::split(var.value, "x");
   if (list.size() != 2)
      return;

   width = stoi(list[0]);
   height = stoi(list[1]);
   log("Internal resolution: %u x %u.", width, height);

   app->viewport_changed({width, height});
}

static void frame_time_cb(retro_usec_t usec)
{
   frame_delta = usec / 1000000.0f;
}

void retro_run(void)
{
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      update_variables();

   GLuint fb = hw_render.get_current_framebuffer();
   glBindFramebuffer(GL_FRAMEBUFFER, fb);

   LibretroGLApplication::InputState state{};

   input_poll_cb();

   float factor = float(1.0f / 0x8000);

   float analog_x = factor * input_state_cb(0, RETRO_DEVICE_ANALOG,
         RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);

   float analog_y = factor * input_state_cb(0, RETRO_DEVICE_ANALOG,
         RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

   float analog_ry = factor * input_state_cb(0, RETRO_DEVICE_ANALOG,
         RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y);

   float analog_rx = factor * input_state_cb(0, RETRO_DEVICE_ANALOG,
         RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X);

   state.analog.x = analog_x;
   state.analog.y = analog_y;
   state.analog.rx = analog_rx;
   state.analog.ry = analog_ry;

   state.pressed.left  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   state.pressed.right = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
   state.pressed.up    = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   state.pressed.down  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   state.pressed.a     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
   state.pressed.b     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
   state.pressed.x     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X);
   state.pressed.y     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   state.pressed.l     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   state.pressed.r     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);

   state.triggered.left  = state.pressed.left && !last_input_state.pressed.left;
   state.triggered.right = state.pressed.right && !last_input_state.pressed.right;
   state.triggered.up    = state.pressed.up && !last_input_state.pressed.up;
   state.triggered.down  = state.pressed.down && !last_input_state.pressed.down;
   state.triggered.a     = state.pressed.a && !last_input_state.pressed.a;
   state.triggered.b     = state.pressed.b && !last_input_state.pressed.b;
   state.triggered.x     = state.pressed.x && !last_input_state.pressed.x;
   state.triggered.y     = state.pressed.y && !last_input_state.pressed.y;
   state.triggered.l     = state.pressed.l && !last_input_state.pressed.l;
   state.triggered.r     = state.pressed.r && !last_input_state.pressed.r;

   last_input_state = state;

   if (!use_frame_time_cb)
      frame_delta = 1.0f / 60.0f;

   app->run(frame_delta, state, fb);
   video_cb(RETRO_HW_FRAME_BUFFER_VALID, width, height, 0);
}

static void context_reset(void)
{
   rglgen_resolve_symbols(hw_render.get_proc_address);
   ContextManager::get().notify_reset();
}

static void context_destroy(void)
{
   ContextManager::get().notify_destroyed();
}

bool retro_load_game(const struct retro_game_info *info)
{
   last_input_state = LibretroGLApplication::InputState{};

   auto name = app->get_application_name_short();
   name += "_resolution";

   string res = "Internal resolution; ";
   auto resolutions = app->get_resolutions();
   for (auto& r : resolutions)
      res += to_string(r.width) + "x" + to_string(r.height) + "|";
   res.resize(res.size() - 1);

   retro_variable variables[] = {
      { name.c_str(), res.c_str() },
      { nullptr, nullptr },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log("XRGB8888 is not supported.");
      return false;
   }

   hw_render.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
   hw_render.context_reset = context_reset;
   hw_render.context_destroy = context_destroy;
   hw_render.bottom_left_origin = true;
   hw_render.depth = true;
   hw_render.stencil = true;
   app->get_context_version(hw_render.version_major, hw_render.version_minor);

   if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;

   const char *libretro = nullptr;
   if (!environ_cb(RETRO_ENVIRONMENT_GET_LIBRETRO_PATH, &libretro) || !libretro)
   {
      log("Can't determine libretro path.");
      return false;
   }

   app->set_dir(Path::basedir(libretro));
   log("Loaded from dir: %s.", libretro);

   app->load();
   update_variables();

   struct retro_frame_time_callback cb = { frame_time_cb, 1000000 / 60 };
   use_frame_time_cb = environ_cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &cb);

   return true;
}

void retro_unload_game(void)
{
   app->unload();
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *, size_t)
{
   return false;
}

bool retro_unserialize(const void *, size_t)
{
   return false;
}

void *retro_get_memory_data(unsigned)
{
   return nullptr;
}

size_t retro_get_memory_size(unsigned)
{
   return 0;
}

void retro_reset(void)
{}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned, bool, const char *)
{}

