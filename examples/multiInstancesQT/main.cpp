#include <QApplication>
#include <QMessageBox>
#include "mainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;
	window.resize( 600, 600 );

	//window.showFullScreen();
	window.show();

	QMessageBox::about( NULL, "VideoMan Qt example", "This example shows how to create a MDI application with Qt. Each subwindow will contain a QGlWidget with its own VideoMan object" );

	return app.exec();
}