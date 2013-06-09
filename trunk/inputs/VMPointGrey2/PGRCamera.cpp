#include "PGRCamera.h"
#include <sstream>
#include <iostream>
#include "Error.h"
#include "BusManager.h"

using namespace FlyCapture2;
using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

#define INITIALIZE         0x000
#define CAMERA_POWER       0x610

#define CURRENT_VIDEO_FORMAT  0x608
#define CURRENT_VIDEO_MODE    0x604

#define IMAGE_POSITION	      0x008
#define IMAGE_SIZE			0x00C
#define COLOR_CODING		0x010
#define PACKET_PARA_INQ	      0x040
#define BYTE_PER_PACKET	      0x044
#define VALUE_SETTING	      0x07c
#define REG_FRAME_RATE	      0x83C

#define MAX_IMAGE_SIZE_INQ 0x000
#define UNIT_SIZE_INQ 0x004
#define UNIT_POSITION_INQ 0x04c

#define PIN_DIRECTION 0x11F8
#define GPIO_STRPAT_CTRL 0x110C
#define GPIO_CTRL_PIN_0 0x1110
#define GPIO_CTRL_PIN_1 0x1120
#define GPIO_CTRL_PIN_2 0x1130
#define GPIO_CTRL_PIN_3 0x1140
#define GPIO_XTRA_PIN_2 0x1134
#define GPIO_XTRA_PIN_3 0x1144
#define GPIO_STRPAT_MASK_PIN_2 0x1138
#define GPIO_STRPAT_MASK_PIN_3 0x1148
#define STROBE_0_CNT 0x1500
#define STROBE_1_CNT 0x1504

unsigned long videoModeBase[] ={0xA00,0xA80,0xB00,0xB80,0xC00,0xC80};//(pag47)*/

void copyStringToChar( const std::string &src, char **dst )
{
	if ( src.length() > 0 )
	{
		*dst = new char[src.length() + 1];
		strcpy( *dst, src.c_str() );
	}
	else
		*dst = NULL;
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void PrintError( Error error )
{
    printf( "%s\n", error.GetDescription() );
}

PGRCamera::PGRCamera(void)
{
	//recording = false;
	//started = false;
	//convertForTheRenderer = false;
	//imageConverted.pData = NULL;
	serialNumber = 0;
	//act = 0;
	//showControlDlgThread = NULL;	
	lastImage = NULL;	
	dlg = NULL;
}


PGRCamera::~PGRCamera(void)
{
	//Close showControlDlgThread
	//
	//DWORD exitCode = 1;	
	//if ( showControlDlgThread )
	//{
	//	GetExitCodeThread( showControlDlgThread, &exitCode);
	//	TerminateThread( showControlDlgThread, exitCode);		
	//	CloseHandle( showControlDlgThread );
	//	showControlDlgThread = NULL;
	//}
	//Power off
	cam.WriteRegister( CAMERA_POWER, 0x00000000 );
	if ( dlg )
	{
		dlg->Hide();
		dlg->Disconnect();
		delete dlg;
	}

	// Stop capturing images
    stop();
    // Disconnect the camera
	
	Error error = cam.Disconnect();
    if (error != PGRERROR_OK)
        PrintError( error );

//	recording = false;
	rawImage.ReleaseBuffer();
	convertedImage.ReleaseBuffer();	
}

FlyCapture2::FrameRate PGRCamera::buildFrameRate( double fps )
{
	if ( fps == 1.875 )
		return FRAMERATE_1_875;
	else if ( fps == 3.75 )
		return FRAMERATE_3_75;
	else if ( fps == 7.5 )
		return FRAMERATE_7_5;
	else if	( fps == 15.0 )
		return FRAMERATE_15;
	else if ( fps == 30.0 )
		return FRAMERATE_30;
	else if ( fps == 60.0 )
		return FRAMERATE_60;
	else if ( fps == 120.0 )
		return FRAMERATE_120;
	else if ( fps == 240.0 )
		return FRAMERATE_240;
	else
		return FRAMERATE_FORMAT7;
}

FlyCapture2::VideoMode PGRCamera::buildVideoMode( VMInputFormat format )
{	
	if ( format.width == 1600 && format.height == 1200 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:			
				return VIDEOMODE_1600x1200RGB;								
			case VM_GREY16:			
				return VIDEOMODE_1600x1200Y16;				
			case VM_GREY8:
				return VIDEOMODE_1600x1200Y8;				
			case VM_YUV422:
				return VIDEOMODE_1600x1200YUV422;				
		}
	}	
	else if ( format.width == 1280 && format.height == 960 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return VIDEOMODE_1280x960RGB;								
			case VM_GREY16:			
				return VIDEOMODE_1280x960Y16;				
			case VM_GREY8:
				return VIDEOMODE_1280x960Y8;				
			case VM_YUV422:
				return VIDEOMODE_1280x960YUV422;							
				
		}
	}
	else if ( format.width == 1024 && format.height == 768 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return VIDEOMODE_1024x768RGB;								
			case VM_GREY16:			
				return VIDEOMODE_1024x768Y16;				
			case VM_GREY8:
				return VIDEOMODE_1024x768Y8;				
			case VM_YUV422:
				return VIDEOMODE_1024x768YUV422;											
		}
	}
	else if ( format.width == 800 && format.height == 600 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return VIDEOMODE_800x600RGB;								
			case VM_GREY16:			
				return VIDEOMODE_800x600Y16;				
			case VM_GREY8:
				return VIDEOMODE_800x600Y8;				
			case VM_YUV422:
				return VIDEOMODE_800x600YUV422;				
		}
	}	
	else if ( format.width == 640 && format.height == 480 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return VIDEOMODE_640x480RGB;				
			case VM_GREY16:
				return VIDEOMODE_640x480Y16;				
			case VM_GREY8:
				return VIDEOMODE_640x480Y8;				
			case VM_YUV422:
				return VIDEOMODE_640x480YUV422;				
			case VM_YUV411:
				return VIDEOMODE_640x480YUV411;			
		}
	}
	else if ( format.width == 320 && format.height == 240 )
	{
		return VIDEOMODE_320x240YUV422;
	}
	else if ( format.width == 160 && format.height == 120 )
	{
		return VIDEOMODE_160x120YUV444;
	}
	return VIDEOMODE_FORMAT7;
}
void PGRCamera::getFirstPixelFormatSupported(const FlyCapture2::Format7Info &fmt7Info,FlyCapture2::PixelFormat &fmt7PixFmt)
{
	if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_RGB)==PIXEL_FORMAT_RGB) fmt7PixFmt=PIXEL_FORMAT_RGB;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_RGBU) == PIXEL_FORMAT_RGBU) fmt7PixFmt=PIXEL_FORMAT_RGBU;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_BGR )==PIXEL_FORMAT_BGR) fmt7PixFmt=PIXEL_FORMAT_BGR;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_BGRU) == PIXEL_FORMAT_BGRU ) fmt7PixFmt=PIXEL_FORMAT_BGRU;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_422YUV8) == PIXEL_FORMAT_422YUV8) fmt7PixFmt=PIXEL_FORMAT_422YUV8;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_411YUV8) == PIXEL_FORMAT_411YUV8) fmt7PixFmt=PIXEL_FORMAT_411YUV8;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_MONO8) == PIXEL_FORMAT_MONO8) fmt7PixFmt=PIXEL_FORMAT_MONO8;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_MONO16) == PIXEL_FORMAT_MONO16) fmt7PixFmt=PIXEL_FORMAT_MONO16;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_RAW8) == PIXEL_FORMAT_RAW8) fmt7PixFmt=PIXEL_FORMAT_RAW8;
	else if ((fmt7Info.pixelFormatBitField & PIXEL_FORMAT_RAW16) == PIXEL_FORMAT_RAW16) fmt7PixFmt=PIXEL_FORMAT_RAW16;

}

FlyCapture2::PixelFormat PGRCamera::buildPixelFormat( VMPixelFormat pixelFormat )
{
	switch( pixelFormat )
	{
		case VM_RGB24:
			return PIXEL_FORMAT_RGB;
		case VM_RGB32:
			return PIXEL_FORMAT_RGBU;
		case VM_BGR24:
			return PIXEL_FORMAT_BGR;
		case VM_BGR32:
			return PIXEL_FORMAT_BGRU;
		case VM_YUV422:
			return PIXEL_FORMAT_422YUV8;
		case VM_YUV411:
			return PIXEL_FORMAT_411YUV8;
		case VM_GREY8:
			return PIXEL_FORMAT_MONO8;
		case VM_GREY16:
			return PIXEL_FORMAT_MONO16;
		case VM_RAW8:
			return PIXEL_FORMAT_RAW8;
		case VM_RAW16:
			return PIXEL_FORMAT_RAW16;
	}
	return PIXEL_FORMAT_RGB;		
	/*PIXEL_FORMAT_RGB16
	PIXEL_FORMAT_S_MONO16
	PIXEL_FORMAT_S_RGB16
	PIXEL_FORMAT_RAW16
	PIXEL_FORMAT_MONO12
	PIXEL_FORMAT_RAW12
	PIXEL_FORMAT_BGR16
	PIXEL_FORMAT_BGRU16
	PIXEL_FORMAT_422YUV8_JPEG*/
}


void frameCallback( Image* pImage, const void* pCallbackData )
{
	PGRCamera *pgrCamerap = (PGRCamera*)pCallbackData;
	// Convert the raw image
	/*Error error = pImage->Convert( PIXEL_FORMAT_MONO8, &pgrCamerap->convertedImage );
	//error = convertedImage.Save( "C:/kk.pgm" );
	if (error != PGRERROR_OK)
	{
		PrintError( error );

		return;

	}

	//convertedImage.DeepCopy( &convertedImageCopy );

	pgrCamerap->pixelBuffer = (char*)pgrCamerap->convertedImage.GetData();	

	pgrCamerap->lastImage = pImage;

	if( pgrCamerap->callback )

		(*pgrCamerap->callback)( pgrCamerap->pixelBuffer );*/
	if ( pgrCamerap->pixelBuffer == NULL )
	{
		pgrCamerap->pixelBuffer = (char*)pImage->GetData();	
		pgrCamerap->lastImage = pImage;
		
		if( pgrCamerap->callback )
			(*pgrCamerap->callback)( pgrCamerap->pixelBuffer, pgrCamerap->inputID, (double)pImage->GetTimeStamp().seconds, pgrCamerap->frameCallbackData );			
	}
}

void PGRCamera::setFrameCallback( getFrameCallback theCallback, void *data )
{
	callback = theCallback ;
	frameCallbackData = data;
	stop();
	Error error = cam.StartCapture( frameCallback, this );
	if (error != PGRERROR_OK)
		PrintError( error );
	//cam.SetCallback( frameCallback, this );
}

bool PGRCamera::resetCamera( )
{
	Error error = cam.WriteRegister( INITIALIZE,0x80000000 );
	return ( error != PGRERROR_OK );
}

bool PGRCamera::initCamera( unsigned long aSerialNumber, VMInputFormat *aFormat )
{
	serialNumber = aSerialNumber;

	if ( aSerialNumber == 0 )
	{
		PGRGuid guid[64];
		unsigned int psize = 64;
		CameraSelectionDlg selectionDlg;
		bool ok;
		selectionDlg.ShowModal( &ok, guid, &psize );
		if ( !ok || psize != 1 ) 
			return false;
		Error error = cam.Connect(&guid[0]);
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}
	}
	else
	{
		Error error;
		BusManager busMgr;
		PGRGuid guid;
		error = busMgr.GetCameraFromSerialNumber( aSerialNumber, &guid );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}

		error = cam.Connect(&guid);
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}
	}

	resetCamera();
	//cam.WriteRegister( CAMERA_POWER, 0x80000000 );
	//setTrigger( false, 3, 14 );

	if ( aFormat == NULL || aFormat->showDlg )
	{
		// Show the camera configuration dialog.		
		CameraControlDlg controlDlg;
		controlDlg.Connect( &cam );
		controlDlg.ShowModal();
		controlDlg.Hide();
		controlDlg.Disconnect();
		//Initialize the format		
		cam.GetVideoModeAndFrameRate( &m_videoMode, &m_frameRate );
		bool availableFormat = resolveFormat( m_videoMode, m_frameRate, format );	
		if ( !availableFormat )
			return false;
	}
	else if ( aFormat != NULL )
	{
		m_videoMode = buildVideoMode( *aFormat );		
		m_frameRate = buildFrameRate( aFormat->fps );
		bool supported = false;
		if ( m_videoMode != VIDEOMODE_FORMAT7 && m_frameRate != FRAMERATE_FORMAT7 ) 
		{
			//Check if the combination videomode and framerate is supported
			Error error = cam.GetVideoModeAndFrameRateInfo( m_videoMode, m_frameRate, &supported );
			if ( error != PGRERROR_OK )
			{
				PrintError( error );
				return false;
			}
		}
		if ( supported )
		{
			//Set standard videomode and framerate						
			Error error = cam.SetVideoModeAndFrameRate( m_videoMode, m_frameRate );
			if (error != PGRERROR_OK)
			{
				PrintError( error );
				return false;
			}
			//Initialize the format member
			bool availableFormat = resolveFormat( m_videoMode, m_frameRate, format );	
			if ( !availableFormat )
				return false;
		}
		else		
		{
			PixelFormat fmt7PixFmt = buildPixelFormat( aFormat->getPixelFormatOut() );
			// Find the Format 7 mode that matches aFormat
			// Query for available Format 7 modes			
			bool found = false;
			for ( int currentMode = 0; !found && currentMode < NUM_MODES; ++currentMode )
			{
				m_fmt7Info.mode = static_cast<Mode>( MODE_0 + currentMode );
				bool supported;			
				Error error = cam.GetFormat7Info( &m_fmt7Info, &supported );
				if (error != PGRERROR_OK)
				{
					PrintError( error );
					return false;
				}
				if ( supported && m_fmt7Info.maxWidth == aFormat->width && m_fmt7Info.maxHeight == aFormat->height &&
					( aFormat->getPixelFormatOut() == VM_UNKNOWN || ( ( m_fmt7Info.pixelFormatBitField & fmt7PixFmt ) == fmt7PixFmt ) ) )
				{
					//same resolution and pixelFormat
					found = true;
				}
			}
			if ( found )
			{
				if (aFormat->getPixelFormatOut() == VM_UNKNOWN ) 
					getFirstPixelFormatSupported(m_fmt7Info,fmt7PixFmt);

				format.SetFormat( m_fmt7Info.maxWidth, m_fmt7Info.maxHeight, aFormat->fps, resolvePixelFormat( fmt7PixFmt ), resolvePixelFormat( fmt7PixFmt ) );
				m_fmt7ImageSettings.mode = m_fmt7Info.mode;
				m_fmt7ImageSettings.offsetX = 0;
				m_fmt7ImageSettings.offsetY = 0;
				m_fmt7ImageSettings.width = m_fmt7Info.maxWidth;
				m_fmt7ImageSettings.height = m_fmt7Info.maxHeight;
				m_fmt7ImageSettings.pixelFormat = fmt7PixFmt;

				// Validate the settings to make sure that they are valid
				bool valid;
				Error error = cam.ValidateFormat7Settings( &m_fmt7ImageSettings, &valid, &m_fmt7PacketInfo );
				if (error != PGRERROR_OK)
				{
					PrintError( error );
					return false;
				}
				if ( !valid )
				{
					// Settings are not valid
					printf("Format7 settings are not valid\n");
					return false;
				}

				// Set the settings to the camera
				error = cam.SetFormat7Configuration( &m_fmt7ImageSettings, m_fmt7PacketInfo.recommendedBytesPerPacket );
				if (error != PGRERROR_OK)
				{
					PrintError( error );
					return false;
				}

				setFrameRate( aFormat->fps );
			}
			else
			{
				printf("Format7 settings are not valid\n");
				return false;
			}
		}		
	}

	//Check Image Size and Sensor offset
	/*unsigned int roiSize;
	cam.ReadRegister( 0xa0c, &roiSize );
	width = roiSize >> 16;
	//Offest de la imagen dentro del sensor
	height = roiSize & 0x0000FFFF;
	unsigned int roiPos;
	cam.ReadRegister( 0xa08, &roiPos );
	xOffset = roiPos >> 16;
	yOffset = roiPos & 0x0000FFFF;*/

    // Start capturing images
	Error error = cam.StartCapture(); //Start Isochronous image capture
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }

	// Retrieve frame rate property
	Property frmRate;
	frmRate.type = FRAME_RATE;
	error = cam.GetProperty( &frmRate );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return false;
	}				
	format.fps = frmRate.absValue;

	//Check the video mode and initialize the format	
	/*bool availableFormat = resolveFormat( videoMode, frameRate, format );	
	if ( !availableFormat )
		return false;*/
	
	//Change the user's format
	if ( aFormat != NULL )	
		*aFormat = format; 
	
	// Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }
	m_bayerTileFormat = camInfo.bayerTileFormat;

	ostringstream ss;
	ss << aSerialNumber;
	copyStringToChar( ss.str(), &identification.uniqueName );
	copyStringToChar( "PGR_CAMERA2", &identification.identifier );
	copyStringToChar( camInfo.modelName, &identification.friendlyName );
	return true;
}

void PGRCamera::releaseFrame()
{
	rawImage.ReleaseBuffer();
	lastImage = NULL;
	pixelBuffer = NULL;	
}

inline char *PGRCamera::getFrame( bool wait )
{
	//return pixelBuffer;
	Error error = cam.RetrieveBuffer( &rawImage );
	
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return NULL;
    }

    // Convert the raw image
    /*error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage );
	//error = convertedImage.Save( "C:/kk.pgm" );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		return NULL;
    }*/
	//convertedImage.DeepCopy( &convertedImageCopy );
	pixelBuffer = (char*)rawImage.GetData();
	return pixelBuffer;
}

/*template<class ClassType>
unsigned long WINAPI showPropertyPageThread(void *parameters)
{
	ClassType *device = static_cast<ClassType*>(parameters);
	device->Show();
	return 0;
}*/

void PGRCamera::showPropertyPage()
{
	//DWORD dwThreadID;
	//showControlDlgThread = CreateThread( NULL, 0, showPropertyPageThread<CameraControlDlg>, (LPVOID)dlg, 0, &dwThreadID );
	if ( !dlg )
	{
		dlg = new CameraControlDlg();
		dlg->Connect( &cam );
		
	}
	dlg->Show();
}

VideoManInputController *PGRCamera::getController()
{
	return (VideoManInputController*)( controller );
}

bool PGRCamera::linkController( VideoManInputController *acontroller )
{
	if ( acontroller != NULL && acontroller->setInput(this) )
	{
		controller = acontroller;
		return true;
	}
	return false;
}


int PGRCamera::getNumberOfLostFrames()
{
	//return lostFrames;
	return 0;
}

VMPixelFormat PGRCamera::resolvePixelFormat( FlyCapture2::PixelFormat pFormat )
{
	switch( pFormat )
	{
		case PIXEL_FORMAT_MONO8:
			return VM_GREY8;
		case PIXEL_FORMAT_RAW8:
			return VM_RAW8;
		case PIXEL_FORMAT_411YUV8:
			return VM_YUV411;
		case PIXEL_FORMAT_422YUV8:
			return VM_YUV422;
		//case PIXEL_FORMAT_444YUV8:
			//return VM_YUV444;
		case PIXEL_FORMAT_MONO16:
		case PIXEL_FORMAT_S_MONO16:
			return VM_GREY16;
		case PIXEL_FORMAT_RAW16:
			return VM_RAW16;
		case PIXEL_FORMAT_RGB8:		
			return VM_RGB24;
		case PIXEL_FORMAT_BGR:
			return VM_BGR24;
		case PIXEL_FORMAT_RGBU:		
			return VM_RGB32;
		case PIXEL_FORMAT_BGRU:
			return VM_BGR32;
	}
	return VM_UNKNOWN;
}


bool PGRCamera::resolveFormat( VideoMode videoMode, FrameRate frameRate, VMInputFormat &rFormat )
{
	switch ( videoMode )
	{
		case VIDEOMODE_1600x1200YUV422:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case VIDEOMODE_1600x1200RGB:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case VIDEOMODE_1600x1200Y8:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case VIDEOMODE_1600x1200Y16:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case VIDEOMODE_1280x960YUV422:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case VIDEOMODE_1280x960RGB:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case VIDEOMODE_1280x960Y8:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case VIDEOMODE_1280x960Y16:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_GREY16, VM_GREY16 );
			break;
		}			
		case VIDEOMODE_1024x768Y8:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case VIDEOMODE_1024x768Y16:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case VIDEOMODE_1024x768YUV422:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case VIDEOMODE_1024x768RGB:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case VIDEOMODE_800x600Y8:
		{
			rFormat.SetFormat( 800, 600, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case VIDEOMODE_800x600Y16:
		{
			rFormat.SetFormat( 800, 600, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case VIDEOMODE_800x600YUV422:
		{
			rFormat.SetFormat( 800, 600, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case VIDEOMODE_800x600RGB:
		{
			rFormat.SetFormat( 800, 600, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case VIDEOMODE_640x480RGB:
		{
			rFormat.SetFormat( 640, 480, 30, VM_RGB24, VM_RGB24 );			
			break;
		}
		case VIDEOMODE_640x480YUV422:
		{
			rFormat.SetFormat( 640, 480, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case VIDEOMODE_640x480YUV411:
		{
			rFormat.SetFormat( 640, 480, 30, VM_YUV411, VM_YUV411 );
			break;
		}
		case VIDEOMODE_640x480Y16:
		{
			rFormat.SetFormat( 640, 480, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case VIDEOMODE_640x480Y8:
		{
			rFormat.SetFormat( 640, 480, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case VIDEOMODE_320x240YUV422:
		{
			rFormat.SetFormat( 320, 240, 30, VM_YUV422, VM_YUV422 );
			break;
		}		
		case VIDEOMODE_FORMAT7:
		{
			//Get the format 7 configuration
			unsigned int pPacketSize;
			float pPercentage;
			Error error = cam.GetFormat7Configuration( &m_fmt7ImageSettings, &pPacketSize, &pPercentage );
			if ( error != PGRERROR_OK )
			{
				PrintError( error );
				return false;
			}
			//Get the format 7 info
			m_fmt7Info.mode = m_fmt7ImageSettings.mode;
			bool supported;			
			error = cam.GetFormat7Info( &m_fmt7Info, &supported );
			if (error != PGRERROR_OK)
			{
				PrintError( error );
				return false;
			}
			if ( m_fmt7ImageSettings.width != m_fmt7Info.maxWidth || m_fmt7ImageSettings.height != m_fmt7Info.maxHeight )
			{
				roiMode = true;
				xRoi = m_fmt7ImageSettings.offsetX;
				yRoi = m_fmt7ImageSettings.offsetY;
				wRoi = m_fmt7ImageSettings.width;
				hRoi = m_fmt7ImageSettings.height;
			}
			rFormat.SetFormat( m_fmt7Info.maxWidth, m_fmt7Info.maxHeight, 30, resolvePixelFormat( m_fmt7ImageSettings.pixelFormat ), resolvePixelFormat( m_fmt7ImageSettings.pixelFormat ) );
			break;
		}
		default:
		{
			return false;		
		}
	}
	switch ( frameRate )
	{
		case FRAMERATE_1_875:
		{
			rFormat.fps = 1.875f;
			break;
		}
		case FRAMERATE_3_75:
		{
			rFormat.fps = 3.75f;
			break;
		}
		case FRAMERATE_7_5:
		{
			rFormat.fps = 7.5f;
			break;
		}
		case FRAMERATE_15:
		{
			rFormat.fps = 15.0f;
			break;
		}
		case FRAMERATE_30:
		{
			rFormat.fps = 30.0f;
			break;
		}
		case FRAMERATE_60:
		{
			rFormat.fps = 60.0f;
			break;
		}
		case FRAMERATE_120:
		{
			rFormat.fps = 120.0f;
			break;
		}
		case FRAMERATE_240:
		{
			rFormat.fps = 240.0f;
			break;
		}
		case FRAMERATE_FORMAT7:
		{
			Property frmRate;
			frmRate.type = FRAME_RATE;
			Error error = cam.GetProperty( &frmRate );
			if (error != PGRERROR_OK)
			{
				PrintError( error );
				return false;
			}
			rFormat.fps = frmRate.absValue;
			break;
		}
		default:
			return false;
	}
	return true;
}

void PGRCamera::getAvailableDevices(  VMInputIdentification **deviceList, int &numDevices )
{	
	numDevices = 0;
	*deviceList = NULL;

	BusManager busMgr;
	unsigned int numCameras;	
	Error error = busMgr.GetNumOfCameras(&numCameras);
	if ( error != PGRERROR_OK )
	{
		PrintError( error );
		return;
	}
	if ( numCameras == 0 )
		return;
	
	numDevices = numCameras;
	*deviceList = new VMInputIdentification[numCameras];
	for (unsigned int i=0; i < numCameras; i++)
    {
		VMInputIdentification device;

        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return;
        }

		/*unsigned int serial;
		error = busMgr.GetCameraSerialNumberFromIndex( i, &serial );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return;
		}*/

		Camera cam;
		error = cam.Connect(&guid);
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return;
		}

		 // Get the camera information
		CameraInfo camInfo;	
		error = cam.GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return;
		}			
		
		ostringstream ss;
		ss << camInfo.serialNumber;
		copyStringToChar( ss.str(), &device.uniqueName );
		copyStringToChar( "PGR_CAMERA2", &device.identifier );
		copyStringToChar( camInfo.modelName, &device.friendlyName );
		(*deviceList)[i] = device;		
		error = cam.Disconnect();
		if (error != PGRERROR_OK)
			PrintError( error );
    }
}

bool PGRCamera::setImageROI( int x, int y, int width, int height )
{	
	if ( m_videoMode == VIDEOMODE_FORMAT7 )
	{
		Error error = cam.StopCapture();
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}
		m_fmt7ImageSettings.offsetX = x;
		m_fmt7ImageSettings.offsetY = y;
		m_fmt7ImageSettings.width = width;
		m_fmt7ImageSettings.height = height;
		// Validate the settings to make sure that they are valid
		bool valid;
		error = cam.ValidateFormat7Settings( &m_fmt7ImageSettings, &valid, &m_fmt7PacketInfo );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}
		if ( !valid )
		{
			// Settings are not valid
			printf("Format7 settings are not valid\n");
			return false;
		}
		error = cam.SetFormat7Configuration( &m_fmt7ImageSettings, m_fmt7PacketInfo.recommendedBytesPerPacket );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
		}

		if ( m_fmt7ImageSettings.width != m_fmt7Info.maxWidth || m_fmt7ImageSettings.height != m_fmt7Info.maxHeight )
		{
			roiMode = true;
			xRoi = x;
			yRoi = y;
			wRoi = width;
			hRoi = height;
		}
		else
			roiMode = false;

		//Restart the camera
		if ( callback )
			error = cam.StartCapture( frameCallback, this );
		else
			error = cam.StartCapture();
	    if (error != PGRERROR_OK)
	    {
			PrintError( error );
			return false;
		}
		return true;
	}
	cout << "Camera is not initialized with Format 7" << endl;
	return false;
}

bool PGRCamera::setGainControl( bool autoGain )
{
	Property prop;
	prop.type = GAIN;
	cam.GetProperty( &prop );
	prop.autoManualMode = autoGain;
	//prop.valueA = 400;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}

bool PGRCamera::getGainControl()
{
	Property prop;
	prop.type = GAIN;
	cam.GetProperty( &prop );
	return prop.autoManualMode;
}

bool PGRCamera::setExposureControl( bool autoExp )
{
	Property prop;
	prop.type = AUTO_EXPOSURE;
	cam.GetProperty( &prop );
	prop.autoManualMode = autoExp;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}

bool PGRCamera::setSharpnessControlValue(float value)
{
	Property prop;
	prop.type = SHARPNESS;
	cam.GetProperty( &prop );
	prop.autoManualMode = false;
	prop.valueA = (unsigned int)value;	
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}

bool PGRCamera::setSharpnessControl( bool autoSharp )
{
	Property prop;
	prop.type = SHARPNESS;
	cam.GetProperty( &prop );
	prop.autoManualMode = autoSharp;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}


bool PGRCamera::setShutterControl( bool autoShutter )
{
	Property prop;
	prop.type = SHUTTER;
	cam.GetProperty( &prop );
	prop.autoManualMode = autoShutter;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}

bool PGRCamera::setShutterTime( float shutterTime )
{
	Property prop;
	prop.type = SHUTTER;
	cam.GetProperty( &prop );
	prop.autoManualMode = false;
	prop.absControl = true;
	prop.absValue = shutterTime;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}

bool PGRCamera::moveImageROI( int x, int y )
{
	unsigned long position = 0; 
	position |= x + m_fmt7Info.imageHStepSize;
	position = ( position << 16 ) | y + m_fmt7Info.imageVStepSize;
	Error error = cam.WriteRegister( videoModeBase[m_fmt7Info.mode] + IMAGE_POSITION, position );
	return ( error == PGRERROR_OK );
}

unsigned int PGRCamera::getRegisterValue(unsigned long _register )
{
	unsigned int value;
	cam.ReadRegister( _register, &value );	
	return value;
}

bool PGRCamera::getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit )
{
	if ( m_videoMode == VIDEOMODE_FORMAT7 )
	{
		Hmax = m_fmt7Info.maxWidth;
		Vmax = m_fmt7Info.maxHeight;
		Hunit = m_fmt7Info.imageHStepSize;
		Vunit = m_fmt7Info.imageVStepSize;
		HPosUnit = m_fmt7Info.offsetHStepSize;
		VPosUnit = m_fmt7Info.offsetVStepSize;
		return true;
	}
	return false;
}

bool PGRCamera::resetImageROI()
{
	return setImageROI( 0, 0, m_fmt7Info.maxWidth, m_fmt7Info.maxHeight );
}

int PGRCamera::buildColorCoding( VMPixelFormat pixelFormat )
{
	switch( pixelFormat )
	{
	case VM_GREY8:
		return 0;
	case VM_YUV411:
		return 1;
	case VM_YUV422:
		return 2;
	//case 3:
	//	return VM_YUV444;
	case VM_RGB24:
		return 4;
	case VM_GREY16:
		return 5;
	case VM_RAW8:
		return 9;
	default:
		return -1;
	}
}

bool PGRCamera::setTrigger( bool triggerOn, int source, int mode )
{
	TriggerMode triger;
	cam.GetTriggerMode( &triger );	
	triger.onOff = triggerOn;
	if ( triggerOn )
	{
		triger.source = source;
		triger.mode = mode;
	}
	Error error = cam.SetTriggerMode( &triger );
	if ( error != PGRERROR_OK )
	{
		PrintError( error );
		return false;
	}
	return true;
}

bool PGRCamera::fireSoftwareTrigger( bool broadcast )
{
	return ( cam.FireSoftwareTrigger( broadcast ) == PGRERROR_OK );
}

bool PGRCamera::setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source )
{
	StrobeControl strobeControl;
	strobeControl.source = source;	
	strobeControl.delay = delay;
	strobeControl.duration = duration;
	strobeControl.onOff = onOff;	
	strobeControl.polarity = polarity;
	error = cam.SetStrobe( &strobeControl );
	if ( error != PGRERROR_OK )
	{
		PrintError( error );
		return false;
	}
	return true;	
}

void PGRCamera::stop()
{
	Error error = cam.StopCapture();
    if (error != PGRERROR_OK)           
        PrintError( error );
}

bool PGRCamera::setFrameRate( double frameRate )
{
	Property prop;
	prop.type = FRAME_RATE;
	cam.GetProperty( &prop );
	prop.autoManualMode = false;
	prop.absControl = true;
	prop.onOff = true;
	prop.absValue = (float)frameRate;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}