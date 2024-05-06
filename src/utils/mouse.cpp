#include "mouse.h"

Mouse mouse;

int CAMERA_BUTTON = GLFW_MOUSE_BUTTON_MIDDLE;

auto buttons = {
    GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_MOUSE_BUTTON_LEFT,
    GLFW_MOUSE_BUTTON_3,     GLFW_MOUSE_BUTTON_4,
};

void Mouse::update(GLFWwindow *window) {
    for (auto BTN : buttons) {
        auto buttonState = glfwGetMouseButton(window, BTN);
        if (buttonState == GLFW_PRESS && buttonPressed[BTN] == false) {
            buttonPressed[BTN] = true;
            glfwGetCursorPos(window, &mouseDownX, &mouseDownY);
        }

        if (buttonPressed[BTN]) {
            double mouseCurrentX, mouseCurrentY;
            glfwGetCursorPos(window, &mouseCurrentX, &mouseCurrentY);
            buttonDelta[BTN] = {mouseCurrentX - mouseDownX,
                mouseCurrentY - mouseDownY};

            if (BTN == CAMERA_BUTTON) {
                glfwSetCursorPos(window, mouseDownX, mouseDownY);
            }
        }

        if (buttonState == GLFW_RELEASE && buttonPressed[BTN] == true) {
            buttonPressed[BTN] = false;
            buttonDelta[BTN] = {0, 0};
        }
    }
}
