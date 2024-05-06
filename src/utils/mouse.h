#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>

class Mouse {
    double mouseDownX, mouseDownY;
    std::map<int, bool> buttonPressed;
    std::map<int, glm::vec2> buttonDelta;

public:
    void update(GLFWwindow *window);

    bool getButtonPressed(int GLFW_MOUSE_BUTTON) { return buttonPressed[GLFW_MOUSE_BUTTON]; }
    glm::vec2 getButtonDelta(int GLFW_MOUSE_BUTTON) { return buttonDelta[GLFW_MOUSE_BUTTON]; }
};

extern Mouse mouse;
