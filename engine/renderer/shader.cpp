#include "renderer/shader.hpp"

#include "glad/gl.h"
#include "logger/logger.hpp"

#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

std::string Shader::readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    ERRLOG("Failed to open shader file: ", path);
    return "";
  }
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
  unsigned int shader = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    ERRLOG(typeStr, " shader compilation failed: ", infoLog);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
  std::string vertSrc = readFile(vertexPath);
  std::string fragSrc = readFile(fragmentPath);

  if (vertSrc.empty() || fragSrc.empty()) {
    return false;
  }

  unsigned int vert = compileShader(GL_VERTEX_SHADER, vertSrc);
  unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
  if (!vert || !frag) {
    if (vert) glDeleteShader(vert);
    if (frag) glDeleteShader(frag);
    return false;
  }

  m_id = glCreateProgram();
  glAttachShader(m_id, vert);
  glAttachShader(m_id, frag);
  glLinkProgram(m_id);

  int success;
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
    ERRLOG("Shader program linking failed: ", infoLog);
    glDeleteProgram(m_id);
    m_id = 0;
  }

  glDeleteShader(vert);
  glDeleteShader(frag);

  return m_id != 0;
}

Shader::~Shader() {
  if (m_id) {
    glDeleteProgram(m_id);
  }
}

void Shader::use() const {
  glUseProgram(m_id);
}

void Shader::setBool(const std::string& name, bool value) const {
  glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
  glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
  glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
  glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
  glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
  glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
  glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
