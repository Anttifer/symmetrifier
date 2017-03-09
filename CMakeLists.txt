cmake_minimum_required(VERSION 2.8)
project(symmetry)

file(GLOB_RECURSE SRC_FILES src/*)
file(GLOB_RECURSE INCLUDE_FILES include/*)

include_directories(include)

find_package(Eigen3 REQUIRED)
find_package(glfw3 3.1 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(supersymmetry
	${SRC_FILES}
	${INCLUDE_FILES}
)

target_include_directories(supersymmetry
	PRIVATE
	${GLEW_INCLUDE_DIRS}
	${GLFW3_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(supersymmetry
	PRIVATE
	glfw
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)

set_property(TARGET supersymmetry PROPERTY CXX_STANDARD 14)
