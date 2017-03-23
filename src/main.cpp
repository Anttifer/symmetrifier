//#ifdef RELEASE
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

#include <QApplication>
#include "QtWindow.h"

//--------------------

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QtWindow window;
	window.show();

	return app.exec();
}
