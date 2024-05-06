#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;

    float radius = 5.0f;

    float fovy = 45;
    float aspect = 1920.0f / 1080.0f;
    float near = 0.1f;
    float far = 150.0f;

    float sensitivity = 0.2;

public:
    void setAspect(float aspect) { this->aspect = aspect; }

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
    void offsetRadius(double offset) { radius += offset * sensitivity; }
    void update();
};

extern Camera camera;
