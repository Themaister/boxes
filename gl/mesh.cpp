#include "mesh.hpp"
#include "shader.hpp"
#include <set>

using namespace std;
using namespace glm;

namespace GL
{
   template<typename T>
   inline T parse_line(const string& data);

   template<>
   inline vec2 parse_line(const string& data)
   {
      float x = 0, y = 0;
      vector<string> split = String::split(data, " ");
      if (split.size() >= 2)
      {
         x = stof(split[0]);
         y = stof(split[1]);
      }

      return vec2(x, y);
   }

   template<>
   inline vec3 parse_line(const string& data)
   {
      float x = 0, y = 0, z = 0;
      vector<string> split = String::split(data, " ");
      if (split.size() >= 3)
      {
         x = stof(split[0]);
         y = stof(split[1]);
         z = stof(split[2]);
      }
      return vec3(x, y, z);
   }

   inline size_t translate_index(int index, size_t size)
   {
      return index < 0 ? size + index + 1 : index;
   }

   static map<string, Material> parse_mtllib(const string& path)
   {
      map<string, Material> materials;

      ifstream file(asset_path(path).c_str(), ios::in);
      if (!file.is_open())
         throw runtime_error(String::cat("Failed to open mtllib: ", path));

      Material current;
      string current_mtl;

      for (string line; getline(file, line); )
      {
         line = String::strip(line);
         size_t split_point = line.find_first_of(' ');
         string type = line.substr(0, split_point);
         string data = split_point != string::npos ? line.substr(split_point + 1) : string();

         if (type == "newmtl")
         {
            if (!current_mtl.empty())
               materials[current_mtl] = current;

            current = Material();
            current_mtl = data;
         }
         else if (type == "Ka")
            current.ambient = parse_line<vec3>(data);
         else if (type == "Kd")
            current.diffuse = parse_line<vec3>(data);
         else if (type == "Ks")
            current.specular = parse_line<vec3>(data);
         else if (type == "Ns")
            current.specular_power = stof(data);
         else if (type == "map_Kd")
            current.diffuse_map = Path::join(Path::basedir(path), data);
      }

      materials[current_mtl] = current;
      return materials;
   }

   void Mesh::finalize()
   {
      arrays.clear();
      GLsizei offset = 0;
      if (has_vertex)
      {
         arrays.push_back({ Shader::VertexLocation, 3, GL_FLOAT, GL_FALSE, 0, offset});
         offset += 3 * sizeof(float);
      }

      if (has_normal)
      {
         arrays.push_back({ Shader::NormalLocation, 3, GL_FLOAT, GL_FALSE, 0, offset});
         offset += 3 * sizeof(float);
      }

      if (has_texcoord)
      {
         arrays.push_back({ Shader::TexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, offset});
         offset += 2 * sizeof(float);
      }

      for (auto& array : arrays)
         array.stride = offset;
   }

   struct Face 
   {
      int vertex = -1;
      int normal = -1;
      int texcoord = -1; 
      unsigned buffer_index = 0;

      bool operator<(const Face& other) const
      {
         if (vertex - other.vertex)
            return vertex < other.vertex;
         if (normal - other.normal)
            return normal < other.normal;
         if (texcoord - other.texcoord)
            return texcoord < other.texcoord;

         return false;
      }
   };

   static void parse_vertex(const string& vert,
         set<Face>& faces,
         Mesh& mesh,
         const vector<vec3>& vertex,
         const vector<vec3>& normal,
         const vector<vec2>& tex)
   {
      Face face;
      auto coords = String::split(vert, "/", true);

      if (coords.size() == 1) // Vertex only
      {
         size_t coord = translate_index(stoi(coords[0]), vertex.size());
         if (coord && vertex.size() >= coord)
            face.vertex = coord - 1;

         mesh.has_vertex = true;
      }
      else if (coords.size() == 2) // Vertex/Texcoord
      {
         size_t coord_vert = translate_index(stoi(coords[0]), vertex.size());
         size_t coord_tex  = translate_index(stoi(coords[1]), tex.size());

         if (coord_vert && vertex.size() >= coord_vert)
            face.vertex = coord_vert - 1;
         if (coord_tex && tex.size() >= coord_tex)
            face.texcoord = coord_tex - 1;

         mesh.has_vertex = true;
         mesh.has_texcoord = true;
      }
      else if (coords.size() == 3 && coords[1].size()) // Vertex/Texcoord/Normal
      {
         size_t coord_vert   = translate_index(stoi(coords[0]), vertex.size());
         size_t coord_tex    = translate_index(stoi(coords[1]), tex.size());
         size_t coord_normal = translate_index(stoi(coords[2]), normal.size());

         if (coord_vert && vertex.size() >= coord_vert)
            face.vertex = coord_vert - 1;
         if (coord_tex && tex.size() >= coord_tex)
            face.texcoord = coord_tex - 1;
         if (coord_normal && normal.size() >= coord_normal)
            face.normal = coord_normal - 1;

         mesh.has_vertex = true;
         mesh.has_texcoord = true;
         mesh.has_normal = true;
      }
      else if (coords.size() == 3 && !coords[1].size()) // Vertex//Normal
      {
         size_t coord_vert   = translate_index(stoi(coords[0]), vertex.size());
         size_t coord_normal = translate_index(stoi(coords[2]), normal.size());

         if (coord_vert && vertex.size() >= coord_vert)
            face.vertex = coord_vert - 1;
         if (coord_normal && normal.size() >= coord_normal)
            face.normal = coord_normal - 1;

         mesh.has_vertex = true;
         mesh.has_normal = true;
      }

      auto itr = faces.find(face);
      if (itr == end(faces))
      {
         face.buffer_index = faces.size();
         faces.insert(face);

         if (face.vertex >= 0)
            mesh.vbo.insert(end(mesh.vbo),
                  value_ptr(vertex[face.vertex]),
                  value_ptr(vertex[face.vertex]) + 3);

         if (face.normal >= 0)
            mesh.vbo.insert(end(mesh.vbo),
                  value_ptr(normal[face.normal]),
                  value_ptr(normal[face.normal]) + 3);

         if (face.texcoord >= 0)
            mesh.vbo.insert(end(mesh.vbo),
                  value_ptr(tex[face.texcoord]),
                  value_ptr(tex[face.texcoord]) + 2);

         mesh.ibo.push_back(face.buffer_index);
      }
      else
         mesh.ibo.push_back(itr->buffer_index);
   }

   static void parse_face(const string& data,
         set<Face>& faces,
         Mesh& mesh,
         const vector<vec3>& vertex,
         const vector<vec3>& normal,
         const vector<vec2>& tex)
   {
      vector<string> vertices = String::split(data, " ");
      if (vertices.size() > 3)
         vertices.resize(3);

      for (auto& vert : vertices)
         parse_vertex(vert, faces, mesh, vertex, normal, tex);
   }

   vector<Mesh> load_meshes_obj(const std::string& path)
   {
      vector<Mesh> meshes;

      vector<vec3> vertex;
      vector<vec3> normal;
      vector<vec2> tex;

      Material current_material;
      map<string, Material> materials;

      Mesh current;

      set<Face> faces;

      ifstream file(asset_path(path).c_str(), ios::in);
      if (!file.is_open())
         throw runtime_error(String::cat("Failed to open OBJ: ", path));

      for (string line; getline(file, line); )
      {
         line = String::strip(line);
         size_t split_point = line.find_first_of(' ');
         string type = line.substr(0, split_point);
         string data = split_point != string::npos ? line.substr(split_point + 1) : string();

         if (type == "v")
            vertex.push_back(parse_line<vec3>(data));
         else if (type == "vn")
            normal.push_back(parse_line<vec3>(data));
         else if (type == "vt")
            tex.push_back(parse_line<vec2>(data));
         else if (type == "f")
            parse_face(data, faces, current, vertex, normal, tex);
         else if (type == "usemtl")
         {
            if (!current.ibo.empty()) // Different texture, new mesh.
            {
               current.finalize();
               meshes.push_back(move(current));
               current = Mesh();
            }

            faces.clear();
            current.material = materials[data];
         }
         else if (type == "mtllib")
            materials = parse_mtllib(Path::join(Path::basedir(path), data));
      }

      if (!current.ibo.empty())
      {
         current.finalize();
         meshes.push_back(move(current));
      }

      return meshes;
   }
}

