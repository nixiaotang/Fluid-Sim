#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "util.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


// window settings
const unsigned int SCREEN_WIDTH = 600;
const unsigned int SCREEN_HEIGHT = 600;

// key settings
bool spacePressed = false;
bool showColour = false;

// mouse state
double mouseX = 0.0, mouseY = 0.0;
double prevMouseX = 0.0, prevMouseY = 0.0;
float mouseVelX = 0.0, mouseVelY = 0.0;
bool mousePressed = false;

// pressure settings
const int PRESSURE_ITERATIONS = 20;


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


    // ---- GET FRAMEBUFFER SIZE (important for Retina/HiDPI displays) ------
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Calculate scale factor between window and framebuffer
    float pixelRatio = (float)fbWidth / SCREEN_WIDTH;


    // ---- SHADERS --------------------------------------
    Shader forcefieldShader("shaders/default.vert", "shaders/fluidsim/forcefield.frag");
    Shader advectionShader("shaders/default.vert", "shaders/fluidsim/advection.frag");
    Shader pressureShader("shaders/default.vert", "shaders/fluidsim/pressure.frag");
    Shader projectPressureShader("shaders/default.vert", "shaders/fluidsim/projectPressure.frag");
    Shader fluidVisShader("shaders/default.vert", "shaders/fluidsim/fluidVis.frag");


    // ---- PING-PONG FRAMEBUFFERS --------------------------------------
    // Fluid textures: xy = velocity, z = dye, w = hue (RGBA32F)
    unsigned int fluidTex[2];
    unsigned int fluidFBO[2];
    for (int i = 0; i < 2; i++) {
        fluidTex[i] = createTexture(fbWidth, fbHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        fluidFBO[i] = createFBO(fluidTex[i]);
    }

    // Pressure textures: single channel (R32F)
    unsigned int pressureTex[2];
    unsigned int pressureFBO[2];
    for (int i = 0; i < 2; i++) {
        pressureTex[i] = createTexture(fbWidth, fbHeight, GL_R32F, GL_RED, GL_FLOAT);
        pressureFBO[i] = createFBO(pressureTex[i]);
    }

    // Ping-pong indices
    int readIdx = 0;
    int writeIdx = 1;
    int pReadIdx = 0;
    int pWriteIdx = 1;


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
    float prevTime = 0.0f;
    float DT = 1.0f / 60.0f;  // initial value, updated each frame

    while (!glfwWindowShouldClose(window)) {

        // Calculate delta time
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - prevTime;
        prevTime = currentTime;
        DT = std::min(deltaTime, 1.0f / 30.0f);     // Clamp DT to prevent instability (e.g., when window is minimized)

        // input
        processInput(window);

        glBindVertexArray(VAO);
        glViewport(0, 0, fbWidth, fbHeight);


        // ---- FORCEFIELD --------------------------------------
        // (add forces from mouse input)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidTex[readIdx]);
        glBindFramebuffer(GL_FRAMEBUFFER, fluidFBO[writeIdx]);

        forcefieldShader.use();
        forcefieldShader.setInt("uFluid", 0);
        forcefieldShader.setVec3("iResolution", fbWidth, fbHeight, 1.0f);
        forcefieldShader.setFloat("iTime", currentTime);
        forcefieldShader.setVec2("iMouse", mouseX * pixelRatio, mouseY * pixelRatio);
        forcefieldShader.setVec2("iMouseVel", mouseVelX * pixelRatio, mouseVelY * pixelRatio);
        forcefieldShader.setFloat("DT", DT);
        forcefieldShader.setFloat("InputRadius", 0.02f);
        forcefieldShader.setFloat("InputStrength", 1000.0f);
        forcefieldShader.setFloat("DyeStrength", 20.0f);
        forcefieldShader.setFloat("VelocityDecay", 0.001);
        forcefieldShader.setFloat("DyeDecay", 0.001);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        std::swap(readIdx, writeIdx);


        // ---- ADVECTION --------------------------------------
        // (move quantities along velocity field)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidTex[readIdx]);
        glBindFramebuffer(GL_FRAMEBUFFER, fluidFBO[writeIdx]);

        advectionShader.use();
        advectionShader.setInt("uFluid", 0);
        advectionShader.setVec3("iResolution", fbWidth, fbHeight, 1.0f);
        advectionShader.setFloat("DT", DT);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        std::swap(readIdx, writeIdx);


        // ---- PRESSURE SOLVE --------------------------------------
        // (Jacobi iterations)
        // uFluid stays bound to velocity texture throughout pressure iterations
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidTex[readIdx]);
        
        for (int i = 0; i < PRESSURE_ITERATIONS; i++) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pressureTex[pReadIdx]);
            glBindFramebuffer(GL_FRAMEBUFFER, pressureFBO[pWriteIdx]);

            pressureShader.use();
            pressureShader.setInt("uFluid", 0);
            pressureShader.setInt("pFluid", 1);
            pressureShader.setVec3("iResolution", fbWidth, fbHeight, 1.0f);
            pressureShader.setFloat("DT", DT);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            std::swap(pReadIdx, pWriteIdx);
        }


        // ---- PROJECT PRESSURE --------------------------------------
        // (subtract pressure gradient)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pressureTex[pReadIdx]);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fluidFBO[writeIdx]);
        projectPressureShader.use();
        projectPressureShader.setInt("uFluid", 0);
        projectPressureShader.setInt("pFluid", 1);
        projectPressureShader.setVec3("iResolution", fbWidth, fbHeight, 1.0f);
        projectPressureShader.setFloat("DT", DT);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        std::swap(readIdx, writeIdx);

        
        // ---- VISUALIZATION --------------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidTex[readIdx]);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);       // render to default framebuffer (screen)
        fluidVisShader.use();
        fluidVisShader.setInt("uFluid", 0);
        fluidVisShader.setVec3("iResolution", fbWidth, fbHeight, 1.0f);
        fluidVisShader.setInt("ColourType", showColour ? 1 : 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);                    // swap buffers (double buffer - separate output and rendering buffer to reduce artifacts)
        glfwPollEvents();                           // checks for keyboard input, mouse movement... etc.
    }
    
    // deallocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    for (int i = 0; i < 2; i++) {
        glDeleteFramebuffers(1, &fluidFBO[i]);
        glDeleteTextures(1, &fluidTex[i]);
        glDeleteFramebuffers(1, &pressureFBO[i]);
        glDeleteTextures(1, &pressureTex[i]);
    }

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

    // mouse input
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseY = SCREEN_HEIGHT - mouseY;  // flip Y to match OpenGL coords
    
    mousePressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    mouseVelX = mousePressed ? (float)(mouseX - prevMouseX) : 0.0f;
    mouseVelY = mousePressed ? (float)(mouseY - prevMouseY) : 0.0f;

    // toggle lighting when SPACE is pressed
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
        spacePressed = true;
        showColour = !showColour;
    }
    
    // reset flag when key is released
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && spacePressed) spacePressed = false;
}
