#include "camera.h"
#include "mouse.h"
#include "imgui.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera camera;

void Camera::update(GLFWwindow* window) {
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    bool middle_pressed = mouse.getPressed(GLFW_MOUSE_BUTTON_MIDDLE);
    bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    if (middle_pressed && !shift_pressed) {
        yaw += sensitivity * mouse.getMiddleDelta().x;
        pitch -= sensitivity * mouse.getMiddleDelta().y;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }
    if (middle_pressed && shift_pressed) {
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 right = glm::cross(getFront(), up);
        center -= 0.001f * mouse.getMiddleDelta().x * right;
        center += 0.001f * mouse.getMiddleDelta().y * up;
    }

    if (radius < 0.1) {
        radius = 0.1;
    }
}
