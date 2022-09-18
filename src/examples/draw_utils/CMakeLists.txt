# Target name
set(target draw_utils)

# Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(GLEW_H /Users/wangchenhui/brew-2.2.2/Cellar/glew/2.2.0_1/include/GL)
set(GLFW_H /Users/wangchenhui/brew-2.2.2/Cellar/glfw/3.3.7/include/GLFW)
include_directories(${GLEW_H} ${GLFW_H})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Sources
file(GLOB sources
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


# Build executable
add_executable(${target}
        ${sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/glad.c
        ${CMAKE_CURRENT_SOURCE_DIR}/shader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
        )


# Project options
set_target_properties(${target}
        PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        )

# Compile options
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(-D_USE_MATH_DEFINES) # for M_PI
endif ()

target_compile_options(${target}
        PRIVATE

        PUBLIC
        ${DEFAULT_COMPILE_OPTIONS}

        INTERFACE
        )

# Link libraries
# 添加目标链接
set(GLEW_LINK /Users/wangchenhui/brew-2.2.2/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.dylib)
set(GLFW_LINK /Users/wangchenhui/brew-2.2.2/Cellar/glfw/3.3.7/lib/libglfw.3.dylib)

target_link_libraries(${target}
        PRIVATE
        ${DEFAULT_LINKER_OPTIONS}
        jet
        ${OPENGL} ${GLEW_LINK} ${GLFW_LINK}
        )






