//#ifdef RELEASE
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

#include "App.h"

//--------------------

int main(int argc, char* argv[]) {
	App app(argc, argv);
	app.loop();

	return 0;
}
