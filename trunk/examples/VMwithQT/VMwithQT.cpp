#include <QApplication>
#include "GLWindow.h"
#include <qmenubar.h>


class QPopupMenu;

 int main(int argc, char *argv[])
 {
	QApplication app(argc, argv);

	QMainWindow window;
	window.resize( 600, 600 );
	
	GLWindow *windowGL = new GLWindow();
	window.setCentralWidget( windowGL );

	//Create the menu bar
	QMenuBar *menu = new QMenuBar();
	QMenu *video = new QMenu( "&Video Input", menu );
	QMenu *Add = new QMenu( "&Add", video );
	Add->addAction( "&Camera", windowGL, SLOT(openCamera()) );
	Add->addAction( "&Video File", windowGL, SLOT(openVideoFile()) );
	video->addMenu( Add );
	QMenu *renderer = new QMenu( "&Renderer", menu );
	QMenu *mode = new QMenu( "&Visualization Mode", renderer );
	mode->addAction( "Mode 0", windowGL, SLOT(changeMode0()) );
	mode->addAction( "Mode 1", windowGL, SLOT(changeMode1()) );
	renderer->addMenu( mode );
	menu->addMenu( video );
	menu->addMenu( renderer );
	
	window.setMenuBar( menu );

	//window.showFullScreen();
	window.show();
	
	return app.exec();
 }