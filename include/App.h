#ifndef APP_H
#define APP_H

#include "Window.h"
#include "InputManager.h"
#include "ShaderCanvas.h"
#include "Mesh.h"
#include "PinwheelPlane.h"
#include "Tiling.h"
#include "GLObjects.h"

//--------------------

class App {
public:
	App (int argc, char* argv[]);
	App (const App&) = delete;

	App& operator=(const App&) = delete;

	void loop (void);

private:
	Eigen::Vector2f scale_to_world (const Eigen::Vector2f&);

	// Renders the chosen image in world coordinates (0,0) - (1, y).
	// Possibly repeated?
	void render_image          (const GL::Texture& image, int width, int height, GLuint framebuffer = 0);
	void render_symmetry_frame (bool symmetrifying, int width, int height, GLuint framebuffer = 0);

	void render_wave           (int width, int height, GLuint framebuffer = 0);
	void render_pinwheel       (int width, int height, GLuint framebuffer = 0);
	void render_texture        (const GL::Texture& texture, int width, int height, GLuint framebuffer = 0);
	void render_mesh           (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
	void render_on_mesh        (const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer = 0);

	// Key callbacks.
	void print_screen  (int, int, int);

	// Test callbacks.
	void test_update_objects_cb     (double, double);
	void test_left_click_cb         (int, int);
	void test_scroll_cb             (double, double);

	// Framework objects.
	MainWindow    window_;
	InputManager  input_manager_;

	ShaderCanvas  canvas_;
	Mesh          cube_;
	Mesh          torus_;
	PinwheelPlane plane_;
	Tiling        tiling_;

	GL::Texture debug_tex_;
	bool symmetrifying_;

	Eigen::Vector2f screen_center_;
	// TODO: Move these into InputManager?
	Eigen::Vector2f press_position_;
	Eigen::Vector2f plane_static_position_;
	Eigen::Vector2f tiling_static_position_;
	Eigen::Vector2f screen_center_static_position_;

	double pixels_per_unit_;
	double zoom_factor_;
	double time_;
};
#endif // APP_H
