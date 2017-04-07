#ifndef GUI_H
#define GUI_H

#include "GLFWImGui.h"
#include <Eigen/Geometry>

class Tiling;

class GUI
{
public:
	GUI (MainWindow&, Tiling&);
	GUI (const GUI&) = delete;

	GUI& operator= (const GUI&) = delete;

	void render(int width, int height, GLuint framebuffer = 0);

	// Getters for the values held by the GUI at the moment.
	// These are either the internal variables or the variables the GUI is set to track.
	const Eigen::Vector3f& clear_color             (void) const { return *clear_color_; }
	const Eigen::Vector2f& screen_center           (void) const { return *screen_center_; }
	double                 pixels_per_unit         (void) const { return *pixels_per_unit_; }
	bool                   frame_visible           (void) const { return *frame_visible_; }
	bool                   result_visible          (void) const { return *result_visible_; }
	bool                   menu_bar_visible        (void) const { return *menu_bar_visible_; }
	bool                   settings_window_visible (void) const { return *settings_window_visible_; }
	bool                   usage_window_visible    (void) const { return *usage_window_visible_; }
	int                    export_width            (void) const { return *export_width_; }
	int                    export_height           (void) const { return *export_height_; }

	// TODO: Maybe allow setting only the non-tracked, internal variables in the future?
	void set_clear_color             (const Eigen::Vector3f& i) { *clear_color_             = i; }
	void set_screen_center           (const Eigen::Vector2f& i) { *screen_center_           = i; }
	void set_pixels_per_unit         (double i)                 { *pixels_per_unit_         = i; }
	void set_frame_visible           (bool i)                   { *frame_visible_           = i; }
	void set_result_visible          (bool i)                   { *result_visible_          = i; }
	void set_menu_bar_visible        (bool i)                   { *menu_bar_visible_        = i; }
	void set_settings_window_visible (bool i)                   { *settings_window_visible_ = i; }
	void set_usage_window_visible    (bool i)                   { *usage_window_visible_    = i; }
	void set_export_width            (int i)                    { *export_width_            = i; }
	void set_export_height           (int i)                    { *export_height_           = i; }

	template <typename T>
	using MemberExportCallback = void (T::*)(int, int, const char*);
	using ExportCallback       = std::function<void(int, int, const char*)>;

	template <typename T>
	void set_export_callback (const MemberExportCallback<T>& callback, T* this_pointer);
	void set_export_callback (const ExportCallback& callback) { export_callback_ = callback; }

	void clear_color_track             (Eigen::Vector3f& t) { clear_color_             = &t; }
	void screen_center_track           (Eigen::Vector2f& t) { screen_center_           = &t; }
	void pixels_per_unit_track         (double& t)          { pixels_per_unit_         = &t; }
	void frame_visible_track           (bool& t)            { frame_visible_           = &t; }
	void result_visible_track          (bool& t)            { result_visible_          = &t; }
	void menu_bar_visible_track        (bool& t)            { menu_bar_visible_        = &t; }
	void settings_window_visible_track (bool& t)            { settings_window_visible_ = &t; }
	void usage_window_visible_track    (bool& t)            { usage_window_visible_    = &t; }
	void export_width_track            (int& t)             { export_width_            = &t; }
	void export_height_track           (int& t)             { export_height_           = &t; }

	void clear_color_untrack             (void) { clear_color_             = &clear_color_internal_; }
	void screen_center_untrack           (void) { screen_center_           = &screen_center_internal_; }
	void pixels_per_unit_untrack         (void) { pixels_per_unit_         = &pixels_per_unit_internal_; }
	void frame_visible_untrack           (void) { frame_visible_           = &frame_visible_internal_; }
	void result_visible_untrack          (void) { result_visible_          = &result_visible_internal_; }
	void menu_bar_visible_untrack        (void) { menu_bar_visible_        = &menu_bar_visible_internal_; }
	void settings_window_visible_untrack (void) { settings_window_visible_ = &settings_window_visible_internal_; }
	void usage_window_visible_untrack    (void) { usage_window_visible_    = &usage_window_visible_internal_; }
	void export_width_untrack            (void) { export_width_            = &export_width_internal_; }
	void export_height_untrack           (void) { export_height_           = &export_height_internal_; }

private:
	// Helper functions.
	void draw_menu_bar          (void);
	void draw_settings_window   (void);
	void draw_usage_window      (void);

	void draw_symmetry_settings (void);
	void draw_view_settings     (void);
	void draw_frame_settings    (void);
	void draw_export_settings   (void);

	GLFWImGui   implementation_;

	MainWindow&      window_;
	Tiling&          tiling_;

	Eigen::Vector3f clear_color_internal_;
	Eigen::Vector2f screen_center_internal_;
	double          pixels_per_unit_internal_;
	bool            frame_visible_internal_;
	bool            result_visible_internal_;
	bool            menu_bar_visible_internal_;
	bool            settings_window_visible_internal_;
	bool            usage_window_visible_internal_;
	int             export_width_internal_;
	int             export_height_internal_;

	Eigen::Vector3f* clear_color_;
	Eigen::Vector2f* screen_center_;
	double*          pixels_per_unit_;
	bool*            frame_visible_;
	bool*            result_visible_;
	bool*            menu_bar_visible_;
	bool*            settings_window_visible_;
	bool*            usage_window_visible_;
	int*             export_width_;
	int*             export_height_;

	ExportCallback export_callback_;

	std::string export_base_name_;
	std::string export_filename_;

	// This is used for positioning windows
	// depending on menu bar existence and size.
	float menu_bar_height_;
};

#include "GUI.tcc"

#endif // GUI_H
