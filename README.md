### Dependencies:
These must be available and locatable by CMake:
- [GLEW](https://github.com/nigels-com/glew)
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)

The application also depends on
- [GLFW](https://www.glfw.org)
- [Dear ImGui](https://github.com/ocornut/imgui),

which are included as submodules in this repository.

### Building and running:
```bash
git clone --recursive https://github.com/Anttifer/symmetrifier.git
mkdir symmetrifier/build && cd symmetrifier/build

# Using the "Unix Makefiles" generator with a release build here,
# but you can substitute your own choices.
cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j8
../run.sh
```

### Usage preview:
![Group 3\*3 and a butterfly](usage_sample.png)
