#ifndef APP_H
#define APP_H

#include "Window.h"
#include "GUI.h"
#include "ShaderCanvas.h"
#include "Mesh.h"
#include "Layering.h"
#include "Tiling.h"
#include "GLObjects.h"
#include "Rectangle.h"

//--------------------

class App
{
public:
	App (int argc, char* argv[]);
	App (const App&) = delete;

	App& operator= (const App&) = delete;

	void loop (void);

private:
	// Renders everything but the GUI, layered.
	void render_layered_scene (const Rectangle<int>& viewport, GLuint framebuffer = 0);

	void render_layer        (const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer = 0);
	void render_layer_images (const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer = 0);


	// Renders everything but the GUI.
	void render_scene         (int width, int height, GLuint framebuffer = 0);

	// TODO: A rendering system (as a Renderer class perhaps)?
	void render_base_image      (const Tiling& tiling, int width, int height, GLuint framebuffer = 0);
	void render_tiling          (const Tiling& tiling, int width, int height, GLuint framebuffer = 0);
	void render_tiling_hq       (const Tiling& tiling, int width, int height, GLuint framebuffer = 0);
	void render_symmetry_frame  (const Tiling& tiling, const Rectangle<int>& viewport, GLuint framebuffer = 0);

	// Export cropping frame.
	void render_export_frame    (const Rectangle<int>& viewport, GLuint framebuffer = 0);

	// Layered mouse callbacks.
	void layered_position_callback    (double, double);
	void layered_left_click_callback  (int, int);
	void layered_right_click_callback (int, int);
	void layered_scroll_callback      (double, double);

	// Mouse callbacks.
	void mouse_position_callback (double, double);
	void mouse_button_callback   (int, int, int);
	void mouse_scroll_callback   (double, double);

	// Old mouse callbacks.
	void position_callback    (double, double);
	void left_click_callback  (int, int);
	void right_click_callback (int, int);
	void scroll_callback      (double, double);

	// Key callbacks.
	void keyboard_callback       (int, int, int, int);

	// Old key callbacks.
	void print_screen  (int, int, int);

	void path_drop_callback      (int, const char**);

	// Utilities.
	void            load_layer_image     (const char* filename);
	void            export_result        (int, int, const char*);
	Eigen::Vector2f screen_to_view       (double x, double y);
	Eigen::Vector2f view_to_world        (const Eigen::Vector2f&);
	Eigen::Vector2f screen_to_world      (double x, double y);
	Eigen::Vector2f screen_to_world_old  (const Eigen::Vector2f&);

	// Framework objects.
	MainWindow    window_;
	double        time_;
	ShaderCanvas  canvas_;
	Layering      layering_;
	Tiling&       tiling_;
	GUI           gui_;

	// Parameters and options.
	Eigen::Vector3f clear_color_;
	Eigen::Vector2f screen_center_;
	double          pixels_per_unit_;
	bool            show_symmetry_frame_;
	bool            show_result_;
	bool            show_settings_;
	bool            show_export_settings_;
	int             export_width_;
	int             export_height_;

	// These are constants in practice.
	double zoom_factor_;

	// Mouse input helper variables.
	Eigen::Vector2f press_position_;
	Eigen::Vector2f screen_center_static_position_;
	Eigen::Vector2f object_static_position_;
	Eigen::Vector2f tiling_static_position_;
	Eigen::Vector2f base_image_static_position_;
	double          object_static_rotation_;
	double          tiling_static_rotation_;
	double          base_image_static_rotation_;
};
#endif // APP_H
