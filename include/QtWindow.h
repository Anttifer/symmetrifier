#ifndef QTWINDOW_H
#define QTWINDOW_H

#include <QMainWindow>
class QtSymmetryCanvas;
class QPlainTextEdit;
class QPushButton;

class QtWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit QtWindow   (QWidget* parent = 0);
	QtWindow            (const QtWindow&) = delete;
	QtWindow& operator= (const QtWindow&) = delete;

private slots:
	void wut_slot(void);

private:
	QWidget* central_area_;
	QtSymmetryCanvas* symmetry_canvas_;
	QPushButton* left_button_;
};
#endif // QTWINDOW_H
