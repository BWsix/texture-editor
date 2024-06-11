add_executable(editor
    utils/camera.cpp
    utils/camera.h
    utils/transform.cpp
    utils/transform.h
    utils/includes.h
    utils/mouse.cpp
    utils/mouse.h
    utils/shader.cpp
    utils/shader.h
    utils/texture.cpp
    utils/texture.h
    utils/utils.cpp
    utils/utils.h

    main.cpp
    mesh.cpp
    mesh.h
    textureEditor.cpp
    textureEditor.h
    textures.cpp
    textures.h
)

target_link_libraries(editor PRIVATE beamlib glm stb OpenMeshCore OpenMeshTools fmt eigen nlohmann_json)

if(UNIX)
    target_compile_options(editor PRIVATE "-Wall" "-Wextra" "-ggdb" "-O0")
endif()
