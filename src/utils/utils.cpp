#include "imgui.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

double getTimeElapsed() { return glfwGetTime(); }
float getFrameRate() { return ImGui::GetIO().Framerate; }
float getDeltaTime() { return ImGui::GetIO().DeltaTime; }
