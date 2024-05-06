add_executable(editor
    utils/camera.cpp
    utils/camera.h
    utils/includes.h
    utils/mouse.cpp
    utils/mouse.h
    utils/shader.cpp
    utils/shader.h
    utils/utils.cpp
    utils/utils.h

    main.cpp
    mesh.cpp
    mesh.h
)

target_link_libraries(editor PRIVATE beamlib glm stb OpenMeshCore OpenMeshTools fmt)

if(UNIX)
    target_compile_options(editor PRIVATE "-Wall" "-Wextra" "-ggdb")
endif()
