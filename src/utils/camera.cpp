#include "camera.h"
#include "mouse.h"
#include "utils.h"
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera camera;

void Camera::update() {
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    if (mouse.getButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        yaw += sensitivity * mouse.getButtonDelta(GLFW_MOUSE_BUTTON_MIDDLE).x;
        pitch -= sensitivity * mouse.getButtonDelta(GLFW_MOUSE_BUTTON_MIDDLE).y;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        radius -= movementSpeed * getDeltaTime();
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        radius += movementSpeed * getDeltaTime();
    }
    if (radius < 0.1) {
        radius = 0.1;
    }
}
