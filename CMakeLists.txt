cmake_minimum_required(VERSION 2.8)
project(symmetry)

file(GLOB_RECURSE SRC_FILES src/*)
file(GLOB_RECURSE INCLUDE_FILES include/*)

include_directories(include)

find_package(Eigen3 REQUIRED)
find_package(glfw3 3.2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(symmetrifier
	${SRC_FILES}
	${INCLUDE_FILES}
)

target_include_directories(symmetrifier
	PRIVATE
	${GLEW_INCLUDE_DIRS}
	${GLFW3_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(symmetrifier
	PRIVATE
	glfw
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)

set_property(TARGET symmetrifier PROPERTY CXX_STANDARD 14)
