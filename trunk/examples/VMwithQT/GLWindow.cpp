#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "GLWindow.h"
#include <QApplication>
#include <QString>
#include <QTimer>
#include <string>
#include <iostream>
#include "cameraSelector.h"


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
	QString fileName = QFileDialog::getOpenFileName( this, tr("Openg Video File"), m_dirPath );
	if ( fileName.isEmpty() )
		return;
	//Get the path of the file
	QDir dir( fileName );
	dir.cdUp();
	m_dirPath = dir.absolutePath();

	//Initialize one input from a video file
	int videoInputID;
	VMInputFormat format;
	VMInputIdentification device;
	device.fileName = new char[fileName.length() + 1];
	strcpy( device.fileName, fileName.toAscii().data() );
	bool withHighgui = false;
	if ( m_videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow
	else if ( m_videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
	{
        device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui
        withHighgui = true;
	}

	else
		QMessageBox::critical( this, tr("Video files not supported"), tr("You have to build VMDirectShow or VMHighgui" ) );
	//play in real-time
	format.clock = true;
	//Render the audio stream
	format.renderAudio = true;
	//Initialize the video file is the path
	makeCurrent();
	if ( ( videoInputID = m_videoMan.addVideoInput( device, &format ) ) != -1 )
	{
		m_videoMan.playVideo( videoInputID );
		m_videoInputIDs.push_back( videoInputID );
        if ( withHighgui )
        {
            m_videoMan.setVerticalFlip( videoInputID, true );
        }
	}
	delete device.fileName;
}


void GLWindow::openCamera()
{
	VMInputIdentification *deviceList;
	int numDevices;
	m_videoMan.getAvailableDevices( &deviceList, numDevices ); //list all the available devices

	//Show camera selector dialog
	CameraSelector *cameraSelector = new CameraSelector( this, true, true );
	//fill the devices list
	for ( int i = 0; i < numDevices; i++ )
	{
		QListWidgetItem *lst =new QListWidgetItem( QString( deviceList[i].identifier ) + QString( " - " ) + QString( deviceList[i].friendlyName ), cameraSelector->listWidget );
		cameraSelector->listWidget->insertItem(i,lst);
	}
	cameraSelector->listWidget->show();
	bool error = false;
	vector<VMInputIdentification> selected;
	if ( cameraSelector->listWidget->count() != 0 )
	{
		cameraSelector->exec();
		if ( cameraSelector->isAccepted() )
		{
			vector<int> selectedIndexes;
			cameraSelector->getSelectedIndexes( selectedIndexes );
			for ( int s = 0; s < (int)selectedIndexes.size(); ++s )
				selected.push_back( deviceList[selectedIndexes[s]] );
		}
		else
			error = true;
	}
	else
	{
		QMessageBox::warning( this, tr("Open Camera"), tr("No device available"), QMessageBox::Ok );
		error = true;
	}
	delete cameraSelector;
	if ( error || selected.empty() )
	{
		m_videoMan.freeAvailableDevicesList( &deviceList, numDevices );
		return;
	}

	//Initialize the selected devices
	int d = 0;
	makeCurrent();
	for ( int d = 0; d < (int)selected.size(); ++d )
	{
		VMInputFormat format;
		VMInputIdentification device = selected[d];
		format.showDlg = true;
		format.clock = true;
		int inputID;
		if ( ( inputID = m_videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			m_videoMan.showPropertyPage( inputID );
			m_videoInputIDs.push_back( inputID );
		}
		else
			QMessageBox::critical( this, tr("Error"), tr("Initializing camera " ) + QString( device.friendlyName ) );
	}
	m_videoMan.freeAvailableDevicesList( &deviceList, numDevices );
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
	for ( int i = 0; i < (int)m_videoInputIDs.size(); ++i )
	{
		if ( m_videoMan.getFrame( m_videoInputIDs[i] ) )
		{
			m_videoMan.updateTexture( m_videoInputIDs[i] );
			m_videoMan.releaseFrame( m_videoInputIDs[i] );
		}
		m_videoMan.renderFrame( m_videoInputIDs[i] );
		m_videoMan.drawInputBorder( m_videoInputIDs[i], 1, 0.6f, 0.6f, 0.6f  );
	}
}
