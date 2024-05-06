add_subdirectory(blib)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(OpenMesh)

add_library(stb
    stb/stb_image.cpp
    stb/stb_image.h
    stb/stb_include.cpp
    stb/stb_include.h
)
target_include_directories(stb PUBLIC "stb/")
