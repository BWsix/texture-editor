#include "camera.h"
#include "mouse.h"
#include "imgui.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera camera;

void Camera::update() {
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    if (mouse.getPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        yaw += sensitivity * mouse.getMiddleDelta().x;
        pitch -= sensitivity * mouse.getMiddleDelta().y;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

    if (radius < 0.1) {
        radius = 0.1;
    }
}
