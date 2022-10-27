cmake_minimum_required(VERSION 3.16)

project(SecondDepthShadowMapping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
cmake_policy(SET CMP0072 NEW)
set(CMAKE_BUILD_TYPE Release)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	set(CONAN_DISABLE_CHECK_COMPILER TRUE)
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()
else()
	message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

include_directories(include)

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

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

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

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)
