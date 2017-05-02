#ifndef GUI_H
#define GUI_H

#include "GLFWImGui.h"
#include <Eigen/Geometry>
#include <unordered_map>
#include "Rectangle.h"

class Layering;

class GUI
{
public:
	GUI (MainWindow&, Layering&);

	void render(int width, int height, GLuint framebuffer = 0);

	const Rectangle<int>& graphics_area           (void) const { return graphics_area_; }

	bool                  capturing_mouse         (void) const;
	bool                  capturing_keyboard      (void) const;

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
	bool                   export_window_visible   (void) const { return *export_window_visible_; }
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
	void set_export_window_visible   (bool i)                   { *export_window_visible_   = i; }
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
	void export_window_visible_track   (bool& t)            { export_window_visible_   = &t; }
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
	void export_window_visible_untrack   (void) { export_window_visible_   = &export_window_visible_internal_; }
	void export_width_untrack            (void) { export_width_            = &export_width_internal_; }
	void export_height_untrack           (void) { export_height_           = &export_height_internal_; }

private:
	// Helper functions.
	void draw_menu_bar              (void);
	void draw_settings_window       (void);
	void draw_usage_window          (void);
	void draw_export_window         (void);

	void draw_layer_settings        (void);
	void draw_layer_order_buttons   (size_t layer_index);
	void draw_image_order_buttons   (size_t layer_index, size_t image_index);

	// We can't alter the layering while iterating it, so we need these.
	void schedule_layer_swap        (size_t source, size_t destination);
	void schedule_image_transfer    (size_t layer, size_t source, size_t destination);
	void schedule_image_transfer    (size_t source_layer, size_t destination_layer);
	void schedule_layer_deletion    (size_t layer);
	void schedule_image_deletion    (size_t layer, size_t image);

	// Only one operation can be scheduled per frame.
	void do_scheduled_operation     (void);

	void draw_symmetry_settings     (void);
	void draw_symmetry_settings_old (void);
	void draw_symmetry_modal        (void);

	void draw_view_settings         (void);
	void draw_frame_settings        (void);
	void draw_image_settings        (void);
	void draw_image_settings_old    (void);
	void draw_export_settings       (void);

	void populate_thumbnail_map (void);

	GLFWImGui implementation_;

	MainWindow&      window_;
	Layering&        layering_;

	Eigen::Vector3f clear_color_internal_;
	Eigen::Vector2f screen_center_internal_;
	double          pixels_per_unit_internal_;
	bool            frame_visible_internal_;
	bool            result_visible_internal_;
	bool            menu_bar_visible_internal_;
	bool            settings_window_visible_internal_;
	bool            usage_window_visible_internal_;
	bool            export_window_visible_internal_;
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
	bool*            export_window_visible_;
	int*             export_width_;
	int*             export_height_;

	ExportCallback export_callback_;

	std::unordered_map<std::string, GL::Texture> thumbnail_map_;

	std::string export_base_name_;
	std::string export_filename_;

	Rectangle<int> graphics_area_;

	// These are used for positioning windows and the graphics area.
	float top_margin_;
	float bottom_margin_;
	float left_margin_;
	float right_margin_;

	float settings_width_;

	// These are used for scheduling layer operations.
	bool layer_swap_scheduled_;
	bool image_transfer_scheduled_;
	bool layer_deletion_scheduled_;
	bool image_deletion_scheduled_;
	bool transfer_between_layers_;

	size_t layer_swap_source_;
	size_t layer_swap_destination_;

	size_t image_transfer_source_;
	size_t image_transfer_destination_;
	size_t image_transfer_source_layer_;
	size_t image_transfer_destination_layer_;

	size_t deletion_layer_;
	size_t deletion_image_;

	// TODO: Get rid of this.
	float usage_window_height_;
};

#include "GUI.tcc"

#endif // GUI_H
