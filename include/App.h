#ifndef APP_H
#define APP_H

#include "Window.h"
#include "GUI.h"
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
	void render_scene          (int width, int height, GLuint framebuffer = 0);

	// Renders the chosen image in world coordinates (0,0) - (1, y).
	void render_image          (const GL::Texture& image, int width, int height, GLuint framebuffer = 0);

	// Renders the symmetrified plane or the symmetrification frame.
	// TODO: Separate frame rendering and symmetrified plane rendering.
	void render_symmetry_frame (bool symmetrifying, int width, int height, GLuint framebuffer = 0);

	// Renders the GUI using dear ImGUI.
	void render_gui            (int width, int height, GLuint framebuffer = 0);

	// Mouse callbacks.
	void position_callback    (double, double);
	void left_click_callback  (int, int);
	void right_click_callback (int, int);
	void scroll_callback      (double, double);

	// Key callbacks.
	void print_screen  (int, int, int);

	// Utilities.
	void            load_texture    (const char*);
	Eigen::Vector2f screen_to_world (const Eigen::Vector2f&);

	// Framework objects.
	MainWindow    window_;
	double        time_;
	GUI           gui_;
	ShaderCanvas  canvas_;


	Tiling      tiling_;
	GL::Texture base_image_;
	bool        symmetrifying_;
	bool        show_settings_;

	Eigen::Vector2f screen_center_;
	Eigen::Vector2f press_position_;

	Eigen::Vector2f tiling_static_position_;
	double tiling_static_rotation_;
	Eigen::Vector2f screen_center_static_position_;

	double pixels_per_unit_;
	double zoom_factor_;
};
#endif // APP_H
