#include <QMainWindow>
#include <QFileDialog>
#include "mainWindow.h"
#include "VideoManInputFormat.h"
#include <string>
#include <iostream>
#include "GLWindow.h"

using namespace std;

MainWindow::MainWindow( )
{	
	setCentralWidget( &m_mdiArea );	
	//m_mdiArea.setViewMode( QMdiArea::TabbedView );

	//Create the menu bar
	QMenuBar *menu = new QMenuBar();
	QMenu *video = new QMenu( "&Video Input", menu );
	QMenu *Add = new QMenu( "&Add", video );
	Add->addAction( "&Camera", this, SLOT(openCamera()) );
	Add->addAction( "&Video File", this, SLOT(openVideoFile()) );
	video->addMenu( Add );	
	menu->addMenu( video );
	setMenuBar( menu );

	//All the videoman objects will share the m_mainGLwidget's Opengl context
	m_mainGLwidget = new QGLWidget();
	//Each videoman object will have each own Opengl context
	//m_mainGLwidget = NULL;
}

MainWindow::~MainWindow()
{
	if ( m_mainGLwidget )
		delete m_mainGLwidget;
}

void MainWindow::openVideoFile()
{
	// Share the OpenGL contexts for avoid problems
	GLWindow *windowGL = new GLWindow( this, m_mainGLwidget );		
	if ( windowGL->openVideoFile() )
	{
		m_mdiArea.addSubWindow( windowGL );
		windowGL->show();
		m_mdiArea.tileSubWindows ();
	}
	else
		delete windowGL;
}


void MainWindow::openCamera()
{
	// Share the OpenGL contexts for avoid problems
	GLWindow *windowGL = new GLWindow( this, m_mainGLwidget );
	if ( windowGL->openCamera() )
	{
		m_mdiArea.addSubWindow( windowGL );
		windowGL->show();
		m_mdiArea.tileSubWindows ();
	}
	else
		delete windowGL;
}