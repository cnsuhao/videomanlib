#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "GLWindow.h"
#include <qapplication.h>
#include <qtimer.h>
#include <string>
#include <iostream>

using namespace std;
using namespace VideoMan;


GLWindow::GLWindow( int timerInterval, QWidget *parent, const char *name ) :
			QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	if( timerInterval == 0 )
		m_timer = 0;
	else
	{
		m_timer = new QTimer( this );
		connect( m_timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
		m_timer->start( timerInterval );
	}
}

void GLWindow::keyPressEvent( QKeyEvent *e )
{
	switch( e->key() )
	{
	case Qt::Key_Escape:
		close();
	}
}

void GLWindow::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		float x = static_cast<float>( e->x() );
		float y = static_cast<float>( e->y() );
		int input;
		if ( ( input = m_videoMan.screenToImageCoords( x, y ) ) != -1 )
		{
			m_videoMan.changeMainVisualizationInput( input );
		}
	}
}

void GLWindow::timeOutSlot()
{
	updateGL();
}

void GLWindow::changeMode0()
{
	m_videoMan.changeVisualizationMode( 0 );
}

void GLWindow::changeMode1()
{
	m_videoMan.changeVisualizationMode( 1 );
}

void GLWindow::openVideoFile()
{	
	QString fileName = QFileDialog::getOpenFileName();
	if ( fileName.isEmpty() )
		return;
	int videoInputID;
	VMInputFormat format;
	VMInputIdentification device;
	//Initialize one input from a video file
	string fileNameSt = QString(fileName.toAscii()).toStdString();
	device.fileName = new char[fileNameSt.length() + 1];
	strcpy( device.fileName, fileNameSt.c_str() );
	if ( m_videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
	else if ( m_videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
		device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
	else 
		QMessageBox::critical( this, tr("Video files not supported"), tr("You have to build VMDirectShow or VMHighgui" ) );
	//play in real-time
	format.clock = true;
	format.dropFrames = true;
	//Render the audio stream
	format.renderAudio = true;
	//Initialize the video file is the path 
	makeCurrent();
	if ( ( videoInputID = m_videoMan.addVideoInput( device, &format ) ) != -1 )
	{
		m_videoMan.playVideo( videoInputID );
		m_videoInputIDs.push_back( videoInputID );
	}
	delete device.fileName;
}


void GLWindow::openCamera()
{
	VMInputIdentification *deviceList;
	int numDevices;
	m_videoMan.getAvailableDevices( &deviceList, numDevices ); //list all the available devices
	int inputID = -1;
	int d = 0;
	makeCurrent();
	while ( d < numDevices && inputID == -1 )
	{
		VMInputFormat format;
		VMInputIdentification device = deviceList[d];
		format.showDlg = true;
		format.clock = true;
		format.dropFrames = true;
		if ( ( inputID = m_videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			m_videoMan.showPropertyPage( inputID );
			m_videoInputIDs.push_back( inputID );
		}
		++d;
	}	
	m_videoMan.freeAvailableDevicesList( &deviceList, numDevices );
	if ( inputID == -1 )
		QMessageBox::critical( this, tr("Error"), tr("Camera not found" ) );
}


void GLWindow::initializeGL()
{
	
}
	
void GLWindow::resizeGL( int width, int height )
{
	m_videoMan.changeScreenSize( 0, 0, width, height );
}
	
void GLWindow::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT );
	for ( int i = 0; i < m_videoInputIDs.size(); ++i )
	{
		m_videoMan.getFrame( m_videoInputIDs[i] );
		m_videoMan.updateTexture( m_videoInputIDs[i] );
		m_videoMan.releaseFrame( m_videoInputIDs[i] );
		m_videoMan.renderFrame( m_videoInputIDs[i] );
		m_videoMan.drawInputBorder( m_videoInputIDs[i], 1, 0.6f, 0.6f, 0.6f  );
	}
}
