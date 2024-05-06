#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>

class Mouse {
    double mouseDownX, mouseDownY;
    std::map<int, bool> pressed;
    glm::vec2 deltaMiddle;

public:
    void update(GLFWwindow *window);
    bool getPressed(int GLFW_MOUSE_BUTTON) { return pressed[GLFW_MOUSE_BUTTON]; }
    glm::vec2 getMiddleDelta() { return deltaMiddle; }
};

extern Mouse mouse;
