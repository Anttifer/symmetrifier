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

	// Renders the symmetrified plane.
	void render_tiling    (int width, int height, GLuint framebuffer = 0);
	void render_tiling_hq (int width, int height, GLuint framebuffer = 0);

	// Renders the symmetrification frame.
	void render_symmetry_frame  (int width, int height, GLuint framebuffer = 0);

	// Renders the export cropping frame.
	void render_export_frame    (int width, int height, GLuint framebuffer = 0);

	// Renders the GUI using dear ImGui.
	void render_gui           (int width, int height, GLuint framebuffer = 0);

	void show_symmetry_groups (void);
	void show_view_settings   (void);
	void show_frame_settings  (void);
	void show_export_settings (void);

	// Mouse callbacks.
	void position_callback    (double, double);
	void left_click_callback  (int, int);
	void right_click_callback (int, int);
	void scroll_callback      (double, double);

	// Key callbacks.
	void print_screen  (int, int, int);

	// Utilities.
	void            load_texture    (const char*);
	void            export_result   (void);
	Eigen::Vector2f screen_to_world (const Eigen::Vector2f&);

	// Framework objects.
	MainWindow    window_;
	double        time_;
	GUI           gui_;
	ShaderCanvas  canvas_;

	Tiling      tiling_;
	GL::Texture base_image_;
	bool        show_result_;
	bool        show_symmetry_frame_;
	bool        show_export_frame_;
	bool        show_settings_;

	Eigen::Vector2f screen_center_;
	Eigen::Vector2f press_position_;

	Eigen::Vector2f tiling_static_position_;
	double tiling_static_rotation_;
	Eigen::Vector2f screen_center_static_position_;

	Eigen::Vector3f clear_color_;
	double pixels_per_unit_;
	double zoom_factor_;

	int         export_width_;
	int         export_height_;
	std::string export_base_name_;
	std::string export_filename_;
};
#endif // APP_H
