#pragma once

#include "fmt/base.h"
#include <glm/glm.hpp>

inline void dbg(glm::vec3 v) {
    fmt::println("({}, {}, {})", v.x, v.y, v.z);
}
