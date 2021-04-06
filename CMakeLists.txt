cmake_minimum_required(VERSION 3.18)

project(SecondDepthShadowMapping)

set(CMAKE_BUILD_TYPE Debug)

set(SRCS
	src/stb_image.cpp
	src/main.cpp
	src/window.cpp
	src/shader_light.cpp
	src/camera.cpp
	src/mesh.cpp
	src/object.cpp
	src/grid_axis.cpp
	src/scene.cpp
	src/framebuffer.cpp
	src/app.cpp
	src/graphics.cpp
	src/imgui.cpp
	src/imgui_draw.cpp
	src/imgui_tables.cpp
	src/imgui_widgets.cpp
	src/imgui_impl_sdl.cpp
	src/imgui_impl_opengl3.cpp)

set(HEADERS
	include/stb_image.h
	include/color.hpp
	include/window.hpp
	include/shader_light.hpp
	include/camera.hpp
	include/mesh.hpp
	include/object.hpp
	include/grid_axis.hpp
	include/scene.hpp
	include/framebuffer.hpp
	include/app.hpp
	include/graphics.hpp
	include/rapidxml.hpp
	include/imgui.h
	include/imconfig.h
	include/imgui_internal.h
	include/imstb_textedit.h
	include/imstb_rectpack.h
	include/imstb_truetype.h
	include/imgui_impl_sdl.h
	include/imgui_impl_opengl3.h)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

cmake_policy(SET CMP0072 NEW)

find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
else()
	message(FATAL_ERROR "sdl2 not found")
endif()

find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
else()
	message(FATAL_ERROR "glew not found")
endif()

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY})
else()
	message(FATAL_ERROR "OpenGL not found")
endif()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	target_link_libraries(${PROJECT_NAME} ${OpenMP_LD_FLAGS})
	target_link_libraries(${PROJECT_NAME} -fopenmp)
else()
	message(FATAL_ERROR "OpenMP not found")
endif()

find_package(ASSIMP REQUIRED)
if(ASSIMP_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
else()
	message(FATAL_ERROR "Assimp not found")
endif()