//#ifdef RELEASE
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

#include <QApplication>
#include "QtWindow.h"
#include <QOpenGLContext>

// Global OpenGL context pointer.
// The context is required anywhere OpenGL functions are called,
// and its lifetime must be constrained to that of QApplication,
// so it makes sense to declare a global pointer to it.
QOpenGLContext* g_pContext;

//--------------------

int main(int argc, char* argv[])
{
	QSurfaceFormat format;
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication app(argc, argv);

	QOpenGLContext context;
	context.create();
	g_pContext = &context;

	QtWindow window;
	window.show();

	return app.exec();
}
