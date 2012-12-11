#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "GLWindow.h"
#include <qapplication.h>
#include <qtimer.h>
#include <string>
#include <iostream>
#include <cxcore.h>
#include <cv.h>
	
using namespace std;

VideoManControl videoMan;

GLWindow::GLWindow( QMainWindow *window, int timerInterval, QWidget *parent, const char *name ) :
			QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	setFocusPolicy(Qt::StrongFocus);

	if( timerInterval == 0 )
		m_timer = 0;
	else
	{
		m_timer = new QTimer( this );
		connect( m_timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
		m_timer->start( timerInterval );
	}
	m_image = NULL;
	m_binary = NULL;
	m_binaryAux = NULL;
	m_grey = NULL;
	m_storage = NULL;

	m_findContoursMode = 2;
	m_findContoursModes.push_back( IND_TEXT( CV_RETR_EXTERNAL, "CV_RETR_EXTERNAL" ) );
	m_findContoursModes.push_back( IND_TEXT( CV_RETR_LIST, "CV_RETR_LIST" ) );
	m_findContoursModes.push_back( IND_TEXT( CV_RETR_CCOMP, "CV_RETR_CCOMP" ) );
	m_findContoursModes.push_back( IND_TEXT( CV_RETR_TREE, "CV_RETR_TREE" ) );

	m_contoursApproximationMode = 1;
	m_contoursApproximationModes.push_back( IND_TEXT( CV_CHAIN_CODE, "CV_CHAIN_CODE" ) );
	m_contoursApproximationModes.push_back( IND_TEXT( CV_CHAIN_APPROX_NONE, "CV_CHAIN_APPROX_NONE" ) );
	m_contoursApproximationModes.push_back( IND_TEXT( CV_CHAIN_APPROX_SIMPLE, "CV_CHAIN_APPROX_SIMPLE" ) );
	m_contoursApproximationModes.push_back( IND_TEXT( CV_CHAIN_APPROX_TC89_L1, "CV_CHAIN_APPROX_TC89_L1" ) );
	m_contoursApproximationModes.push_back( IND_TEXT( CV_CHAIN_APPROX_TC89_KCOS, "CV_CHAIN_APPROX_TC89_KCOS" ) );
	m_contoursApproximationModes.push_back( IND_TEXT( CV_LINK_RUNS, "CV_LINK_RUNS" ) );

	m_morphologyOp = 0;
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_ERODE, "CV_MOP_ERODE" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_DILATE, "CV_MOP_DILATE" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_OPEN, "CV_MOP_OPEN" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_CLOSE, "CV_MOP_CLOSE" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_GRADIENT, "CV_MOP_GRADIENT" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_TOPHAT, "CV_MOP_TOPHAT" ) );
	m_morphologyOps.push_back( IND_TEXT( CV_MOP_BLACKHAT, "CV_MOP_BLACKHAT" ) );	

	m_minThreshold = 0;
	m_maxThreshold = 0;
	m_invertThresh = false;

	m_mainWindow = window;
	createToolbar();
}

GLWindow::~GLWindow()
{
	if ( m_image )
		cvReleaseImage( &m_image );
	if ( m_binary )
		cvReleaseImage( &m_binary );
	if ( m_binaryAux )
		cvReleaseImage( &m_binaryAux );	
	if ( m_grey )
		cvReleaseImage( &m_grey );
	if ( m_storage )
		cvReleaseMemStorage( &m_storage);
}

void GLWindow::createToolbar()
{
	QToolBar *m_toolBar = new QToolBar( tr( "Controls" ) );	
	m_toolBar->setFixedWidth( 200 );
	
	// Find contours parameters
	QGroupBox *modesGrp = new QGroupBox( tr( "Modes" ) );
	QVBoxLayout *modesGrpLayout = new QVBoxLayout( modesGrp );
	modesGrpLayout->addWidget( new QLabel( tr("Find contour" ), modesGrp ) );	
	QComboBox *listFindModes = new QComboBox();
	modesGrpLayout->addWidget( listFindModes );	
	modesGrpLayout->addWidget( new QLabel( tr("Contour approximation" ), modesGrp  ) );
	QComboBox *listApproxModes = new QComboBox();
	modesGrpLayout->addWidget( listApproxModes );
	
	// cvThreshold parameters
	QGroupBox *thresholdGrp = new QGroupBox( tr( "Thresholds" ) );
	QGridLayout *thresholdGrpLayout = new QGridLayout( thresholdGrp );
	QLabel *minThreshLabel = new QLabel( tr("Min Thresh") );
	QSpinBox *minThresh = new QSpinBox();
	minThresh->setRange( 0, 255 );
	QLabel *maxThreshLabel = new QLabel( tr("Max Thresh") );
	QSpinBox *maxThresh = new QSpinBox();
	maxThresh->setRange( 0, 255 );
	QCheckBox *invertThresh = new QCheckBox( tr( "Invert" ) );
	thresholdGrpLayout->addWidget( minThreshLabel, 0, 0 );
	thresholdGrpLayout->addWidget( minThresh, 0, 1 );
	thresholdGrpLayout->addWidget( maxThreshLabel, 1, 0 );	
	thresholdGrpLayout->addWidget( maxThresh, 1, 1 );	
	thresholdGrpLayout->addWidget( invertThresh, 2, 0, 1, 2 );	

	// Morphology operations controls
	QGroupBox *morphologyGrp = new QGroupBox( tr( "Morphology" ) );
	QVBoxLayout *morphologyGrpLayout = new QVBoxLayout( morphologyGrp );
	morphologyGrpLayout->addWidget( new QLabel( tr("Operation" ), morphologyGrp ) );	
	QComboBox *listOperations = new QComboBox( morphologyGrp );
	morphologyGrpLayout->addWidget( listOperations );	


	//Blob properties table
	QGroupBox *propertiesGrp = new QGroupBox( tr( "Selected blob properties" ) );
	QVBoxLayout *propertiesGrpLayout = new QVBoxLayout( propertiesGrp );
	QStringList headers;
	headers.push_back( "Area" );
	headers.push_back( "Perimeter" );
	headers.push_back( "Elongation" );
	headers.push_back( "Roundness" );
	m_blobPropertiesTable = new QTableWidget( headers.size(), 1 );		
	m_blobPropertiesTable->setVerticalHeaderLabels( headers );
	m_blobPropertiesTable->horizontalHeader()->hide( );
	for ( int h = 0; h < headers.size(); ++h )
	{
		m_blobPropertiesTable->setItem( h, 0, new QTableWidgetItem ); 
		m_blobPropertiesTable->item( h, 0 )->setText( QString::number( 0.0 ) );
	}	
	propertiesGrpLayout->addWidget( m_blobPropertiesTable );
	propertiesGrpLayout->addItem( new QSpacerItem( 1, 20, QSizePolicy::Expanding, QSizePolicy::Expanding ) );

	// Videoman logo
	QLabel *logo = new QLabel();
	logo->setPixmap( QPixmap( "videomanLogo.png" ) );

	
	m_toolBar->addWidget( modesGrp );
	m_toolBar->addWidget( thresholdGrp );
	m_toolBar->addWidget( morphologyGrp);
	m_toolBar->addWidget( propertiesGrp);
	m_toolBar->addWidget( logo );

	m_mainWindow->addToolBar(Qt::RightToolBarArea,m_toolBar);

	setListInputs( listFindModes, m_findContoursModes, m_findContoursMode );
	connect( listFindModes, SIGNAL(currentIndexChanged(int)), this, SLOT(sltListFindModesChanged(int)) );
	setListInputs( listApproxModes, m_contoursApproximationModes, m_contoursApproximationMode );
	connect( listApproxModes, SIGNAL(currentIndexChanged(int)), this, SLOT(sltListApproxModesChanged(int)) );	
	setListInputs( listOperations, m_morphologyOps, m_morphologyOp );
	connect( listOperations, SIGNAL(currentIndexChanged(int)), this, SLOT(sltListMorphologyOpChanged(int)) );

	connect( minThresh, SIGNAL( valueChanged (int) ), this, SLOT( sltMinThreshChanged(int)) );
	connect( maxThresh, SIGNAL( valueChanged (int) ), this, SLOT( sltMaxThreshChanged(int)) );
	connect( invertThresh, SIGNAL( 	stateChanged(int) ), this, SLOT( sltInvertThreshChanged(int)) );
}

bool GLWindow::event( QEvent *event )
{
	if (event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = (QKeyEvent *)event;
		keyPressEvent(keyEvent);
		return false;
	}

	else if (event->type() == QEvent::MouseMove) 
	{
		QMouseEvent *mouseEvent = (QMouseEvent *)event;
		mouseMoveEvent(mouseEvent);
		return false;
	}

	return QGLWidget::event(event);
}

void GLWindow::keyPressEvent( QKeyEvent *e )
{
	switch( e->key() )
	{	
	case Qt::Key_Plus:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x, y, zoom + 1 );
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x, y, zoom + 1 );
			break;
		}
	case Qt::Key_Minus:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x, y, zoom - 1 );
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x, y, zoom - 1 );
			break;
		}
	case Qt::Key_Left:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x - 5, y, zoom, true);
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x - 5, y, zoom, true );
			break;
		}
	case Qt::Key_Right:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x + 5, y, zoom, true );
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x + 5, y, zoom, true );
			break;
		}
	case Qt::Key_Up:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x, y + 5, zoom, true );
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x, y + 5, zoom, true );
			break;
		}
	case Qt::Key_Down:
		{
			float x, y, zoom;
			videoMan.getRendererZoom( videoInputID, x, y, zoom );
			videoMan.setRendererZoom( videoInputID, x, y - 5, zoom, true );
			videoMan.getRendererZoom( binaryInputID, x, y, zoom );
			videoMan.setRendererZoom( binaryInputID, x, y - 5, zoom, true );
			break;
		}
	case Qt::Key_R:
		{
			videoMan.resetRendererZoom( videoInputID );
			videoMan.resetRendererZoom( binaryInputID );
		}
	}
}

void GLWindow::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		float x = static_cast<float>( e->x() );
		float y = static_cast<float>( e->y() );
		int input;
		if ( ( input = videoMan.screenToImageCoords( x, y ) ) != -1 )
		{
			for ( int b = 0; b < (int)m_blobs.size(); ++b )
			{
				if ( x >= m_blobs[b].rect.x && x < m_blobs[b].rect.x + m_blobs[b].rect.width &&
					y >= m_blobs[b].rect.y && y < m_blobs[b].rect.y + m_blobs[b].rect.height )
				{
					m_selectedBlob = b;

					m_blobPropertiesTable->item( 0, 0 )->setText( QString::number( m_blobs[b].area ) );
					m_blobPropertiesTable->item( 1, 0 )->setText( QString::number( m_blobs[b].perimeter ) );
					m_blobPropertiesTable->item( 2, 0 )->setText( QString::number( m_blobs[b].elongation ) );
					m_blobPropertiesTable->item( 3, 0 )->setText( QString::number( m_blobs[b].roundness ) );
					return;
				}
			}
		}
	}
}

void GLWindow::timeOutSlot()
{
	updateGL();
}

void GLWindow::changeMode0()
{
	videoMan.changeVisualizationMode( 0 );
}

void GLWindow::changeMode1()
{
	videoMan.changeVisualizationMode( 1 );
}

void GLWindow::openVideoFile()
{	
	videoMan.deleteInputs();
	QString fileName = QFileDialog::getOpenFileName();
	if ( fileName.isEmpty() )
		return;
	int videoInputID;
	VideoManInputFormat format;
	inputIdentification device;
	//Initialize one input from a video file
	string fileNameSt = QString(fileName.toAscii()).toStdString();
	device.fileName = new char[fileNameSt.length() + 1];
	strcpy( device.fileName, fileNameSt.c_str() );
	if ( videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
	else if ( videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
		device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
	else 
		QMessageBox::critical( this, tr("Video files not supported"), tr("You have to build VMDirectShow or VMHighgui" ) );
	//play in real-time
	format.clock = true;
	format.dropFrames = true;
	//Render the audio stream
	format.renderAudio = true;
	//Initialize the video file is the path 
	if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
	{
		videoMan.playVideo( videoInputID );
	}
	delete device.fileName;
}


void GLWindow::openCamera()
{
	videoMan.deleteInputs();
	inputIdentification *deviceList;
	int numDevices;
	videoMan.getAvailableDevices( &deviceList, numDevices ); //list all the available devices
	int inputID = -1;
	int d = 0;
	while ( d < numDevices && inputID == -1 )
	{
		VideoManInputFormat format;
		inputIdentification device = deviceList[d];
		format.showDlg = true;
		format.clock = true;
		format.dropFrames = true;
		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			videoMan.showPropertyPage( inputID );
		}
		++d;
	}	
	videoMan.freeAvailableDevicesList( &deviceList, numDevices );
	if ( inputID == -1 )
		QMessageBox::critical( this, tr("Error"), tr("Camera not found" ) );
}

void GLWindow::openImageFile( QString fileName )
{
	m_image = cvLoadImage( fileName.toStdString().c_str() );
	if( !m_image )
		return;
	videoMan.setTextureFiltering( 0 );
	
	VideoManInputFormat format;
	inputIdentification device;
	//Initialize one input from an image file	
	device.identifier = "USER_INPUT"; //using highugui
	if ( m_image->nChannels == 3 )
		format.SetFormat( m_image->width, m_image->height, 30, BGR24, BGR24 );
	else if ( m_image->nChannels == 4 )
		format.SetFormat( m_image->width, m_image->height, 30, BGR32, BGR32 );
	else if ( m_image->nChannels == 1 && m_image->depth == 8 )
		format.SetFormat( m_image->width, m_image->height, 30, GREY8, GREY8 );
	else if ( m_image->nChannels == 1 && m_image->depth == 16 )
		format.SetFormat( m_image->width, m_image->height, 30, GREY16, GREY16 );
	else
		return;	
	if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
	{
		videoMan.setUserInputImage( videoInputID, m_image->imageData );
		videoMan.setVerticalFlip( videoInputID, m_image->origin == 0 );		
	}
	m_binary = cvCreateImage( cvSize( m_image->width, m_image->height ), 8, 1 );
	format.SetFormat( m_image->width, m_image->height, 30, GREY8, GREY8 );
	if ( ( binaryInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
	{
		videoMan.setUserInputImage( binaryInputID, m_binary->imageData );
		videoMan.setVerticalFlip( binaryInputID, m_image->origin == 0 );		
	}

	m_binaryAux = cvCreateImage( cvSize( m_image->width, m_image->height ), 8, 1 );

	m_grey = cvCreateImage( cvSize( m_image->width, m_image->height ), 8, 1 );
	cvCvtColor( m_image, m_grey, CV_RGB2GRAY );
	processImage();
}

void GLWindow::openImageFile()
{
	videoMan.deleteInputs();
	QString fileName = QFileDialog::getOpenFileName();
	if ( fileName.isEmpty() )
		return;
	openImageFile( fileName );
}


void GLWindow::initializeGL()
{
	
}
	
void GLWindow::resizeGL( int width, int height )
{
	videoMan.changeScreenSize( 0, 0, width, height );
}


void drawCross( float x, float y, float side )
{
	glBegin( GL_LINES );
		glVertex2f( x - side, y );
		glVertex2f( x + side, y );
		glVertex2f( x, y + side );
		glVertex2f( x, y - side );
	glEnd();
}

void GLWindow::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT );
	
	if ( videoMan.getNumberOfInputs() == 0 )
		return;
	videoMan.getFrame( videoInputID );
	videoMan.updateTexture( videoInputID );
	videoMan.releaseFrame( videoInputID );
	videoMan.renderFrame( videoInputID );
	videoMan.renderFrame( binaryInputID );
	videoMan.activate2DDrawingSetup( binaryInputID );
	glDisable( GL_TEXTURE_2D );
	
	for ( int b = 0; b < (int)m_blobs.size(); ++b )
	{
		if ( m_selectedBlob == b )
			glColor3f( 1.0f, 0.0f ,0.0f );
		else
			glColor3f( 0.0f, 1.0f ,0.0f );
			
		glBegin( GL_LINE_LOOP );
			glVertex2i( m_blobs[b].rect.x, m_blobs[b].rect.y );
			glVertex2i( m_blobs[b].rect.x + m_blobs[b].rect.width, m_blobs[b].rect.y );
			glVertex2i( m_blobs[b].rect.x + m_blobs[b].rect.width, m_blobs[b].rect.y + m_blobs[b].rect.height );
			glVertex2i( m_blobs[b].rect.x, m_blobs[b].rect.y + m_blobs[b].rect.height );
		glEnd();

		if ( m_selectedBlob == b )
		{
			glLineWidth( 3.0f );
			glColor3f( 0.0f, 0.0f, 5.0f );
			drawCross( m_blobs[b].centroide.x + 0.5f, m_blobs[b].centroide.y + 0.5f, 8 );			
			glColor3f( 1.0f, 0.0f, 0.0f );
			drawCross(  m_blobs[b].rect.x + m_blobs[b].rect.width * 0.5f, m_blobs[b].rect.y + m_blobs[b].rect.height * 0.5f, 4 );
			glLineWidth( 1.0f );
			glColor3f( 0.0f, 1.0f, 0.0f );
			glPushMatrix();
			glTranslatef( 0.5f + m_blobs[b].ellipse.center.x, 0.5f + m_blobs[b].ellipse.center.y, 0.0f );
			glRotatef( m_blobs[b].ellipse.angle, 0.0f, 0.0f, 1.0f );
			glBegin( GL_LINES );
				glVertex2f( m_blobs[b].ellipse.size.width * 0.5f, 0.0f );
				glVertex2f( -m_blobs[b].ellipse.size.width * 0.5f, 0.0f );
				glVertex2f( 0.0f, m_blobs[b].ellipse.size.height * 0.5f );
				glVertex2f( 0.0f, -m_blobs[b].ellipse.size.height * 0.5f );
			glEnd();
			glPopMatrix();
			//m_blobs[b].ellipse.
			//cvEllipseBox( img, m_blobs[b].ellipse, CV_RGB(155,155, 155), 1 );
		}
	}
	glEnable( GL_TEXTURE_2D );
}

void GLWindow::setListInputs( QComboBox *list, const std::vector<IND_TEXT> &inputs, int currentIndex )
{
	for ( int m = 0; m < (int)inputs.size(); ++m )
	{
		list->addItem( inputs[m].second );
	}
	list->setCurrentIndex( currentIndex );
}

void GLWindow::sltListFindModesChanged( int index )
{
	m_selectedBlob = -1;
	m_findContoursMode = index;
	processImage();
}


void GLWindow::sltListApproxModesChanged( int index )
{
	m_selectedBlob = -1;
	m_contoursApproximationMode = index;
	processImage();
}

void GLWindow::sltListMorphologyOpChanged( int index )
{
	m_morphologyOp = index;
	processImage();
}

void GLWindow::processImage()
{
	//Threshold
	cvThreshold( m_grey, m_binary, m_minThreshold, 255, CV_THRESH_BINARY );
	cvThreshold( m_grey, m_binaryAux, m_maxThreshold, 255, CV_THRESH_BINARY_INV );
	cvAnd( m_binaryAux, m_binary, m_binary );
	if ( m_invertThresh )
		cvNot( m_binary, m_binary );

	//Morphology
	cvMorphologyEx( m_binary, m_binary, NULL, NULL, m_morphologyOps[m_morphologyOp].first );
	videoMan.updateTexture( binaryInputID, m_binary->imageData );

	//Find Contours
	m_storage = cvCreateMemStorage();
	CvSeq *first;
	const int nC = cvFindContours( m_binary, m_storage, &first, sizeof(CvContour), 
		m_findContoursModes[m_findContoursMode].first, m_contoursApproximationModes[m_contoursApproximationMode].first );		
	m_blobs.clear();
	for( ; first != 0; first = first->h_next )
	{
		CvMoments moments;
		cvMoments( first, &moments, 1 );
		CvContour *countour = (CvContour*)first;

		BLOB blob;
		blob.rect = countour->rect;
		blob.area = moments.m00;
		blob.perimeter = cvContourPerimeter( countour );
		blob.roundness = 4.0f * CV_PI * blob.area / ( blob.perimeter * blob.perimeter );
		const double dif = ( moments.mu20 - moments.mu02 );
		const double sum = ( moments.mu20 + moments.mu02 );
		blob.elongation = fabs( ( dif * dif - ( 4.0 * moments.mu11 * moments.mu11 ) ) / ( sum * sum ) );
		blob.centroide.x = moments.m10 / moments.m00;
		blob.centroide.y = moments.m01 / moments.m00;		
		if ( countour->total >= 6 )
		{
			blob.ellipse = cvFitEllipse2( countour );
		}
		
		m_blobs.push_back( blob );

		//cvDrawContours( m_binaryRedraw, first, cvScalar(255,255,255), cvScalar(255,0,0), -1, CV_FILLED, 8 );
	}	
	cvClearMemStorage( m_storage );
}

void GLWindow::sltMinThreshChanged( int value )
{
	m_minThreshold = value;
	processImage();
}

void GLWindow::sltMaxThreshChanged( int value )
{
	m_maxThreshold = value;
	processImage();
}

void GLWindow::sltInvertThreshChanged( int state )
{
	m_invertThresh = state;
	processImage();
}