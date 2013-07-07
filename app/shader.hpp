#ifndef SHADER_HPP__
#define SHADER_HPP__

#include "../global.hpp"

class Shader : public ContextListener, public ContextResource
{
   public:
      enum AttribLocations
      {
         VertexLocation = 0,
         TexCoordLocation = 1,
         NormalLocation = 2
      };

      void init(const std::string& path_vs, const std::string& path_fs);

      void use();
      static void unbind() { glUseProgram(0); }

      void reset() override;
      void destroyed() override;

   private:
      GLuint prog = 0;
      std::string source_vs, source_fs;
      bool alive = false;

      void compile_shaders();
      void compile_shader(GLuint obj, const std::string& source);
      void log_shader(GLuint obj, const std::string& source);
      void log_program(GLuint obj);
};

#endif

