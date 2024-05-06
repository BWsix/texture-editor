#include "mouse.h"
#include "fmt/base.h"

Mouse mouse;

auto buttons = {
    GLFW_MOUSE_BUTTON_RIGHT,
    GLFW_MOUSE_BUTTON_LEFT,
    GLFW_MOUSE_BUTTON_1,
    GLFW_MOUSE_BUTTON_2,
    GLFW_MOUSE_BUTTON_3,
    GLFW_MOUSE_BUTTON_4,
    GLFW_MOUSE_BUTTON_5,
    GLFW_MOUSE_BUTTON_6,
};

void Mouse::update(GLFWwindow *window) {
    const auto middleButtonState =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
    auto &middleButtonPressed = pressed[GLFW_MOUSE_BUTTON_MIDDLE];

    if (middleButtonState == GLFW_PRESS && middleButtonPressed == false) {
        middleButtonPressed = true;
        glfwGetCursorPos(window, &mouseDownX, &mouseDownY);
    }
    if (middleButtonState) {
        double mouseCurrentX, mouseCurrentY;
        glfwGetCursorPos(window, &mouseCurrentX, &mouseCurrentY);
        glfwSetCursorPos(window, mouseDownX, mouseDownY);
        deltaMiddle = {mouseCurrentX - mouseDownX, mouseCurrentY - mouseDownY};
    }
    if (middleButtonState == GLFW_RELEASE && middleButtonPressed == true) {
        middleButtonPressed = false;
        deltaMiddle = {0, 0};
    }

    for (auto BTN : buttons) {
        const auto buttonState = glfwGetMouseButton(window, BTN);
        if (buttonState == GLFW_PRESS && pressed[BTN] == false) {
            pressed[BTN] = true;
        }
        if (buttonState == GLFW_RELEASE && pressed[BTN] == true) {
            pressed[BTN] = false;
        }
    }
}
