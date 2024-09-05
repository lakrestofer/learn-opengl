#include "app/core.h"

float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int run_app(GLFWwindow *w) {

  while (!glfwWindowShouldClose(w)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set the clear color
    glClear(GL_COLOR_BUFFER_BIT);         // clear buffer

    processInput(w);                      // handle any events

    glfwSwapBuffers(w);                   // swap buffer
    glfwPollEvents();                     // poll for more events
  }
  return 0;
}
