#include <vector>
#include <string>
#include <fstream>

//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 512, HEIGHT = 512;

using namespace LiteMath;

float3 g_camPos(0, 4, 7);
float horizontal = 0;
float vertical = - M_PI / 6;
const float delta = 0.05;
const float speed = 2.0;
float3 direction;
float3 right;
float3 up = float3(0.0, 1.0, 0.0);
int sharp_soft = 0;

void windowResize(GLFWwindow* window, int width, int height)
{
    WIDTH  = width;
    HEIGHT = height;
}

void mouseMove(GLFWwindow* window, double xpos, double ypos)
{
    vertical += 0.05f * delta * (HEIGHT / 2.0 - ypos);
    horizontal += 0.05f * delta * (WIDTH / 2.0 - xpos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    direction = float3(cos(vertical) * sin(horizontal), 0.0, cos(vertical) * cos(horizontal));
    right = float3(sin(horizontal - M_PI / 2.0f), 0, cos(horizontal - M_PI / 2.0f));
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos -= direction * delta * speed;
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos += direction * delta * speed;
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos += right * delta * speed;
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos -= right * delta * speed;
    }
    if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos += up * delta * speed;// up *
    }
    if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        g_camPos -= up * delta * speed;
    }
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
         g_camPos = float3(0.0, 4.0, 7.0);
         vertical = - M_PI / 6;
         horizontal = 0;
         sharp_soft = 0;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        sharp_soft = (sharp_soft + 1) % 2;
    }
    if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        glfwSetWindowShouldClose(window, true);
    }
}

int initGL()
{
	int res = 0;
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}
	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	return 0;
}
unsigned char* tga_image_loading(const char* file_name, int& width, int& height)
{
    std::ifstream in_file(file_name, std::ios::binary);
    long image_size;
    int color;
    char header[18];
    in_file.read(header, 18);
    width = header[13] << 8 + header[12];
    height = header[15] << 8 + header[14];
    color = header[16] >> 3;
    image_size = width * height * color;
    unsigned char* tga = new unsigned char[image_size * sizeof(unsigned char)];
    in_file.read((char *) tga, image_size);
    in_file.close();
    return tga;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height;
    for (unsigned int i = 0; i < faces.size(); ++i) {
        unsigned char *data = tga_image_loading(faces[i].c_str(), width, height);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
            delete[] data;
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            delete[] data;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
    GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "iiiii boyiiiii", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetCursorPosCallback (window, mouseMove);
    glfwSetWindowSizeCallback(window, windowResize);
	glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (initGL() != 0){
        return -1;
    }
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR){
        gl_error = glGetError();
    }
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders);                                                                    GL_CHECK_ERRORS;
    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    std::vector<std::string> cube {
        "../textures/front.tga", "../textures/back.tga",  "../textures/bottom.tga",
        "../textures/top.tga", "../textures/right.tga", "../textures/left.tga"
    };
    unsigned int cubemapTexture = loadCubemap(cube);
    {
        float quadPos[]=
        {
          -1.0f,  1.0f,
          -1.0f, -1.0f,
          1.0f,  1.0f,
          1.0f, -1.0f
        };
        GLuint vertexLocation = 0;


        glGenBuffers(1, &g_vertexBufferObject);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadPos), (GLfloat*)quadPos, GL_STATIC_DRAW);             GL_CHECK_ERRORS;

        glGenVertexArrays(1, &g_vertexArrayObject);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);                                                        GL_CHECK_ERRORS;

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);                                           GL_CHECK_ERRORS;
        glEnableVertexAttribArray(vertexLocation);                                                     GL_CHECK_ERRORS;
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);                            GL_CHECK_ERRORS;
        glBindVertexArray(0);                                                                          GL_CHECK_ERRORS;
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    }
    float cur_time;
	while (!glfwWindowShouldClose(window))
	{
        glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
        glfwPollEvents();
        program.StartUseShader();                                                                      GL_CHECK_ERRORS;
        float4x4 camRotMatrix = mul(rotate_Y_4x4(horizontal), rotate_X_4x4(vertical));
        float4x4 camTransMatrix = translate4x4(g_camPos);
        float4x4 rayMatrix = mul(camTransMatrix, camRotMatrix);
        program.SetUniform("g_rayMatrix", rayMatrix);
        program.SetUniform("g_screenWidth" , WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);
        cur_time = glfwGetTime();
        program.SetUniform("g_curTime", cur_time);
        program.SetUniform("g_SharpSoft", sharp_soft);
        glViewport(0, 0, WIDTH, HEIGHT);        GL_CHECK_ERRORS;
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);   GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBindVertexArray(g_vertexArrayObject); GL_CHECK_ERRORS;
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  GL_CHECK_ERRORS;
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);                   GL_CHECK_ERRORS;
        program.StopUseShader();
    	glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);
	glfwTerminate();
	return 0;
}
