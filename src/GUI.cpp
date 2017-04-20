#include "GUI.h"

#include "Window.h"
#include "Layering.h"
#include "imgui.h"

GUI::GUI(MainWindow& window, Layering& layering) :
	implementation_ (window),
	window_         (window),
	layering_       (layering),

	// Sensible defaults.
	clear_color_internal_             (0.1f, 0.1f, 0.1f),
	screen_center_internal_           (0.5f, 0.5f),
	pixels_per_unit_internal_         (500.0),
	frame_visible_internal_           (true),
	result_visible_internal_          (true),
	menu_bar_visible_internal_        (true),
	settings_window_visible_internal_ (true),
	usage_window_visible_internal_    (false),
	export_window_visible_internal_   (false),
	export_width_internal_            (1600),
	export_height_internal_           (1200),

	clear_color_             (&clear_color_internal_),
	screen_center_           (&screen_center_internal_),
	pixels_per_unit_         (&pixels_per_unit_internal_),
	frame_visible_           (&frame_visible_internal_),
	result_visible_          (&result_visible_internal_),
	menu_bar_visible_        (&menu_bar_visible_internal_),
	settings_window_visible_ (&settings_window_visible_internal_),
	usage_window_visible_    (&usage_window_visible_internal_),
	export_window_visible_   (&export_window_visible_internal_),
	export_width_            (&export_width_internal_),
	export_height_           (&export_height_internal_),

	// Lambda that accepts anything and does nothing.
	export_callback_ ([](...){}),

	export_base_name_     ("image"),
	export_filename_      (export_base_name_ + ".png")
{
	// Set default GUI font.
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("res/DroidSans.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	implementation_.create_fonts_texture();

	populate_thumbnail_map();
}

void GUI::render(int width, int height, GLuint framebuffer)
{
	implementation_.new_frame();

	// Reset per-frame data.
	menu_bar_height_     = 0.0f;
	usage_window_height_ = 0.0f;

	if (*menu_bar_visible_)
		draw_menu_bar();

	if (*settings_window_visible_)
		draw_settings_window();

	if (*usage_window_visible_)
		draw_usage_window();

	if (*export_window_visible_)
		draw_export_window();

	implementation_.render(width, height, framebuffer);
}

bool GUI::capturing_mouse(void) const
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool GUI::capturing_keyboard(void) const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

void GUI::draw_menu_bar(void)
{
	if (ImGui::BeginMainMenuBar())
	{
		menu_bar_height_ = ImGui::GetWindowSize().y;

		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Show settings", "Esc", *settings_window_visible_))
				*settings_window_visible_ ^= true;

			if (ImGui::MenuItem("Show usage", NULL, *usage_window_visible_))
				*usage_window_visible_ ^= true;

			if (ImGui::MenuItem("Show export settings", NULL, *export_window_visible_))
				*export_window_visible_ ^= true;

			if (ImGui::MenuItem("Quit", "Alt+F4"))
				glfwSetWindowShouldClose(window_, GLFW_TRUE);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void GUI::draw_settings_window(void)
{
	auto flags = 0;
	ImGui::SetNextWindowSize({335, 0}, ImGuiSetCond_Once);
	ImGui::SetNextWindowPos({0, menu_bar_height_}, ImGuiSetCond_Once);
	if (ImGui::Begin("Settings", settings_window_visible_, flags))
	{
		draw_symmetry_settings();

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		draw_view_settings();

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		draw_frame_settings();

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		draw_image_settings();

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	}
	ImGui::End();
}

void GUI::draw_usage_window(void)
{
	auto& io = ImGui::GetIO();

	auto flags = ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowSize({350, 0}, ImGuiSetCond_Appearing);
	ImGui::SetNextWindowPos({io.DisplaySize.x - 350, menu_bar_height_}, ImGuiSetCond_Appearing);
	if (ImGui::Begin("Usage", usage_window_visible_, flags))
	{
		ImGui::Bullet();
		ImGui::TextWrapped("Drag and drop the PNG image to symmetrify in this window.");
		ImGui::Bullet();
		ImGui::TextWrapped("Click and drag to move around.");
		ImGui::Bullet();
		ImGui::TextWrapped("Control + drag to move the symmetrification frame.");
		ImGui::Bullet();
		ImGui::TextWrapped("Control + right drag to rotate the symmetrification frame.");
		ImGui::Bullet();
		ImGui::TextWrapped("Scroll to zoom.");
		ImGui::Bullet();
		ImGui::TextWrapped("Control + scroll to resize the symmetrification frame.");
		ImGui::Bullet();
		ImGui::TextWrapped("Spacebar to toggle the symmetrified view.");
		ImGui::Bullet();
		ImGui::TextWrapped("Control + Spacebar to toggle the frame in the symmetrified view.");

		usage_window_height_ = ImGui::GetWindowSize().y;
	}
	ImGui::End();
}

void GUI::draw_export_window(void)
{
	auto& io = ImGui::GetIO();

	auto flags = ImGuiWindowFlags_ShowBorders;
	ImGui::SetNextWindowSize({350, 0}, ImGuiSetCond_Appearing);
	ImGui::SetNextWindowPos({io.DisplaySize.x - 350, usage_window_height_ + menu_bar_height_}, ImGuiSetCond_Appearing);
	if (ImGui::Begin("Export settings", export_window_visible_, flags))
		draw_export_settings();
	ImGui::End();
}

[[deprecated]]
void GUI::draw_symmetry_settings_old(void)
{
	auto& layer = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	ImGui::Text("Symmetry groups");
	ImGui::Separator();

	ImGui::Dummy({0, 0});                   ImGui::SameLine(95);
	ImGui::Text("No reflections");          ImGui::SameLine(215);
	ImGui::Text("Reflections");
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("No rotations");     ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("o", !strncmp(ctiling.symmetry_group(), "o", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("o");
	ImGui::SameLine(25); ImGui::Text("(p1)");
	if (ImGui::Selectable("xx", !strncmp(ctiling.symmetry_group(), "xx", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("xx");
	ImGui::SameLine(25); ImGui::Text("(pg)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("**", !strncmp(ctiling.symmetry_group(), "**", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("**");
	ImGui::SameLine(25); ImGui::Text("(pm)");
	if (ImGui::Selectable("*x", !strncmp(ctiling.symmetry_group(), "*x", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("*x");
	ImGui::SameLine(25); ImGui::Text("(cm)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("2-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("2222", !strncmp(ctiling.symmetry_group(), "2222", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("2222");
	ImGui::SameLine(45); ImGui::Text("(p2)");
	if (ImGui::Selectable("22x", !strncmp(ctiling.symmetry_group(), "22x", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("22x");
	ImGui::SameLine(45); ImGui::Text("(pgg)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*2222", !strncmp(ctiling.symmetry_group(), "*2222", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("*2222");
	ImGui::SameLine(55); ImGui::Text("(pmm)");
	if (ImGui::Selectable("2*22", !strncmp(ctiling.symmetry_group(), "2*22", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("2*22");
	ImGui::SameLine(55); ImGui::Text("(cmm)");
	if (ImGui::Selectable("22*", !strncmp(ctiling.symmetry_group(), "22*", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("22*");
	ImGui::SameLine(55); ImGui::Text("(pmg)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("3-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("333", !strncmp(ctiling.symmetry_group(), "333", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("333");
	ImGui::SameLine(35); ImGui::Text("(p3)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*333", !strncmp(ctiling.symmetry_group(), "*333", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("*333");
	ImGui::SameLine(45); ImGui::Text("(p3m1)");
	if (ImGui::Selectable("3*3", !strncmp(ctiling.symmetry_group(), "3*3", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("3*3");
	ImGui::SameLine(45); ImGui::Text("(p31m)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("4-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("442", !strncmp(ctiling.symmetry_group(), "442", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("442");
	ImGui::SameLine(35); ImGui::Text("(p4)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*442", !strncmp(ctiling.symmetry_group(), "*442", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("*442");
	ImGui::SameLine(45); ImGui::Text("(p4m)");
	if (ImGui::Selectable("4*2", !strncmp(ctiling.symmetry_group(), "4*2", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("4*2");
	ImGui::SameLine(45); ImGui::Text("(p4g)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("6-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("632", !strncmp(ctiling.symmetry_group(), "632", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("632");
	ImGui::SameLine(35); ImGui::Text("(p6)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*632", !strncmp(ctiling.symmetry_group(), "*632", 8), 0, {110, 0}))
		layer.tiling().set_symmetry_group("*632");
	ImGui::SameLine(45); ImGui::Text("(p6m)");
	ImGui::EndGroup();
}

void GUI::draw_symmetry_settings(void)
{
	auto& layer = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	ImGui::Text("Choose the symmetry group");
	ImGui::Separator();

	auto current_group = ctiling.symmetry_group();

	ImGui::Text("Current:"); ImGui::SameLine(148); ImGui::Text(current_group);
	ImGui::Dummy({0, 0}); ImGui::SameLine(100);
	ImGui::PushID("Group choice");
	if (ImGui::ImageButton((ImTextureID)(uintptr_t)thumbnail_map_[current_group], {120, 120}, {0, 1}, {1, 0}, 5))
		ImGui::OpenPopup("Choose a symmetry group");

	auto modal_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	if (ImGui::BeginPopupModal("Choose a symmetry group", NULL, modal_flags))
	{
		draw_symmetry_modal();
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

void GUI::draw_symmetry_modal(void)
{
	auto& layer = layering_.current_layer();

	bool modal_should_close = false;

	auto create_button_group = [&](const char* symmetry_group, int label_offset = 60)
	{
		ImGui::BeginGroup();
		ImGui::Dummy({0, 0}); ImGui::SameLine(label_offset);
		ImGui::Text(symmetry_group);
		ImGui::PushID(symmetry_group);
		if (ImGui::ImageButton((ImTextureID)(uintptr_t)thumbnail_map_[symmetry_group], {120, 120}, {0, 1}, {1, 0}, 10))
		{
			layer.tiling().set_symmetry_group(symmetry_group);
			modal_should_close = true;
		}
		ImGui::PopID();
		ImGui::EndGroup();
	};

	ImGui::PushID("modal buttons");

	// No rotations.
	create_button_group("o");
	ImGui::SameLine();
	create_button_group("xx");
	ImGui::SameLine();
	create_button_group("*x");
	ImGui::SameLine();
	create_button_group("**");

	// 2-fold rotations.
	create_button_group("2222", 45);
	ImGui::SameLine();
	create_button_group("22x", 45);
	ImGui::SameLine();
	create_button_group("22*", 45);
	ImGui::SameLine();
	create_button_group("2*22", 45);
	ImGui::SameLine();
	create_button_group("*2222", 45);

	// 3-fold rotations.
	create_button_group("333", 50);
	ImGui::SameLine();
	create_button_group("3*3", 50);
	ImGui::SameLine();
	create_button_group("*333", 50);

	// 4-fold rotations.
	create_button_group("442", 50);
	ImGui::SameLine();
	create_button_group("4*2", 50);
	ImGui::SameLine();
	create_button_group("*442", 50);

	// 6-fold rotations.
	create_button_group("632", 50);
	ImGui::SameLine();
	create_button_group("*632", 50);
	ImGui::SameLine();

	ImGui::PopID();

	if (modal_should_close)
		ImGui::CloseCurrentPopup();
}

void GUI::draw_view_settings(void)
{
	ImGui::Text("View settings");
	ImGui::Separator();

	ImGui::Text("Show result:"); ImGui::SameLine(130);
	ImGui::Checkbox("##Show result", result_visible_);

	ImGui::Text("Screen center:"); ImGui::SameLine(130);
	ImGui::PushItemWidth(-65.0f);
	ImGui::DragFloat2("##Screen center", screen_center_->data(), 0.01f);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if(ImGui::Button("Reset##Reset screen center"))
		*screen_center_ = {0.5, 0.5};

	// We need a float, not a double.
	float pixels_per_unit = *pixels_per_unit_;
	ImGui::Text("Zoom level:"); ImGui::SameLine(130);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Zoom level", &pixels_per_unit))
		*pixels_per_unit_ = pixels_per_unit;
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset zoom level"))
		*pixels_per_unit_ = 500.0;

	ImGui::Text("Background:"); ImGui::SameLine(130);
	ImGui::PushItemWidth(-1.0f);
	ImGui::ColorEdit3("##Background color", clear_color_->data());
	ImGui::PopItemWidth();
	ImGui::Dummy({0, 0}); ImGui::SameLine(130);
	if (ImGui::Button("Reset##Reset background color"))
		*clear_color_ = {0.1, 0.1, 0.1};
	ImGui::SameLine();
	ImGui::Button("Pick color...");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Not implemented yet :)");
		ImGui::EndTooltip();
	}
}

void GUI::draw_frame_settings(void)
{
	auto& layer = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	ImGui::Text("Frame settings");
	ImGui::Separator();

	ImGui::Text("Show frame:"); ImGui::SameLine(140);
	ImGui::Checkbox("##Show frame", frame_visible_);

	auto frame_position = ctiling.center();
	ImGui::Text("Frame position:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat2("##Frame position", frame_position.data(), 0.01f))
		layer.tiling().set_center(frame_position);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Reset##Reset frame position"))
		layer.tiling().set_center({0.5, 0.5});

	float frame_rotation = ctiling.rotation() / M_PI * 180.0f;
	ImGui::Text("Frame rotation:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Frame rotation", &frame_rotation, 0.5f))
		layer.tiling().set_rotation(frame_rotation / 180.0 * M_PI);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset frame rotation"))
		layer.tiling().set_rotation(0.0);

	float frame_scale = ctiling.scale();
	ImGui::Text("Frame scale:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Frame scale", &frame_scale, 0.01f, 0.001f, FLT_MAX))
		layer.tiling().set_scale(frame_scale);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset frame scale"))
		layer.tiling().set_scale(1.0);

	int num_domains = ctiling.num_lattice_domains();
	bool domains_changed = false;
	ImGui::Text("Domains:"); ImGui::SameLine(140);
	domains_changed |= ImGui::RadioButton("1##Domains 1", &num_domains, 1); ImGui::SameLine();
	domains_changed |= ImGui::RadioButton("4##Domains 2", &num_domains, 4); ImGui::SameLine();
	domains_changed |= ImGui::RadioButton("9##Domains 3", &num_domains, 9); ImGui::SameLine();
	if (domains_changed)
		layer.tiling().set_num_lattice_domains(num_domains);
}

void GUI::draw_image_settings(void)
{
	auto& layer = layering_.current_layer();

	// TODO: Improve.
	for (size_t idx = 0; idx < layer.size(); ++idx)
	{
		ImGui::PushID(idx);

		const auto& cimage = layer.as_const().image(idx);

		ImGui::Text("Image settings");
		ImGui::Separator();

		auto image_position = cimage.center();
		ImGui::Text("Image position:"); ImGui::SameLine(140);
		ImGui::PushItemWidth(-65.0f);
		if (ImGui::DragFloat2("##Image position", image_position.data(), 0.01f))
			layer.image(idx).set_center(image_position);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Reset##Reset image position"))
			layer.image(idx).set_center({0.5f, 0.5f});

		float image_rotation = cimage.rotation() / M_PI * 180.0f;
		ImGui::Text("Image rotation:"); ImGui::SameLine(140);
		ImGui::PushItemWidth(-65.0f);
		if (ImGui::DragFloat("##Image rotation", &image_rotation, 0.5f))
			layer.image(idx).set_rotation(image_rotation / 180.0f * M_PI);
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 12);
		if (ImGui::Button("Reset##Reset image rotation"))
			layer.image(idx).set_rotation(0.0);

		float image_scale = cimage.scale();
		ImGui::Text("Image scale:"); ImGui::SameLine(140);
		ImGui::PushItemWidth(-65.0f);
		if (ImGui::DragFloat("##Image scale", &image_scale, 0.01f, 0.001f, FLT_MAX))
			layer.image(idx).set_scale(image_scale);
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 12);
		if (ImGui::Button("Reset##Reset image scale"))
			layer.image(idx).set_scale(1.0);

		ImGui::PopID();
	}
}

[[deprecated]]
void GUI::draw_image_settings_old(void)
{
	auto& layer = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	ImGui::Text("Image settings");
	ImGui::Separator();

	auto image_position = ctiling.image_center();
	ImGui::Text("Image position:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat2("##Image position", image_position.data(), 0.01f))
		layer.tiling().set_image_center(image_position);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Reset##Reset image position"))
		layer.tiling().set_image_center({0.5f, 0.5f});

	float image_rotation = ctiling.image_rotation() / M_PI * 180.0f;
	ImGui::Text("Image rotation:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Image rotation", &image_rotation, 0.5f))
		layer.tiling().set_image_rotation(image_rotation / 180.0f * M_PI);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset image rotation"))
		layer.tiling().set_image_rotation(0.0);

	float image_scale = ctiling.image_scale();
	ImGui::Text("Image scale:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Image scale", &image_scale, 0.01f, 0.001f, FLT_MAX))
		layer.tiling().set_image_scale(image_scale);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset image scale"))
		layer.tiling().set_image_scale(1.0);
}

void GUI::draw_export_settings(void)
{
	auto& layer = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	ImGui::Text("Export settings");
	ImGui::Separator();

	int resolution[] = {*export_width_, *export_height_};
	ImGui::Text("Resolution:"); ImGui::SameLine(120);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragInt2("##Resolution", resolution, 1.0f, 512, 4096))
	{
		*export_width_  = std::max(512, std::min(resolution[0], 4096));
		*export_height_ = std::max(512, std::min(resolution[1], 4096));
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Reset##Reset resolution"))
	{
		*export_width_  = 1600;
		*export_height_ = 1200;
	}
	ImGui::Dummy({0, 0}); ImGui::SameLine(120);
	if (ImGui::Button("Fit to window"))
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		*export_width_  = width;
		*export_height_ = height;
	}

	char buffer[256] = {'\0'};
	std::strncpy(buffer, export_filename_.c_str(), 255);
	ImGui::Text("Export as:"); ImGui::SameLine(120);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::InputText("##Filename", buffer, 256, ImGuiInputTextFlags_CharsNoBlank))
		export_filename_ = buffer;
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset filename"))
		export_filename_ = export_base_name_ + '_' + ctiling.symmetry_group() + ".png";

	bool should_export = false;
	ImGui::Dummy({0, 0}); ImGui::SameLine(120);
	if (ImGui::Button("Export"))
	{
		if (export_filename_.empty())
			ImGui::OpenPopup("No filename");
		else
			should_export = true;
	}
	auto modal_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	if (ImGui::BeginPopupModal("No filename", NULL, modal_flags))
	{
		ImVec2 button_size = {140, 0.0f};
		auto default_name = export_base_name_ + '_' + ctiling.symmetry_group() + ".png";

		ImGui::Text("No filename set.");
		ImGui::Text("Use the default name \"%s\"?", default_name.c_str());
		if (ImGui::Button("OK##Export OK", button_size))
		{
			export_filename_ = default_name;
			should_export = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel##Export cancel", button_size))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (should_export)
		export_callback_(*export_width_, *export_height_, export_filename_.c_str());
}

void GUI::populate_thumbnail_map(void)
{
	thumbnail_map_["o"]     = GL::Texture::from_png("res/thumbnails/o");
	thumbnail_map_["xx"]    = GL::Texture::from_png("res/thumbnails/xx");
	thumbnail_map_["*x"]    = GL::Texture::from_png("res/thumbnails/_x");
	thumbnail_map_["**"]    = GL::Texture::from_png("res/thumbnails/__");

	thumbnail_map_["2222"]  = GL::Texture::from_png("res/thumbnails/2222");
	thumbnail_map_["22x"]   = GL::Texture::from_png("res/thumbnails/22x");
	thumbnail_map_["22*"]   = GL::Texture::from_png("res/thumbnails/22_");
	thumbnail_map_["2*22"]  = GL::Texture::from_png("res/thumbnails/2_22");
	thumbnail_map_["*2222"] = GL::Texture::from_png("res/thumbnails/_2222");

	thumbnail_map_["333"]   = GL::Texture::from_png("res/thumbnails/333");
	thumbnail_map_["3*3"]   = GL::Texture::from_png("res/thumbnails/3_3");
	thumbnail_map_["*333"]  = GL::Texture::from_png("res/thumbnails/_333");

	thumbnail_map_["442"]   = GL::Texture::from_png("res/thumbnails/442");
	thumbnail_map_["4*2"]   = GL::Texture::from_png("res/thumbnails/4_2");
	thumbnail_map_["*442"]  = GL::Texture::from_png("res/thumbnails/_442");

	thumbnail_map_["632"]   = GL::Texture::from_png("res/thumbnails/632");
	thumbnail_map_["*632"]  = GL::Texture::from_png("res/thumbnails/_632");
}
