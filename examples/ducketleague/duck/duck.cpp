#include "duck.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Custom specialization of std::hash injected in namespace std
namespace std {
  template <>
  struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
      std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
      return h1;
    }
  };
}  // namespace std

void Duck::initializeGL(GLuint program) {
  position = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
  position = glm::translate(position, glm::vec3(-5.0f, -0.0f, 0.0f));
  position = glm::rotate(position, glm::radians(90.0f), glm::vec3(-1.0f, 0, 0));
  position = glm::rotate(position, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  m_program = program;

  // Delete previous buffers
  glDeleteBuffers(1, &m_EBO);
  glDeleteBuffers(1, &m_VBO);

  // Generate VBO
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
               m_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(),
               m_indices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  /*
    VAO
  */

  // Release previous VAO
  glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  GLint positionAttribute{glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), nullptr);
  }

    GLint normalAttribute{glGetAttribLocation(program, "inNormal")};
  if (normalAttribute >= 0) {
    glEnableVertexAttribArray(normalAttribute);
    GLsizei offset{sizeof(glm::vec3)};
    glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  GLint texCoordAttribute{glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    glEnableVertexAttribArray(texCoordAttribute);
    GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3)};
    glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  GLint tangentCoordAttribute{glGetAttribLocation(program, "inTangent")};
  if (tangentCoordAttribute >= 0) {
    glEnableVertexAttribArray(tangentCoordAttribute);
    GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)};
    glVertexAttribPointer(tangentCoordAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  // End of binding
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Duck::update(Ball* ball) {
  auto ballPosition { glm::vec3(ball->x(), ball->y(), ball->z()) };
  auto carPosition { glm::vec3(x(), y(), z()) };

  auto normal { glm::normalize(ballPosition - carPosition) };
  float tmp { ball->radius + radius - glm::length(ballPosition - carPosition) / 2 };

  auto distance { glm::distance(ballPosition, carPosition) };

  // If it collided
  if (distance < ball->radius + radius) {
    ball->direction = normal * speed * 2.0f;
    ball->direction.y = abs(ball->direction.y) + speed * 2.0f;

    // Launch ball in the direction of the normal, plus some vertical motion
    glm::translate(ball->position, tmp*normal);
  } 
}

void Duck::paintGL() {
  glUseProgram(m_program);

  // Get location of uniform variables
  GLint modelMatrixLoc{ glGetUniformLocation(m_program, "modelMatrix") };
  GLint lightDirLoc{ glGetUniformLocation(m_program, "lightDirWorldSpace") };
  GLint shininessLoc{ glGetUniformLocation(m_program, "shininess") };
  GLint IaLoc{ glGetUniformLocation(m_program, "Ia") };
  GLint IdLoc{ glGetUniformLocation(m_program, "Id") };
  GLint IsLoc{ glGetUniformLocation(m_program, "Is") };
  GLint KaLoc{ glGetUniformLocation(m_program, "Ka") };
  GLint KdLoc{ glGetUniformLocation(m_program, "Kd") };
  GLint KsLoc{ glGetUniformLocation(m_program, "Ks") };
  GLint diffuseTexLoc{ glGetUniformLocation(m_program, "diffuseTex") };
  GLint normalTexLoc{ glGetUniformLocation(m_program, "normalTex") };

  glUniform1i(diffuseTexLoc, 0);
  glUniform1i(normalTexLoc, 1);

  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::mat4 m_rotation{1.0f};
  auto lightDirRotated{m_rotation * m_lightDir};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};

  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &position[0][0]);

  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  glBindVertexArray(m_VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_normalTexture);

  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);

  glUseProgram(0);
}

void Duck::loadModelFromFile(std::string_view path) {
  auto basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath;  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{ reader.GetAttrib() };
  const auto& shapes{ reader.GetShapes() };
  const auto& materials{ reader.GetMaterials() };

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;
  m_hasTexCoords = false;

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over indices
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      tinyobj::index_t index{shape.mesh.indices.at(offset)};

      // Vertex position
      std::size_t startIndex{static_cast<size_t>(3 * index.vertex_index)};
      float vx{attrib.vertices.at(startIndex + 0)};
      float vy{attrib.vertices.at(startIndex + 1)};
      float vz{attrib.vertices.at(startIndex + 2)};

      // Vertex normal
      float nx{};
      float ny{};
      float nz{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        startIndex = 3 * index.normal_index;
        nx = attrib.normals.at(startIndex + 0);
        ny = attrib.normals.at(startIndex + 1);
        nz = attrib.normals.at(startIndex + 2);
      }

      // Vertex texture coordinates
      float tu{};
      float tv{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        startIndex = 2 * index.texcoord_index;
        tu = attrib.texcoords.at(startIndex + 0);
        tv = attrib.texcoords.at(startIndex + 1);
      }

      Vertex vertex{};
      vertex.position = {vx, vy, vz};
      vertex.normal = {nx, ny, nz};
      vertex.texCoord = {tu, tv};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  // Use properties of first material, if available
  if (!materials.empty()) {
    const auto& mat{materials.at(0)};  // First material
    m_Ka = glm::vec4(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1);
    m_Kd = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1);
    m_Ks = glm::vec4(mat.specular[0], mat.specular[1], mat.specular[2], 1);
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);

    if (!mat.normal_texname.empty()) {
      loadNormalTexture(basePath + mat.normal_texname);
    } else if (!mat.bump_texname.empty()) {
      loadNormalTexture(basePath + mat.bump_texname);
    }
  } else {
    // Default values
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 25.0f;
  }

  this->standardize();
  

  if (!m_hasNormals) {
    computeNormals();
  }

  if (m_hasTexCoords) {
    computeTangents();
  }
}

float Duck::x() {
  return position[3][0];
}

float Duck::y() {
  return position[3][1];
}

float Duck::z() {
  return position[3][2];
}

void Duck::x(float value) {
  position[3][0] = value;
}

void Duck::y(float value) {
  position[3][1] = value;
}

void Duck::z(float value) {
  position[3][2] = value;
}

void Duck::standardize() {
  // Center to origin and normalize largest bound to [-1, 1]

  // Get bounds
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }

  // Center and scale
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;

    // Makes y centered relative to the field
    vertex.position.z += 0.2f;
  }
}

void Duck::computeNormals() {
  // Clear previous vertex normals
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::zero<glm::vec3>();
  }

  // Compute face normals
  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    // Get face vertices
    Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
    Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
    Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

    // Compute normal
    const auto edge1{b.position - a.position};
    const auto edge2{c.position - b.position};
    glm::vec3 normal{glm::cross(edge1, edge2)};

    // Accumulate on vertices
    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  // Normalize
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Duck::computeTangents() {
  // Reserve space for bitangents
  std::vector<glm::vec3> bitangents(m_vertices.size(), glm::vec3(0));

  // Compute face tangents and bitangents
  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    // Get face indices
    const auto i1{m_indices.at(offset + 0)};
    const auto i2{m_indices.at(offset + 1)};
    const auto i3{m_indices.at(offset + 2)};

    // Get face vertices
    Vertex& v1{m_vertices.at(i1)};
    Vertex& v2{m_vertices.at(i2)};
    Vertex& v3{m_vertices.at(i3)};

    const auto e1{v2.position - v1.position};
    const auto e2{v3.position - v1.position};
    const auto delta1{v2.texCoord - v1.texCoord};
    const auto delta2{v3.texCoord - v1.texCoord};

    // clang-format off
    glm::mat2 M;
    M[0][0] =  delta2.t;
    M[0][1] = -delta1.t;
    M[1][0] = -delta2.s;
    M[1][1] =  delta1.s;
    M *= (1.0f / (delta1.s * delta2.t - delta2.s * delta1.t));

    auto tangent{glm::vec4(M[0][0] * e1.x + M[0][1] * e2.x,
                           M[0][0] * e1.y + M[0][1] * e2.y,
                           M[0][0] * e1.z + M[0][1] * e2.z, 0.0f)};

    auto bitangent{glm::vec3(M[1][0] * e1.x + M[1][1] * e2.x,
                             M[1][0] * e1.y + M[1][1] * e2.y,
                             M[1][0] * e1.z + M[1][1] * e2.z)};
    // clang-format on

    // Accumulate on vertices
    v1.tangent += tangent;
    v2.tangent += tangent;
    v3.tangent += tangent;

    bitangents.at(i1) += bitangent;
    bitangents.at(i2) += bitangent;
    bitangents.at(i3) += bitangent;
  }

  for (auto&& [i, vertex] : iter::enumerate(m_vertices)) {
    const auto& n{vertex.normal};
    const auto& t{glm::vec3(vertex.tangent)};

    // Orthogonalize t with respect to n
    const auto tangent = t - n * glm::dot(n, t);
    vertex.tangent = glm::vec4(glm::normalize(tangent), 0);

    // Compute handedness of re-orthogonalized basis
    const auto b{glm::cross(n, t)};
    const auto handedness{glm::dot(b, bitangents.at(i))};
    vertex.tangent.w = (handedness < 0.0f) ? -1.0f : 1.0f;
  }
}

void Duck::loadDiffuseTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;

  glDeleteTextures(1, &m_diffuseTexture);
  m_diffuseTexture = abcg::opengl::loadTexture(path);
}

void Duck::loadNormalTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;

  glDeleteTextures(1, &m_normalTexture);
  m_normalTexture = abcg::opengl::loadTexture(path);
}


void Duck::move(float acceleration, float panSpeed, float deltaTime) {
  auto rotation { glm::radians(-60 * panSpeed * deltaTime) };

  position = glm::rotate(position, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  lookDirection = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0.0f, 1.0f)) * glm::vec4(lookDirection, 1.0f);

  speed += acceleration * deltaTime * 10;

  speed = std::max(std::min(speed, 10.0f), -10.0f);

  position = glm::translate(position, glm::vec3(0.0f, -1.0f, 0.0f) * speed * deltaTime);

  // Check borders
  if (x() < -10.5f) {
    x(-10.5f);
  }

  if (x() > 10.0f) {
    x(10.0f);
  }

  if (z() < -5.1f) {
    z(-5.1f);
  }

  if (z() > 5.1f) {
    z(5.1f);
  }

  // If not accelerating, then starts friction
  if (acceleration == 0.0f) {
    speed *= 1 - 0.8 * deltaTime;
    if (speed <= 0.1f) {
      speed = 0;
    }
  }
}