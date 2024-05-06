#include "beamlib/blib.h"
#include "fmt/base.h"
#include "glad/gl.h"
#include "mesh.h"
#include "utils/camera.h"
#include "utils/mouse.h"
#include "utils/shader.h"

int main() {
    auto *const window = Blib::CreateWindow("texture editor", 1920, 1080);
    glfwSetScrollCallback(window, [](GLFWwindow *window, double x, double y) {
        (void)window; (void)x;
        camera.offsetRadius(-y);
    });

    auto wireframe = Shader().compile("shaders/wireframe.vert.glsl", "shaders/wireframe.frag.glsl");
    MyMesh mesh;
    if (!mesh.loadFromFile("models/dragon.obj")) {
        exit(1);
    }
    mesh.setup();

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        mouse.update(window);
        camera.update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        wireframe.use();
        wireframe.setMat4("model", glm::mat4(1));
        mesh.render(wireframe);

        glfwSwapBuffers(window);
    }

    Blib::DestroyWindow(window);

    return 0;
}
