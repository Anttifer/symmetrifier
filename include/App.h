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
	// Renders everything but the GUI.
	void render_scene           (const Rectangle<int>& viewport, GLuint framebuffer = 0);
	void render_scene_hq        (const Rectangle<int>& viewport, GLuint framebuffer = 0);

	void render_layer           (const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer = 0);
	void render_layer_images    (const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer = 0);
	void render_symmetry_frame  (const Tiling& tiling, const Rectangle<int>& viewport, GLuint framebuffer = 0);

	void render_export_frame    (const Rectangle<int>& viewport, GLuint framebuffer = 0);

	// Input handling.
	void mouse_position_callback (double, double);
	void mouse_button_callback   (int, int, int);
	void mouse_scroll_callback   (double, double);
	void keyboard_callback       (int, int, int, int);
	void path_drop_callback      (int, const char**);

	// Utilities.
	void            load_layer_image      (const char* filename);
	void            next_layer_object     (void);
	void            previous_layer_object (void);
	void            export_result         (int, int, const char*);
	Eigen::Vector2f screen_to_view        (double x, double y);
	Eigen::Vector2f view_to_world         (const Eigen::Vector2f&);
	Eigen::Vector2f screen_to_world       (double x, double y);

	// Framework objects.
	MainWindow    window_;
	double        time_;
	ShaderCanvas  canvas_;
	Layering      layering_;
	GUI           gui_;

	// Parameters and options.
	Eigen::Vector3f clear_color_;
	Eigen::Vector2f screen_center_;
	double          pixels_per_unit_;
	double          zoom_factor_;
	bool            show_symmetry_frame_;
	bool            show_result_;
	bool            show_object_settings_;
	bool            show_view_settings_;
	bool            show_export_settings_;
	int             export_width_;
	int             export_height_;

	// Mouse input helper variables.
	Eigen::Vector2f press_position_;
	Eigen::Vector2f screen_center_static_position_;
	Eigen::Vector2f object_static_position_;
	double          object_static_rotation_;
};
#endif // APP_H
