#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


// window settings
const unsigned int SCREEN_WIDTH = 600;
const unsigned int SCREEN_HEIGHT = 600;

// key settings
bool spacePressed = false;
bool showLighting = false;


int main() {

    // ---- INIT WINDOW --------------------------------------

    // GLFW - init and config
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW - create window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    } 
    glfwMakeContextCurrent(window);

    // GLFW - register window resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD init, load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // ---- SHADERS --------------------------------------
    Shader raytraceShader("shaders/default.vert", "shaders/default.frag");

    // ---- SETUP VERTEX DATA --------------------------------------
    float vertices[] = {
        // positions
        1.0f, -1.0f, 0.0f,       // bottom right
        -1.0f, -1.0f, 0.0f,      // bottom left
        1.0f, 1.0f, 0.0f,        // top right
        -1.0f, 1.0f, 0.0f,     // top left
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);                          // generate unique buffer ID for VBO
    glBindVertexArray(VAO);                         // bind Vertex Array Object

    glBindBuffer(GL_ARRAY_BUFFER, VBO);             // bind buffer to `GL_ARRAY_BUFFER` target
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);                      // copies vertex data to currently bound buffer (VBO)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);                   // set vertex attribute pointers
    glEnableVertexAttribArray(0);


    // ---- RENDER LOOP --------------------------------------
    while (!glfwWindowShouldClose(window)) {

        // input
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        raytraceShader.use();

        // draw triangles
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glfwSwapBuffers(window);                    // swap buffers (double buffer - separate output and rendering buffer to reduce artifacts)
        glfwPollEvents();                           // checks for keyboard input, mouse movement... etc.
    }
    

    // deallocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // terminate window
    glfwTerminate();

    return 0;
}

// GLFW - callback function to account for window resizing
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
    // tell OpenGL the size of the rendering viewport
    glViewport(0, 0, width, height);
}

// GLFW - process input, queries GLFW to detect if certain keys are pressed/released this frame
void processInput(GLFWwindow *window) {
    // close the window if user presses "ESC" key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
