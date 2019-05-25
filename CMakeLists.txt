cmake_minimum_required(VERSION 3.0)
project(glsl-editor)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})


find_package(glfw3 3.3 REQUIRED)


add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

