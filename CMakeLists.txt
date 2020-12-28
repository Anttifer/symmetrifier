cmake_minimum_required(VERSION 3.0)
project(symmetry)

file(GLOB_RECURSE SRC_FILES src/*)
file(GLOB_RECURSE INCLUDE_FILES include/*)

set(IMGUI_SRC_FILES
	external/imgui/imgui.cpp
	external/imgui/imgui_draw.cpp
	external/imgui/imgui_demo.cpp
)

set(GLEW_USE_STATIC_LIBS ON)

if(NOT MSVC)
	string(APPEND CMAKE_CXX_FLAGS_DEBUG
		" -O0 -Wall -Wextra -Wshadow -pedantic"
)
endif()

find_package(Eigen3 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)

add_library(imgui STATIC
	${IMGUI_SRC_FILES}
)

target_include_directories(imgui
	PUBLIC
	external/imgui
)

add_executable(symmetrifier
	${SRC_FILES}
)

target_include_directories(symmetrifier
	PRIVATE
	include
	${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(symmetrifier
	PRIVATE
	imgui
	glfw3
	GLEW::GLEW
)

set_property(TARGET symmetrifier PROPERTY CXX_STANDARD 17)
