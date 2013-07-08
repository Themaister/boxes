#ifndef SHADER_HPP__
#define SHADER_HPP__

#include "../global.hpp"
#include <vector>
#include <map>

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

      void reserve_define(const std::string& name, unsigned bits);
      void set_define(const std::string& name, unsigned value);

   private:
      std::map<unsigned, GLuint> progs;
      unsigned current_permutation = 0;

      unsigned total_bits = 0;
      struct Define
      {
         unsigned start_bit;
         unsigned bits;
         unsigned value;
         std::string name;
      };
      std::vector<Define> defines;

      std::string source_vs, source_fs;
      bool alive = false;

      unsigned compile_shaders();
      void compile_shader(GLuint obj, const std::string& source,
            const std::vector<std::string>& defines);
      void log_shader(GLuint obj, const std::vector<const GLchar*>& source);
      void log_program(GLuint obj);

      std::vector<std::string> current_defines() const;
      unsigned compute_permutation() const;

      bool active = false;
};

#endif

