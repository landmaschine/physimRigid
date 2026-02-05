#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  bool load(const std::string& vertexPath, const std::string& fragmentPath);
  void use() const;

  void setBool(const std::string& name, bool value) const;
  void setInt(const std::string& name, int value) const;
  void setFloat(const std::string& name, float value) const;
  void setVec2(const std::string& name, const glm::vec2& value) const;
  void setVec3(const std::string& name, const glm::vec3& value) const;
  void setVec4(const std::string& name, const glm::vec4& value) const;
  void setMat4(const std::string& name, const glm::mat4& value) const;

  unsigned int getID() const { return m_id; }

private:
  unsigned int m_id = 0;

  unsigned int compileShader(unsigned int type, const std::string& source);
  std::string readFile(const std::string& path);
};
