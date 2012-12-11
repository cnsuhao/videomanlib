#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "GLWindow.h"
#include <qapplication.h>
#include <qtimer.h>
#include <string>
#include <iostream>

using namespace std;
using namespace VideoMan;

GLWindow::GLWindow( QWidget *parent, QGLWidget *shareWidget ) :
			QGLWidget(QGLFormat(QGL::SampleBuffers), parent, shareWidget)
{
	connect( this, SIGNAL( frameCallbackSignal( const char *, int ) ), this, SLOT( frameCallbackSlot( const char *, int ) ) );
	m_videoMan = new VideoManControl( VideoManControl::OPENGL_RENDERER );
	m_inputID = -1;
}

GLWindow::~GLWindow()
{
	makeCurrent();
	delete m_videoMan;
}

void GLWindow::timeOutSlot()
{
	m_videoMan->getFrame( m_inputID );
	updateGL();
}

void frameCallback( char *pixelBuffer, size_t input, double timeStamp, void *data )
{
	GLWindow *obj = (GLWindow*)data;
	obj->frameCallbackSignal( pixelBuffer, input );
}

void GLWindow::frameCallbackSlot( const char *pixelBuffer, int input )
{
	updateGL();
}

bool GLWindow::openVideoFile()
{	
	QString fileName = QFileDialog::getOpenFileName();
	if ( fileName.isEmpty() )
		return false;
	VMInputFormat format;
	VMInputIdentification device;
	//Initialize one input from a video file
	string fileNameSt = QString(fileName.toAscii()).toStdString();
	device.fileName = new char[fileNameSt.length() + 1];
	strcpy( device.fileName, fileNameSt.c_str() );
	if ( m_videoMan->supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
	else if ( m_videoMan->supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
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
	if ( ( m_inputID = m_videoMan->addVideoInput( device, &format ) ) != -1 )
	{
		m_videoMan->playVideo( m_inputID );
		if ( m_videoMan->supportFrameCallback( m_inputID ) )
			m_videoMan->setFrameCallback( m_inputID, frameCallback, this );
		else
		{			
			connect( &m_timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
			m_timer.start( 30 );
		}
		delete device.fileName;
		return true;
	}
	QMessageBox::critical( this, tr("Error"), tr("Loading video file " ) + fileName  );
	delete device.fileName;
	return false;
}


bool GLWindow::openCamera()
{
	VMInputIdentification *deviceList;
	int numDevices;
	m_videoMan->getAvailableDevices( &deviceList, numDevices ); //list all the available devices	
	int d = 0;	
	while ( d < numDevices && m_inputID == -1 )
	{
		VMInputFormat format;
		VMInputIdentification device = deviceList[d];
		format.showDlg = true;
		format.clock = true;
		format.dropFrames = true;
		makeCurrent();
		if ( ( m_inputID = m_videoMan->addVideoInput( device, &format ) ) != -1 )
		{
			m_videoMan->showPropertyPage( m_inputID );
			if ( m_videoMan->supportFrameCallback( m_inputID ) )
				m_videoMan->setFrameCallback( m_inputID, frameCallback, this );
			else
			{			
				connect( &m_timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
				m_timer.start( 30 );
			}
		}
		++d;
	}	
	m_videoMan->freeAvailableDevicesList( &deviceList, numDevices );
	if ( m_inputID == -1 )
	{
		QMessageBox::critical( this, tr("Error"), tr("Camera not found" ) );
		return false;
	}
	return true;
}


void GLWindow::initializeGL()
{
}
	
void GLWindow::resizeGL( int width, int height )
{
	m_videoMan->changeScreenSize( 0, 0, width, height );
}
	
void GLWindow::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT );
	m_videoMan->updateTexture( m_inputID );
	m_videoMan->releaseFrame( m_inputID );
	m_videoMan->renderFrame( m_inputID );
}
