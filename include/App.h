#ifndef APP_H
#define APP_H

#include "Window.h"
#include "GUI.h"
#include "ShaderCanvas.h"
#include "Mesh.h"
#include "Tiling.h"
#include "GLObjects.h"

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
	void render_scene (int width, int height, GLuint framebuffer = 0);

	// Renders the chosen image in world coordinates (0,0) - (1, y).
	void render_image (const GL::Texture& image, int width, int height, GLuint framebuffer = 0);

	void render_tiling    (int width, int height, GLuint framebuffer = 0);
	void render_tiling_hq (int width, int height, GLuint framebuffer = 0);

	void render_symmetry_frame  (int width, int height, GLuint framebuffer = 0);

	// Export cropping frame.
	void render_export_frame    (int width, int height, GLuint framebuffer = 0);

	// Mouse callbacks.
	void position_callback    (double, double);
	void left_click_callback  (int, int);
	void right_click_callback (int, int);
	void scroll_callback      (double, double);

	// Key callbacks.
	void print_screen  (int, int, int);

	// Utilities.
	void            export_result   (int, int, const char*);
	Eigen::Vector2f screen_to_world (const Eigen::Vector2f&);

	// Framework objects.
	MainWindow    window_;
	double        time_;
	ShaderCanvas  canvas_;
	Tiling        tiling_;
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
	Eigen::Vector2f tiling_static_position_;
	Eigen::Vector2f base_image_static_position_;
	double          tiling_static_rotation_;
	double          base_image_static_rotation_;
};
#endif // APP_H
