cmake_minimum_required(VERSION 3.13)
project(symmetry)

file(GLOB_RECURSE SRC_FILES src/*)
file(GLOB_RECURSE INCLUDE_FILES include/*)

set(IMGUI_SRC_FILES
	external/imgui/imgui.cpp
	external/imgui/imgui_draw.cpp
	external/imgui/imgui_tables.cpp
	external/imgui/imgui_widgets.cpp
	external/imgui/imgui_demo.cpp
)

set(GLEW_USE_STATIC_LIBS ON)

find_package(Eigen3 REQUIRED)
find_package(GLEW REQUIRED)

option(GLFW_BUILD_DOCS "Build GLFW docs" OFF)
option(GLFW_BUILD_TESTS "Build GLFW tests" OFF)
option(GLFW_BUILD_EXAMPLES "Build GLFW examples" OFF)

add_subdirectory(external/glfw)

add_library(imgui STATIC
	${IMGUI_SRC_FILES}
)

target_include_directories(imgui
	PRIVATE
	external/imgui
)

target_include_directories(imgui
	SYSTEM INTERFACE
	external/imgui
)

add_executable(symmetrifier
	${SRC_FILES}
	${INCLUDE_FILES}
)

target_include_directories(symmetrifier
	PRIVATE
	include
)

target_include_directories(symmetrifier
	SYSTEM PRIVATE
	external/stb
	${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(symmetrifier
	PRIVATE
	imgui
	glfw
	GLEW::GLEW
)

set_property(TARGET symmetrifier PROPERTY CXX_STANDARD 17)

if(MSVC)
	set_property(TARGET symmetrifier PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT symmetrifier)
	target_compile_definitions(symmetrifier PRIVATE _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
	target_compile_options(symmetrifier
		PRIVATE
		$<$<CONFIG:DEBUG>:
			/Od /W4>
	)
	target_link_options(symmetrifier
		PRIVATE
		$<$<CONFIG:RELEASE>:
			/SUBSYSTEM:windows /ENTRY:mainCRTStartup>
	)
else()
	target_compile_options(symmetrifier
		PRIVATE
		$<$<CONFIG:DEBUG>:
			-O0 -Wall -Wextra -Wshadow -pedantic -Wno-address>
	)
endif()

add_custom_command(TARGET symmetrifier POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy $<TARGET_FILE:symmetrifier> ${CMAKE_CURRENT_SOURCE_DIR}
)
