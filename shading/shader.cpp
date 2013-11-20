#include "shader.h"

using namespace Tempest;

Shader::~Shader() {
  }

void Shader::setUniform(const char *name, const Texture2d &t) {
  input.set(name, t);
  }

void Shader::setUniform(const char *name, const Texture3d &t) {
  input.set(name, t);
  }

void Shader::setUniform(const char *name, const Matrix4x4 &t) {
  input.set(name, t);
  }

void Shader::setUniform(const char *name, float x) {
  input.set(name, x);
  }

void Shader::setUniform(const char *name, float x, float y) {
  input.set(name, x, y);
  }

void Shader::setUniform(const char *name, float x, float y, float z) {
  input.set(name, x, y, z);
  }

void Shader::setUniform(const char *name, float x, float y, float z, float w) {
  input.set(name, x, y, z, w);
  }

void Shader::setUniform(const char *name, const float *xyzw, int l) {
  input.set(name, xyzw, l);
  }

void Shader::setUniform(const char *name, const double *xyzw, int l) {
  input.set(name, xyzw, l);
  }

const std::string &Shader::log() const {
  return logv;
  }

void Shader::clearLog() {
  logv.clear();
  }


