cmake_minimum_required(VERSION 3.14)

project(renderer)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CONFIGURATION_TYPES "Release")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -GL -fp:fast")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -ffast-math")
endif()

set(HEADERS
        core/macro.h
        core/camera.h
        core/maths.h
        core/model.h
        core/pipeline.h
        core/sample.h
        core/scene.h
        core/tgaimage.h
        shader/shader.h
        platform/win32.h
        )

set(SOURCES
        core/camera.cpp
        core/maths.cpp
        core/model.cpp
        core/pipeline.cpp
        core/sample.cpp
        core/scene.cpp
        core/tgaimage.cpp
        shader/phong_shader.cpp
        shader/skybox_shader.cpp
        platform/win32.cpp
        main.cpp
        )

add_executable(renderer  ${HEADERS} ${SOURCES})

if(MSVC)
    target_compile_options(renderer PRIVATE /fp:fast)
else()
    target_compile_options(renderer PRIVATE -ffast-math)
    target_link_libraries(renderer  m)
endif()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT renderer)
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})