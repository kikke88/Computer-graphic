//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define STB_IMAGE_IMPLEMENTATION 1
#define STBI_ONLY_JPEG 1
#include "stb_image.h"

int WIDTH = 512, HEIGHT = 512;

float horizontal = -1.25 * M_PI;
float vertical = -M_PI / 5;
const float delta = 0.05;
const float speed = 2.0;
glm::vec3 cameraPos = glm::vec3(-4.2, 4.0,  4.5);
glm::vec3 direction = glm::vec3(cos(vertical) * sin(horizontal), sin(vertical), cos(vertical) * cos(horizontal));
glm::vec3 right = glm::vec3(sin(horizontal - M_PI / 2.0), 0, cos(horizontal - M_PI / 2.0));
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
bool show_map = false;

void windowResize(GLFWwindow* window, int width, int height)
{
    WIDTH  = width;
    HEIGHT = height;
}

void mouseMove(GLFWwindow* window, double xpos, double ypos)
{
    vertical += 0.05f * delta * (HEIGHT / 2.0 - ypos);
    horizontal += 0.05f * delta * (WIDTH / 2.0 - xpos);
    direction = glm::vec3(cos(vertical) * sin(horizontal), sin(vertical), cos(vertical) * cos(horizontal));
    right = glm::vec3(sin(horizontal - M_PI / 2.0f), 0, cos(horizontal - M_PI / 2.0f));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos += direction * delta * speed;
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos -= direction * delta * speed;
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos -= right * delta * speed;
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos += right * delta * speed;
    }
    if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos += up * delta * speed;
    }
    if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        cameraPos -= up * delta * speed;
    }
    if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_1 && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        show_map = false;
        horizontal = -1.25 * M_PI;
        vertical = -M_PI / 5;
        cameraPos = glm::vec3(-4.2, 4.0, 4.5);
    }
    if (key == GLFW_KEY_2 && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        show_map = true;
    }
}

int initGL()
{
    int res = 0;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    return 0;
}

unsigned int TextureLoading(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, tmp;
    unsigned char *data = stbi_load(path, &width, &height, &tmp, 0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return textureID;
}

int main(int argc, char** argv)
{
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "task3", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetCursorPosCallback (window, mouseMove);
    glfwSetWindowSizeCallback(window, windowResize);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    if (initGL() != 0) {
        return -1;
    }
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR) {
        gl_error = glGetError();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::unordered_map<GLenum, std::string> shaders;

    shaders[GL_VERTEX_SHADER] = "vertex_SM.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment_SM.glsl";
    ShaderProgram program_SM(shaders);

    shaders[GL_VERTEX_SHADER] = "vertex_DEPTH.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment_DEPTH.glsl";
    ShaderProgram program_DEPTH(shaders);

    shaders[GL_VERTEX_SHADER] = "vertex_SHOW_DEPTH.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment_SHOW_DEPTH.glsl";
    ShaderProgram program_SHOW_DEPTH(shaders);

    glfwSwapInterval(1);

    float cube_vertices[] = {

         1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,
        -1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  1.0,
        -1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  0.0,
         1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,

        -1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  0.0,
         1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  1.0,
         1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
         1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
        -1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  0.0,
        -1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  0.0,

        -1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  0.0,
        -1.0, -1.0,  1.0, -1.0,  0.0,  0.0,  0.0,  1.0,
        -1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  1.0,
        -1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  1.0,
        -1.0,  1.0, -1.0, -1.0,  0.0,  0.0,  1.0,  0.0,
        -1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  0.0,

         1.0, -1.0,  1.0,  1.0,  0.0,  0.0,  0.0,  0.0,
         1.0, -1.0, -1.0,  1.0,  0.0,  0.0,  0.0,  1.0,
         1.0,  1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,
         1.0,  1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,
         1.0,  1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  0.0,
         1.0, -1.0,  1.0,  1.0,  0.0,  0.0,  0.0,  0.0,

        -1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  0.0,
         1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  1.0,
         1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  1.0,
         1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  1.0,
        -1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0,
        -1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  0.0,

         1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  0.0,
        -1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  1.0,
        -1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
         1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  0.0,
         1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  0.0
    };
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    unsigned int boxTexture = TextureLoading("../textures/box.jpg");
    glm::vec3 cubePositions[] = {
        glm::vec3(-1.3, 1.0, 0.0),
        glm::vec3(1.5, 1.0, 0.0)
    };
    glm::mat4 modelBox1, modelBox2;
    modelBox2 = glm::translate(cubePositions[0]);
    modelBox2 = glm::scale(modelBox2, glm::vec3(0.5, 1.0, 0.5));
    modelBox1 = glm::translate(cubePositions[1]);

    float plane_vertices[] {
        -2.0,  0.0,  2.0,  0.0,  1.0,  0.0,  0.0,  0.0,
         2.0,  0.0,  2.0,  0.0,  1.0,  0.0,  0.0,  2.0,
         2.0,  0.0, -2.0,  0.0,  1.0,  0.0,  2.0,  2.0,
         2.0,  0.0, -2.0,  0.0,  1.0,  0.0,  2.0,  2.0,
        -2.0,  0.0, -2.0,  0.0,  1.0,  0.0,  2.0,  0.0,
        -2.0,  0.0,  2.0,  0.0,  1.0,  0.0,  0.0,  0.0
    };
    unsigned int planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    glBindVertexArray(planeVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    unsigned int grassTexture = TextureLoading("../textures/grass.jpg");
    glm::mat4 modelPlane;
    modelPlane = glm::scale(glm::vec3(2.0));

    float cube_root = std::pow(3, 1.0 / 3.0);
    float sqrt_2 = std::sqrt(2);
    float tetrahedron_vertices[] {
         1.0      , 0.0,              0.0, 0.0, -1.0, 0.0, 0.0, 1.0,
        -1.0 / 2.0, 0.0,  cube_root / 2.0, 0.0, -1.0, 0.0, 1.0, 0.5,
        -1.0 / 2.0, 0.0, -cube_root / 2.0, 0.0, -1.0, 0.0, 0.0, 0.0,
        -1.0 / 2.0,    0.0,  cube_root / 2.0, -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0, 0.0, 1.0,
               0.0, sqrt_2,              0.0, -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0, 1.0, 0.5,
        -1.0 / 2.0,    0.0, -cube_root / 2.0, -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0, 0.0, 0.0,
               1.0,    0.0,             0.0, sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root, 0.0, 1.0,
               0.0, sqrt_2,             0.0, sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root, 1.0, 0.5,
        -1.0 / 2.0,    0.0, cube_root / 2.0, sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root, 0.0, 0.0,
        -1.0 / 2.0,    0.0, -cube_root / 2.0, sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root, 0.0, 1.0,
               0.0, sqrt_2,              0.0, sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root, 1.0, 0.5,
               1.0,    0.0,              0.0, sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root, 0.0, 0.0,

    };
    unsigned int tetrVBO, tetrVAO;
    glGenVertexArrays(1, &tetrVAO);
    glGenBuffers(1, &tetrVBO);
    glBindBuffer(GL_ARRAY_BUFFER, tetrVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertices), tetrahedron_vertices, GL_STATIC_DRAW);
    glBindVertexArray(tetrVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    unsigned int tetrTexture = TextureLoading("../textures/ball2.jpg");
    glm::vec3 tetrPosition = glm::vec3(-1.5, 0.7, 0.0);

    float quad_vertices[] = {

        -1.0, -1.0, 0.0, 0.0, 0.0,
         1.0, -1.0, 0.0, 1.0, 0.0,
        -1.0,  1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, 0.0, 1.0, 0.0,
         1.0,  1.0, 0.0, 1.0, 1.0
    };
    unsigned int quadVBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glBindVertexArray(quadVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    const int WIDTH_DEPTH = 1024, HEIGHT_DEPTH = 1024;
    unsigned int depthFBO;
    glGenFramebuffers(1, &depthFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH_DEPTH, HEIGHT_DEPTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    program_SM.StartUseShader();
    program_SM.SetUniform("diffuseTexture", 2);
    program_SM.SetUniform("shadowMap", 1);
    program_SM.StopUseShader();

    program_SHOW_DEPTH.StartUseShader();
    program_SHOW_DEPTH.SetUniform("depthMap", 0);
    program_SHOW_DEPTH.StopUseShader();

    glm::vec3 lightPos = glm::vec3(-3.0, 4.0, -1.5);

    while (!glfwWindowShouldClose(window)) {

        glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 light_P, light_V;
        glm::mat4 lightVP;
        light_P = glm::ortho(-10.0, 10.0, -10.0, 10.0, 0.1,  8.0);
        light_V = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightVP = light_P * light_V;
        program_DEPTH.StartUseShader();
            program_DEPTH.SetUniform("lightVP", lightVP);
            glViewport(0, 0, WIDTH_DEPTH, HEIGHT_DEPTH);
            glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                double cur_time = glfwGetTime();
                glBindVertexArray(cubeVAO);
                    program_DEPTH.SetUniform("model", modelBox1);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    program_DEPTH.SetUniform("model", modelBox2);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(planeVAO);
                    program_DEPTH.SetUniform("model", modelPlane);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                glBindVertexArray(tetrVAO);
                    glm::vec3 newPosition;
                    while (cur_time > 4 * M_PI) {
                        cur_time -= 4 * M_PI;
                    }
                    if (cur_time < 2 * M_PI) {
                        newPosition = tetrPosition + (float)1.3 * glm::vec3(cos(cur_time), 0, -sin(cur_time));
                    } else {
                        newPosition = tetrPosition + (float)2.5 * glm::vec3(cos(M_PI + cur_time), 0, sin(M_PI + cur_time)) + glm::vec3(3.8, 0.0, 0.0);
                    }
                    glm::mat4 model = glm::mat4(1.0);
                    model = glm::translate(model, newPosition);
                    model = glm::rotate(model, (float)M_PI, glm::vec3(0.0, 0.0, 1.0));
                    model = glm::rotate(model, (float)cur_time, glm::vec3(0.0, 1.0, 0.0));
                    model = glm::scale(model, glm::vec3(0.5));
                    program_DEPTH.SetUniform("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 12);
                glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        program_DEPTH.StopUseShader();

        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (show_map) {
             program_SHOW_DEPTH.StartUseShader();
                 glActiveTexture(GL_TEXTURE0);
                 glBindTexture(GL_TEXTURE_2D, depthMap);
                 glBindVertexArray(quadVAO);
                     glDrawArrays(GL_TRIANGLES, 0, 6);
                 glBindVertexArray(0);
             program_SHOW_DEPTH.StopUseShader();
        } else {
            program_SM.StartUseShader();
                glm::mat4 projection = glm::perspective(glm::radians(60.0), (double)WIDTH / (double)HEIGHT, 0.1, 100.0);
                glm::mat4 view = glm::lookAt(cameraPos, cameraPos + direction, up);
                program_SM.SetUniform("projection", projection);
                program_SM.SetUniform("view", view);
                program_SM.SetUniform("viewPos", cameraPos);
                program_SM.SetUniform("lightPos", lightPos);
                program_SM.SetUniform("lightVP", lightVP);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, depthMap);
                double cur_time = glfwGetTime();
                glm::mat4 model = glm::mat4(1.0);
                GL_CHECK_ERRORS;
                glBindVertexArray(cubeVAO);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, boxTexture);
                    program_SM.SetUniform("model", modelBox1);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    program_SM.SetUniform("model", modelBox2);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
                GL_CHECK_ERRORS;
                glBindVertexArray(planeVAO);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, grassTexture);
                    program_SM.SetUniform("model", modelPlane);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                GL_CHECK_ERRORS;
                glBindVertexArray(tetrVAO);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, tetrTexture);
                    glm::vec3 newPosition;
                    while (cur_time > 4 * M_PI) {
                        cur_time -= 4 * M_PI;
                    }
                    if (cur_time < 2 * M_PI) {
                        newPosition = tetrPosition + (float)1.3 * glm::vec3(cos(cur_time), 0, -sin(cur_time));
                    } else {
                        newPosition = tetrPosition + (float)2.5 * glm::vec3(cos(M_PI + cur_time), 0, sin(M_PI + cur_time)) + glm::vec3(3.8, 0.0, 0.0);
                    }
                    model = glm::mat4(1.0);
                    model = glm::translate(model, newPosition);
                    model = glm::rotate(model, (float)M_PI, glm::vec3(0.0, 0.0, 1.0));
                    model = glm::rotate(model, (float)cur_time, glm::vec3(0.0, 1.0, 0.0));
                    model = glm::scale(model, glm::vec3(0.5));
                    program_SM.SetUniform("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 12);
                glBindVertexArray(0);
            program_SM.StopUseShader();
            GL_CHECK_ERRORS;
        }
        glfwSwapBuffers(window);
    }
    GL_CHECK_ERRORS;
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &tetrVAO);
    glDeleteBuffers(1, &tetrVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glfwTerminate();
    return 0;
}
