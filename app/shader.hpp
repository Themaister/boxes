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

      enum UniformLocation
      {
         GlobalVertexData = 0,
         GlobalFragmentData = 1,
         VertexSlot1 = 2,
         VertexSlot2 = 3,
         VertexSlot3 = 4,
         FragmentSlot1 = 5,
         FragmentSlot2 = 6,
         FragmentSlot3 = 7,
      };

      void init(const std::string& path_vs, const std::string& path_fs);

      void use();
      void unbind();

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

