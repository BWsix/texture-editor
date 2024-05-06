#include "beamlib/blib.h"
#include "glad/gl.h"
#include "imgui.h"
#include "utils/camera.h"
#include "utils/mouse.h"

int main() {
    auto *const window = Blib::CreateWindow("texture editor", 1980, 1080);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        mouse.update(window);
        camera.update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Blib::BeginUI();
        ImGui::ShowDemoWindow();
        Blib::EndUI();

        glfwSwapBuffers(window);
    }

    Blib::DestroyWindow(window);

    return 0;
}
