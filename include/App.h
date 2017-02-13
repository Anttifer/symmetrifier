#ifndef APP_H
#define APP_H

#include "Window.h"
#include "ShaderCanvas.h"
#include "Mesh.h"
#include "PinwheelPlane.h"
#include "GLObjects.h"

//--------------------

class App {
public:
	App (int argc, char* argv[]);
	App (const App&) = delete;

	App& operator=(const App&) = delete;

	void loop (void);

private:
	void render_wave           (int width, int height, GLuint framebuffer = 0);
	void render_pinwheel       (int width, int height, GLuint framebuffer = 0);
	void render_texture        (const GL::Texture& texture, int width, int height, GLuint framebuffer = 0);
	void render_mesh           (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
	void render_extruded_mesh  (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
	void render_on_mesh        (const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer = 0);

	void screenshot (int width, int height);

	// Framework objects.
	MainWindow           window_;

	ShaderCanvas         canvas_;
	Mesh                 cube_;
	Mesh                 torus_;
	PinwheelPlane        plane_;

	GL::Texture          image_;
	GL::Texture          depth_;
	GL::FBO              framebuffer_;

	double               time_;
};
#endif // APP_H
