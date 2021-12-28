#include <splash/gl/shader.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace splash
{
namespace gl
{
namespace
{
GLuint LoadShaderModule(const std::string& shaderFilepath, GLenum shaderStage)
{
  std::string code;
  {
    std::ifstream in(shaderFilepath);
    std::stringstream ss;
    ss << in.rdbuf();
    code = ss.str();
  }

  GLuint shader = glCreateShader(shaderStage);
  const char* codeString = code.c_str();
  glShaderSource(shader, 1, &codeString, NULL);
  glCompileShader(shader);

  GLint success;
  GLchar infoLog[1024];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    std::cout << "Failed to compile shader (" << shaderStage << "), error:" << std::endl
      << infoLog << std::endl;
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}
}

Shader::Shader(const std::string& dirpath, const std::string& name)
{
  const auto vertexShader = LoadShaderModule(dirpath + "\\" + name + ".vert", GL_VERTEX_SHADER);
  const auto fragmentShader = LoadShaderModule(dirpath + "\\" + name + ".frag", GL_FRAGMENT_SHADER);

  program_ = glCreateProgram();
  glAttachShader(program_, vertexShader);
  glAttachShader(program_, fragmentShader);

  glLinkProgram(program_);

  GLint success;
  GLchar infoLog[1024];
  glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(program_, 1024, NULL, infoLog);
    std::cout << "Failed to link shader program, error:" << std::endl
      << infoLog << std::endl;
    glDeleteShader(program_);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
  glDeleteProgram(program_);
}

void Shader::use()
{
  glUseProgram(program_);
}

void Shader::done()
{
  glUseProgram(0);
}

void Shader::uniform1i(const std::string& name, int v)
{
  glUniform1i(location(name), v);
}

void Shader::uniform1f(const std::string& name, float v)
{
  glUniform1f(location(name), v);
}

void Shader::uniform3f(const std::string& name, const glm::vec3& v)
{
  glUniform3fv(location(name), 1, glm::value_ptr(v));
}

void Shader::uniform4f(const std::string& name, const glm::vec4& v)
{
  glUniform4fv(location(name), 1, glm::value_ptr(v));
}

void Shader::uniformMatrix3f(const std::string& name, const glm::mat3& m)
{
  glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::uniformMatrix4f(const std::string& name, const glm::mat4& m)
{
  glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(m));
}

uint32_t Shader::location(const std::string& name)
{
  return glGetUniformLocation(program_, name.c_str());
}
}
}
