// external imports
#include <math.h>
// interntal imports
#include "app/core.h"

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int runApp(GLFWwindow* w, Vertices vs, ShaderSrc vss, ShaderSrc fss) {
  GLuint VAO;        // vertex array object
  GLuint VBO;        // vertex buffer object
  GLuint EBO;        // element buffer object
  GLuint shaders[2]; // { vertex shader, fragment shader}
  GLuint sProgram;   // shader program

  // compile and link shaders
  shaders[0] = initShader(GL_VERTEX_SHADER, vss);
  shaders[1] = initShader(GL_FRAGMENT_SHADER, fss);
  if (!shaderIsValid(shaders[0]) || !shaderIsValid(shaders[1])) return -1;
  sProgram = linkShaders(shaders, 2);
  if (!shaderProgramIsValid(sProgram)) return -1;
  for (int i = 0; i < 2; i++) glDeleteShader(shaders[i]);

  // generate gl objects
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  /// setup how to interpret the buffer data
  glBindVertexArray(VAO);
  // copy vertex data from ram to vram
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vs.size, vs.vertices, GL_STATIC_DRAW);
  // copy index data from ram to vram
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, vs.iSize, vs.indices, GL_STATIC_DRAW);
  // set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  while (!glfwWindowShouldClose(w)) {

    processInput(w); // handle any events

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(sProgram); // activate the program for use
    int vertexColorLocation = glGetUniformLocation(sProgram, "ourColor");
    glUniform4f(
        vertexColorLocation,
        0.0f,
        (sin(glfwGetTime()) / 2.0f) + 0.5f,
        0.0f,
        1.0f
    );
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }
  return 0;
}
