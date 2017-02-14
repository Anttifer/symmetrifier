#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Eigen/Geometry>

class MainWindow;

//--------------------

class InputManager {
public:
	InputManager(MainWindow&);

	Eigen::Vector2f mouse_position      (void);

	// TODO: Generalize this to button_pressed.
	bool            left_button_pressed (void);
private:
	MainWindow& window_;
};

#endif // INPUTMANAGER_H
