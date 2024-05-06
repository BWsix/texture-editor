#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 3.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;

    float radius = 5.0f;

    float fovy = 45;
    float aspect = 800.0f / 600.0f;
    float near = 0.01f;
    float far = 150.0f;

    float sensitivity = 0.05;
    float movementSpeed = 10;

public:
    void setAspect(float aspect) { this->aspect = aspect; }

    void setMovementSpeed(float speed) { movementSpeed = speed; }
    glm::vec3 getFront() const {
        return glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                         sin(glm::radians(pitch)),
                         sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    }
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(center - radius * getFront(), center, {0, 1, 0});
    }
    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fovy), aspect, near, far);
    }
    void update();
};

extern Camera camera;
