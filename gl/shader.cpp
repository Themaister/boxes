#include "shader.hpp"
#include "../util.hpp"
#include <vector>

using namespace std;
using namespace Log;

namespace GL
{
   vector<Shader::Define> Shader::global_defines;
   unsigned Shader::total_global_bits;

   void Shader::log_shader(GLuint obj, const vector<const GLchar*>& source)
   {
      GLint len = 0;
      glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
      if (!len)
         return;
      vector<GLchar> buf;
      buf.resize(len);

      GLsizei dummy;
      glGetShaderInfoLog(obj, len, &dummy, buf.data());

      log("Shader error:\n%s", buf.data());
      log("=============");
      for (auto str : source)
         log("%s", str);
      log("=============");
   }

   void Shader::log_program(GLuint obj)
   {
      GLint status = 0;
      glGetProgramiv(obj, GL_LINK_STATUS, &status);
      if (status)
         return;

      GLint len = 0;
      glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
      if (!len)
         return;
      vector<GLchar> buf;
      buf.resize(len);

      GLsizei dummy;
      glGetProgramInfoLog(obj, len, &dummy, buf.data());

      log("Program error:\n%s", buf.data());
   }

   void Shader::compile_shader(GLuint obj, const string& source,
         const vector<string>& defines)
   {
      vector<const GLchar*> gl_source = {
         "#version 140\nlayout(std140) uniform;\n",
      };
      for (auto& define : defines)
         gl_source.push_back(define.c_str());
      gl_source.push_back(source.c_str());

      glShaderSource(obj, gl_source.size(), gl_source.data(), nullptr);
      glCompileShader(obj);

      GLint status = 0;
      glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
      if (!status)
         log_shader(obj, gl_source);
   }

   vector<string> Shader::current_defines() const
   {
      vector<string> ret;
      for (auto& define : defines)
         ret.push_back(String::cat("#define ",
                  define.name, " ", to_string(define.value), "\n"));
      for (auto& define : global_defines)
         ret.push_back(String::cat("#define ",
                  define.name, " ", to_string(define.value), "\n"));
      return ret;
   }

   GLuint Shader::compile_shaders()
   {
      GLuint prog = glCreateProgram();
      GLuint vert = glCreateShader(GL_VERTEX_SHADER);
      GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

      auto defines = current_defines();

      compile_shader(vert, source_vs, defines);
      compile_shader(frag, source_fs, defines);

      glAttachShader(prog, vert);
      glAttachShader(prog, frag);

      glBindAttribLocation(prog, VertexLocation, "aVertex");
      glBindAttribLocation(prog, TexCoordLocation, "aTexCoord");
      glBindAttribLocation(prog, NormalLocation, "aNormal");

      glLinkProgram(prog);
      log_program(prog);

      glDeleteShader(vert);
      glDeleteShader(frag);

      static const vector<pair<const char*, unsigned>> uniform_mapping = {
         { "GlobalVertexData", GlobalVertexData },
         { "GlobalFragmentData", GlobalFragmentData },
         { "VertexSlot1", VertexSlot1 },
         { "VertexSlot2", VertexSlot2 },
         { "VertexSlot3", VertexSlot3 },
         { "FragmentSlot1", FragmentSlot1 },
         { "FragmentSlot2", FragmentSlot2 },
         { "FragmentSlot3", FragmentSlot3 },
      };

      for (auto& map : uniform_mapping)
      {
         GLuint block = glGetUniformBlockIndex(prog, map.first);
         if (block != GL_INVALID_INDEX)
            glUniformBlockBinding(prog, block, map.second);
      }

      return prog;
   }

   void Shader::reserve_define(const string& name, unsigned define_bits)
   {
      defines.push_back({total_bits, define_bits, 0, name});
      total_bits += define_bits;
      if (total_bits > 16)
         throw std::runtime_error("16 bits of define space exceeded.");
   }

   unsigned Shader::compute_permutation() const
   {
      unsigned permute = 0;
      for (auto& define : defines)
         permute |= define.value << define.start_bit;
      for (auto& define : global_defines)
         permute |= define.value << (define.start_bit + 16);
      return permute;
   }

   void Shader::set_define(const string& name, unsigned value)
   {
      auto itr = find_if(begin(defines),
            end(defines), [&name](const Define& def) {
            return def.name == name;
            });

      if (itr != end(defines))
      {
         itr->value = value & ((1 << itr->bits) - 1);
         current_permutation = compute_permutation();
         if (active)
            use();
      }
   }

   void Shader::reserve_global_define(const string& name, unsigned define_bits)
   {
      global_defines.push_back({total_global_bits, define_bits, 0, name});
      total_global_bits += define_bits;
      if (total_global_bits > 16)
         throw std::runtime_error("16 bits of global define space exceeded.");
   }

   void Shader::set_global_define(const string& name, unsigned value)
   {
      auto itr = find_if(begin(global_defines),
            end(global_defines), [&name](const Define& def) {
            return def.name == name;
            });

      if (itr != end(global_defines))
      {
         itr->value = value & ((1 << itr->bits) - 1);
         current_permutation = compute_permutation();
         if (active)
            use();
      }
   }

   void Shader::init(const string& path_vs, const string& path_fs)
   {
      source_vs = File::read_string(path_vs);
      source_fs = File::read_string(path_fs);

      if (alive)
         for (auto& prog : progs)
            glDeleteProgram(prog.second);

      progs.clear();
   }

   void Shader::use()
   {
      GLuint prog = progs[current_permutation];
      if (!prog)
         progs[current_permutation] = prog = compile_shaders();

      glUseProgram(prog);
      active = true;
   }

   void Shader::unbind()
   {
      glUseProgram(0);
      active = false;
   }

   void Shader::reset()
   {
      alive = true;
   }

   void Shader::destroyed()
   {
      alive = false;
      for (auto& prog : progs)
         glDeleteProgram(prog.second);
      progs.clear();
   }
}

