#include "mesh.hpp"
#include "shader.hpp"

#include <scene.h>
#include <Importer.hpp>
#include <postprocess.h>

using namespace std;

namespace GL
{
   static Mesh load_mesh(const aiMesh& aimesh)
   {
      Mesh mesh;

      GLsizei vertex_offset = 0;
      GLsizei normal_offset = 0;
      GLsizei texture_offset = 0;
      GLsizei current_offset = 0;

      if (aimesh.mVertices)
      {
         vertex_offset = current_offset;
         mesh.arrays.push_back({Shader::VertexLocation,
               3, GL_FLOAT, GL_FALSE, 0, current_offset});
         current_offset += 3 * sizeof(float);
         mesh.has_vertex = true;
      }

      if (aimesh.mNormals)
      {
         normal_offset = current_offset;
         mesh.arrays.push_back({Shader::NormalLocation,
               3, GL_FLOAT, GL_FALSE, 0, current_offset});
         current_offset += 3 * sizeof(float);
         mesh.has_normal = true;
      }

      if (aimesh.mTextureCoords && aimesh.mTextureCoords[0])
      {
         texture_offset = current_offset;
         mesh.arrays.push_back({Shader::TexCoordLocation,
               2, GL_FLOAT, GL_FALSE, 0, current_offset});
         current_offset += 2 * sizeof(float);
         mesh.has_texcoord = true;
      }

      for (auto& array : mesh.arrays)
         array.stride = current_offset;

      for (unsigned i = 0; i < aimesh.mNumFaces; i++)
      {
         auto& face = aimesh.mFaces[i];
         for (unsigned j = 0; j < face.mNumIndices; j++)
            mesh.ibo.push_back(face.mIndices[j]);
      }

      vertex_offset  /= sizeof(float);
      normal_offset  /= sizeof(float);
      texture_offset /= sizeof(float);
      unsigned stride = current_offset / sizeof(float);

      mesh.vbo.resize(stride * aimesh.mNumVertices);

      for (unsigned i = 0; i < aimesh.mNumVertices; i++)
      {
         if (mesh.has_vertex)
         {
            auto& pos = aimesh.mVertices[i];
            memcpy(mesh.vbo.data() + stride * i + vertex_offset,
                  &pos, 3 * sizeof(float));
         }

         if (mesh.has_normal)
         {
            auto& normal = aimesh.mNormals[i];
            memcpy(mesh.vbo.data() + stride * i + normal_offset,
                  &normal, 3 * sizeof(float));
         }

         if (mesh.has_texcoord)
         {
            auto& tex = aimesh.mTextureCoords[0][i];
            memcpy(mesh.vbo.data() + stride * i + texture_offset,
                  &tex, 2 * sizeof(float));
         }
      }

      return mesh;
   }

   vector<Mesh> load_scene(const std::string& path)
   {
      vector<Mesh> meshes;

      Assimp::Importer importer;
      const aiScene *scene = importer.ReadFile(path.c_str(), aiProcessPreset_TargetRealtime_Fast);

      if (!scene)
         throw std::runtime_error(String::cat("Failed to load scene: ", path));

      for (unsigned i = 0; i < scene->mNumMeshes; i++)
         meshes.push_back(load_mesh(*scene->mMeshes[i]));

      return meshes;
   }
}

