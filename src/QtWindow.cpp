#include "QtWindow.h"
#include "QtSymmetryCanvas.h"
#include <QtWidgets>

QtWindow::QtWindow(QWidget* parent) :
	QMainWindow(parent)
{
	QDesktopWidget desktop;
	int width  = desktop.width()*0.8;
	int height = desktop.height()*0.8;

	resize({width, height});

	central_area_ = new QWidget;
	setCentralWidget(central_area_);

	auto layout      = new QHBoxLayout;
	symmetry_canvas_ = new QtSymmetryCanvas;
	left_button_     = new QPushButton("Kek!");

	layout->addWidget(left_button_);
	layout->addWidget(symmetry_canvas_, 1);

	central_area_->setLayout(layout);

	auto file_menu = menuBar()->addMenu("Fil");

	QAction* wut_action = new QAction("wut", this);
	file_menu->addAction(wut_action);
}

void QtWindow::wut_slot(void)
{
	printf("Kek!\n");
}

