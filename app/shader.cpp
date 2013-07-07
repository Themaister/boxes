#include "shader.hpp"
#include "../util.hpp"
#include <vector>

using namespace std;

void Shader::log_shader(GLuint obj, const std::string& source)
{
   GLint len = 0;
   glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
   if (!len)
      return;
   vector<GLchar> buf;
   buf.resize(len);

   GLsizei dummy;
   glGetShaderInfoLog(obj, len, &dummy, buf.data());

   fprintf(stderr, "Shader error:\n%s\n=======\n%s\n=============\n", reinterpret_cast<const char*>(buf.data()), source.c_str());
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

   fprintf(stderr, "Program error:\n%s\n", reinterpret_cast<const char*>(buf.data()));
}

void Shader::compile_shader(GLuint obj, const std::string& source)
{
   const GLchar *gl_source[2] = {
      "#version 140\nlayout(std140) uniform;\n",
      source.c_str(),
   };

   glShaderSource(obj, 2, gl_source, nullptr);
   glCompileShader(obj);

   GLint status = 0;
   glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
   if (!status)
      log_shader(obj, source);
}

void Shader::compile_shaders()
{
   GLuint vert = glCreateShader(GL_VERTEX_SHADER);
   GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

   compile_shader(vert, source_vs);
   compile_shader(frag, source_fs);

   glAttachShader(prog, vert);
   glAttachShader(prog, frag);

   glBindAttribLocation(prog, VertexLocation, "aVertex");
   glBindAttribLocation(prog, TexCoordLocation, "aTexCoord");
   glBindAttribLocation(prog, NormalLocation, "aNormal");

   glLinkProgram(prog);
   log_program(prog);

   glDeleteShader(vert);
   glDeleteShader(frag);

   GLuint block = glGetUniformBlockIndex(prog, "GlobalVertexData");
   if (block != GL_INVALID_INDEX)
      glUniformBlockBinding(prog, block, 0);

   block = glGetUniformBlockIndex(prog, "GlobalFragmentData");
   if (block != GL_INVALID_INDEX)
      glUniformBlockBinding(prog, block, 1);
}

void Shader::init(const std::string& path_vs, const std::string& path_fs)
{
   source_vs = File::read_string(path_vs);
   source_fs = File::read_string(path_fs);
   if (alive)
   {
      if (prog)
         glDeleteProgram(prog);
      prog = glCreateProgram();
      compile_shaders();
   }
}

void Shader::use()
{
   glUseProgram(prog);
}

void Shader::reset()
{
   alive = true;
   prog = glCreateProgram();
   if (!source_vs.empty() && !source_fs.empty())
      compile_shaders();
}

void Shader::destroyed()
{
   alive = false;
   if (prog)
      glDeleteProgram(prog);
   prog = 0;
}

