#pragma once

#include "shader.h"
#include "transform.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    float yaw = -90.0f;
    float pitch = 0.0f;

    float fovy = 45;
    float aspect = 1920.0f / 1080.0f;
    float near = 0.001f;
    float far = 10.0f;

    float sensitivity = 0.05;
    float movementSpeed = 0.2;

    Transform t;

public:
    void setAspect(float aspect) { this->aspect = aspect; }

    glm::vec3 getFront() const {
        return glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                         sin(glm::radians(pitch)),
                         sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    }
    glm::vec3 getPosition() { return t.getPosition(); }
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(t.getLocalPosition(), t.getLocalPosition() + getFront(), {0, 1, 0});
    }
    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fovy), aspect, near, far);
    }
    void update(GLFWwindow* window);

    void renderCenter(Shader prog);
};

extern Camera camera;
