// external imports
#include <math.h>
// interntal imports
#include "app/core.h"

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int runApp(GLFWwindow* w, GLuint VAO, Shader shader) {

  while (!glfwWindowShouldClose(w)) {

    processInput(w); // handle any events

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader); // activate the program for use
    int vertexColorLocation = glGetUniformLocation(shader, "ourColor");
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
