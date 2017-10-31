/*
 * Copyright (c) 2017 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
*/

#ifndef AERGIA_GRAPHICS_SHADER_MANAGER_H
#define AERGIA_GRAPHICS_SHADER_MANAGER_H

#include "utils/open_gl.h"

#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

namespace aergia {

/** \brief singleton
 * Manages shader programs
 */
class ShaderManager {
public:
  static ShaderManager &instance() { return instance_; }
  virtual ~ShaderManager() {}
  /** \brief Creates a shader program from shader files.
   * It expects only one file of each type with extensions .fs, .vs and .gs.
   * \return program id. **-1** if error.
   */
  int loadFromFiles(const char *fl...);
  /** \brief Creates a shader program from strings.
   * \param vs vertex shader
   * \param gs geometry shader
   * \param fs fragment shader
   * \return program id. **-1** if error.
   */
  int loadFromTexts(const char *vs, const char *gs, const char *fs);
  /** \brief use program
   * \param program **[in]** program's id
   * Activate program
   * \return **true** if success
   */
  bool useShader(GLuint program);

private:
  ShaderManager();
  ShaderManager(ShaderManager const &) = delete;
  void operator=(ShaderManager const &) = delete;

  GLuint createProgram(const GLchar *, const GLchar *);
  GLuint compile(const char *shaderSource, GLuint shaderType);
  GLuint createProgram(GLuint objects[], int size);

  static ShaderManager instance_;
};

} // aergia namespace

#endif // AERGIA_GRAPHICS_SHADER_MANAGER_H
