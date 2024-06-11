#include "utils.h"
#include "camera.h"
#include "mouse.h"
#include "imgui.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera camera;

void Camera::update(GLFWwindow* window) {
    if (ImGui::GetIO().WantCaptureMouseUnlessPopupClose) {
        return;
    }

    bool middle_pressed = mouse.getPressed(GLFW_MOUSE_BUTTON_MIDDLE);
    if (middle_pressed) {
        yaw += sensitivity * mouse.getMiddleDelta().x;
        pitch -= sensitivity * mouse.getMiddleDelta().y;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 frontXZ = glm::normalize(glm::vec3(getFront().x, 0, getFront().z));
    glm::vec3 right = glm::normalize(glm::cross(frontXZ, up));

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        t.Translate(movementSpeed * frontXZ * getDeltaTime());
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        t.Translate(-movementSpeed * frontXZ * getDeltaTime());
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        t.Translate(movementSpeed * right * getDeltaTime());
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        t.Translate(-movementSpeed * right * getDeltaTime());
    }
    if (ImGui::IsKeyDown(ImGuiKey_Space)) {
        t.Translate(movementSpeed * up * getDeltaTime());
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        t.Translate(-movementSpeed * up * getDeltaTime());
    }
}
