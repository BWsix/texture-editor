#include "beamlib/blib.h"
#include "glad/gl.h"
#include "imgui.h"
#include "mesh.h"
#include "screen.h"
#include "textureEditor.h"
#include "utils/camera.h"
#include "utils/mouse.h"
#include "utils/shader.h"
#include "utils/texture.h"
#include "textures.h"
#include <Eigen/Dense>


TextureEditor editor;


int main() {
    auto *const window = Blib::CreateWindow("texture editor", 1920, 1080);

    glfwSetWindowSizeCallback(window, [](GLFWwindow *, int width, int height) {
        editor.main_screen.resize(width, height);
        camera.setAspect((float)width / height);
        glViewport(0, 0, width, height < 1 ? 1 : height);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double x, double y) {
        (void)window; (void)x;
        camera.offsetRadius(-y * 0.1);
    });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    editor.load("models/dragon.obj");
    editor.setup();

    textures.loadAll();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Blib::BeginUI();
        mouse.update(window);
        camera.update(window);

        // ImGui::ShowDemoWindow();
        // ImGui::ShowMetricsWindow();

        editor.renderPopupMenu();
        editor.renderMeshLayerEditor();

        editor.bindMainScreen();
        {
            editor.renderBaseModle();

            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                camera.renderCenter(editor.programs.point);
            }
            if (!ImGui::IsKeyDown(ImGuiKey_Space)) {
                editor.renderSavedMeshes(editor.programs.uv);
            }

            if (editor.configs.edit_mode) {
                editor.handleFaceSelector();
                editor.renderSelected();
            }

            if (editor.configs.highlight_hovered_face) {
                editor.highlightHovered();
            }
        }
        editor.unbindMainScreen();

        editor.renderFacePicker();
        if (editor.configs.render_live_uv_solver) {
            editor.renderLiveUVSolver();
        }

        Blib::EndUI();
        glfwSwapBuffers(window);
    }

    Blib::DestroyWindow(window);

    return 0;
}
