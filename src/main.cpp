#include "beamlib/blib.h"
#include "fmt/base.h"
#include "glad/gl.h"
#include "imgui.h"
#include "mesh.h"
#include "screen.h"
#include "utils/camera.h"
#include "utils/mouse.h"
#include "utils/shader.h"
#include <set>

int main() {
    auto *const window = Blib::CreateWindow("texture editor", 1920, 1080);
    glfwSetScrollCallback(window, [](GLFWwindow *window, double x, double y) {
        (void)window; (void)x;
        camera.offsetRadius(-y * 0.1);
    });

    struct {
        Shader wireframe = Shader::From("shaders/wireframe.vert.glsl", "shaders/wireframe.frag.glsl");
        Shader highlight = Shader::From("shaders/wireframe.vert.glsl", "shaders/highlight.frag.glsl");
        Shader screen = Shader::From("shaders/screen.vert.glsl", "shaders/screen.frag.glsl");
    } programs;

    MyMesh mesh;
    if (!mesh.loadFromFile("models/ball.obj")) {
        exit(1);
    }
    mesh.setup();

    Screen screen;
    screen.loadResources();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Blib::BeginUI();
        mouse.update(window);
        camera.update(window);

        screen.bind();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        programs.wireframe.use();
        programs.wireframe.setMat4("model", glm::mat4(1));
        mesh.render(programs.wireframe);

        static std::set<uint> highlighted_faces;
        screen.updateIds();
        if (ImGui::IsKeyDown(ImGuiKey_MouseLeft)) {
            auto pos = ImGui::GetMousePos();
            fmt::println("{} {} -> {}", pos.y, pos.x, screen.getId(pos));
            highlighted_faces.insert(screen.getId(pos));
        }
        if (ImGui::IsKeyDown(ImGuiKey_MouseRight)) {
            highlighted_faces.clear();
        }
        programs.highlight.use();
        programs.highlight.setMat4("model", glm::mat4(1));
        programs.highlight.setVec3("color", {1.0, 0.3, 0.3});
        mesh.highlightFaces(programs.highlight, highlighted_faces);

        screen.unbind();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        screen.render(programs.screen);

        Blib::EndUI();
        glfwSwapBuffers(window);
    }

    Blib::DestroyWindow(window);

    return 0;
}
