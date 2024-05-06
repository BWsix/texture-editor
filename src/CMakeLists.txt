add_executable(editor
    utils/camera.cpp
    utils/camera.h
    utils/mouse.cpp
    utils/mouse.h
    utils/utils.cpp
    utils/utils.h

    main.cpp
)

target_link_libraries(editor PRIVATE beamlib glm stb OpenMeshCore OpenMeshTools)

if(UNIX)
    target_compile_options(editor PRIVATE "-Wall" "-Wextra" "-ggdb")
endif()
