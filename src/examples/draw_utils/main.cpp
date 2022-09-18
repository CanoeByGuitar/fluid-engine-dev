// PNGs ----> GIF
// convert -delay 3 ./*.png ../output.gif

#include <glad/glad.h>
#include <glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "shader.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <cassert>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <filesystem>

#define root_path \
    "/Users/wangchenhui/github/fluid-engine-dev/src/examples/draw_utils/"
#define data_path \
    "/Users/wangchenhui/github/fluid-engine-dev/bin/hybrid_liquid_sim_output/"
#define output_path "/Users/wangchenhui/github/fluid-engine-dev/src/examples/draw_utils/output_images/"
#define particle_num 534361

namespace fs = std::__fs::filesystem;

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 300;
const unsigned int SCR_HEIGHT = 300;



float vertices[particle_num * 3];

void getData(string file_name) {
    std::ifstream infile;
    infile.open(file_name.data());  //将文件流对象与文件连接起来
    assert(infile.is_open());  //若失败,则输出错误消息,并终止程序运行

    std::string s;
    string delimiter = " ";
    int idx = 0;
    while (getline(infile, s)) {
        vector<string> vec;
        size_t pos = 0;
        string temp;
        while ((pos = s.find(' ')) != string::npos) {
            vertices[idx++] =  (2 * stof(s.substr(0, pos)) - 1) ;
            s.erase(0, pos + delimiter.length());
        }
        vertices[idx++] =  (2 * stof(s) - 1);
    }
    infile.close();  //关闭文件输入流
}

void saveImage(const char* filepath, GLFWwindow* w) {
    int width, height;
    glfwGetFramebufferSize(w, &width, &height);
    GLsizei nrChannels = 3;
    GLsizei stride = nrChannels * width;
    stride += (stride % 4) ? (4 - stride % 4) : 0;
    GLsizei bufferSize = stride * height;
    std::vector<char> buffer(bufferSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}

int main() {


    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader(root_path "shaderFile/shader.vert",
                     root_path "shaderFile/shader.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------


    glm::mat4 model;
    model =
        glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view;
    view = glm::translate(view, glm::vec3(0.0f, -0.5f, -5.0f));
    glm::mat4 projection;
    projection = glm::perspective(
        glm::radians(60.0f), (float)(SCR_WIDTH / SCR_HEIGHT), 0.1f, 100.0f);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) , vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    vector<string> fileNames;
    for (const auto & entry : fs::directory_iterator(data_path)){
        fileNames.push_back(entry.path());
    }
    sort(fileNames.begin(),fileNames.end());

    // render loop
    // -----------
    auto iter = fileNames.begin();
    int frame_cnt = 0;
    while (!glfwWindowShouldClose(window) && iter != fileNames.end()) {
        getData(*iter);
        iter++;
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render container
        ourShader.use();

        // 矩阵变换
        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
                           glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) , vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        glPointSize(3.0f);
        glDrawArrays(GL_POINTS, 0 , particle_num );

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        // save image
        char basename[256];
        snprintf(basename, sizeof(basename), "frame_%06d.png", frame_cnt);
        saveImage((string(output_path)+ string(basename)).data(), window);
        frame_cnt++;
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}