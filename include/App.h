#ifndef APP_H
#define APP_H

#include "Window.h"
#include "InputManager.h"
#include "ShaderCanvas.h"
#include "Mesh.h"
#include "PinwheelPlane.h"
#include "GLObjects.h"

//--------------------

class App {
public:
	App (int argc, char* argv[]);
	App (const App&) = delete;

	App& operator=(const App&) = delete;

	void loop (void);

private:
	void update_objects            (void);
	Eigen::Vector2f scale_to_world (const Eigen::Vector2f&);

	void render_wave           (int width, int height, GLuint framebuffer = 0);
	void render_pinwheel       (int width, int height, GLuint framebuffer = 0);
	void render_texture        (const GL::Texture& texture, int width, int height, GLuint framebuffer = 0);
	void render_mesh           (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
	void render_extruded_mesh  (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
	void render_on_mesh        (const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer = 0);

	// Key callbacks.
	void print_screen  (int, int, int);
	void test_mouse_callback  (double, double);
	void test_mousebutton_callback  (int, int);

	// Framework objects.
	MainWindow    window_;
	InputManager  input_manager_;

	ShaderCanvas  canvas_;
	Mesh          cube_;
	Mesh          torus_;
	PinwheelPlane plane_;

	double        pixels_per_unit_;
	double        time_;
};
#endif // APP_H
