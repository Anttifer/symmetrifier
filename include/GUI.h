#ifndef GUI_H
#define GUI_H

#include "GLFWImGui.h"
#include <Eigen/Geometry>
#include <unordered_map>
#include "Rectangle.h"
#include "GUIVariable.h"

class Layering;
class Tiling;
class LayerImage;

class GUI
{
public:
	GUI (MainWindow&, Layering&);

	void render(int width, int height, GLuint framebuffer = 0);

	const Rectangle<int>& graphics_area           (void) const { return graphics_area_; }

	bool                  capturing_mouse         (void) const;
	bool                  capturing_keyboard      (void) const;

	template <typename T>
	using MemberExportCallback = void (T::*)(int, int, const char*);
	using ExportCallback       = std::function<void(int, int, const char*)>;

	template <typename T>
	void set_export_callback (const MemberExportCallback<T>& callback, T* this_pointer);
	void set_export_callback (const ExportCallback& callback) { export_callback_ = callback; }

	void set_tiling_defaults (const Tiling&);
	void set_image_defaults  (const LayerImage&);

	// These variables are used in the GUI and can be set to track external data.
	GUIVariable<Eigen::Vector3f> clear_color_;
	GUIVariable<Eigen::Vector2f> screen_center_;
	GUIVariable<double>          pixels_per_unit_;
	GUIVariable<bool>            frame_visible_;
	GUIVariable<bool>            result_visible_;
	GUIVariable<bool>            menu_bar_visible_;
	GUIVariable<bool>            settings_window_visible_;
	GUIVariable<bool>            usage_window_visible_;
	GUIVariable<bool>            object_settings_visible_;
	GUIVariable<bool>            view_settings_visible_;
	GUIVariable<bool>            export_settings_visible_;
	GUIVariable<int>             export_width_;
	GUIVariable<int>             export_height_;

private:
	// Helper functions.
	void draw_menu_bar              (void);
	void draw_settings_window       (void);
	void draw_usage_window          (void);

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

	void draw_general_toggles         (void);
	void draw_view_settings           (void);
	void draw_current_object_settings (void);
	void draw_current_frame_settings  (void);
	void draw_current_image_settings  (void);
	void draw_export_settings         (void);

	void populate_thumbnail_map (void);

	GLFWImGui implementation_;

	MainWindow&      window_;
	Layering&        layering_;

	Eigen::Vector2f tiling_center_default_;
	double          tiling_rotation_default_;
	double          tiling_scale_default_;

	Eigen::Vector2f image_center_default_;
	double          image_rotation_default_;
	double          image_scale_default_;

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
};

#include "GUI.inl"

#endif // GUI_H
