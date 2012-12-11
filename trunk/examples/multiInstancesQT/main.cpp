#include <QApplication>
#include "mainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;
	window.resize( 600, 600 );

	//window.showFullScreen();
	window.show();

	return app.exec();
}