#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <span>

struct TransformComponent {
  glm::vec2 pos;
  glm::vec2 vel;
};

struct RenderComponent {
  glm::vec3 color{1.0f, 1.0f, 1.0f};
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  bool initialized = false;
  int vertexCount = 0;
};

struct Shape {
  virtual ~Shape() = default;
  virtual std::span<const float> getVertices() const = 0;
  virtual size_t getVertexCount() const = 0;
};

struct Triangle : Shape {
  float vertices[9] = {
    -0.5f, -0.5f, 0.0f,
     0.0f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
  };

  std::span<const float> getVertices() const override { return vertices; }
  size_t getVertexCount() const override { return 3; }
};

struct Rectangle : Shape {
  float vertices[12] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
  };

  std::span<const float> getVertices() const override { return vertices; }
  size_t getVertexCount() const override { return 4; }
};

struct RigidBodyComponent {
  std::unique_ptr<Shape> shape;
};


