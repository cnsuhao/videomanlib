#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
//#include "StdAfx.h"
#include "PGRCamera.h"
//#include "pgrerror.h"
#include <sstream>
#include <iostream>

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
#define FRAME_RATE	      0x83C

#define MAX_IMAGE_SIZE_INQ 0x000
#define UNIT_SIZE_INQ 0x004
#define UNIT_POSITION_INQ 0x04c

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

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

PGRCamera::PGRCamera(void)
{
	controller = NULL;
	context = NULL;
	guiContext = NULL;

	recording = false;
	started = false;

	convertForTheRenderer = false;
	imageConverted.pData = NULL;

	serialNumber = 0;
}


PGRCamera::~PGRCamera(void)
{
	FlyCaptureError capError;
	//Power off
	setRegister( CAMERA_POWER, 0x00000000 );
	if ( started )
	{
		capError = flycaptureStop( context );
		checkCaptureError( capError, "flycaptureStop()" );
	}
	started = false;	

	capError = flycaptureDestroyContext( context );
	checkCaptureError( capError, "flycaptureDestroyContext()" );

	if ( imageConverted.pData != NULL )
	{
		delete [] imageConverted.pData;
		imageConverted.pData = NULL; 
	}

	pgrcamguiDestroyContext( guiContext );

	recording = false;
}

bool PGRCamera::checkCaptureError( FlyCaptureError error, std::string message )
{
	if( error != FLYCAPTURE_OK )
	{
		std::string textError = message;
		textError = textError + flycaptureErrorToString( error );
		cout << textError.c_str() << endl;;
		return false;
	}
	return true;
}


bool PGRCamera::checkGuiError( CameraGUIError error, std::string message )
{
	if( error != PGRCAMGUI_OK )
	{
		cout << message.c_str() << endl;
		return false;
	}
	return true;
}


FlyCaptureVideoMode PGRCamera::buildVideoMode( VMInputFormat format )
{	
	if ( format.width == 1600 && format.height == 1200 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:			
				return FLYCAPTURE_VIDEOMODE_1600x1200RGB;								
			case VM_GREY16:			
				return FLYCAPTURE_VIDEOMODE_1600x1200Y16;				
			case VM_GREY8:
				return FLYCAPTURE_VIDEOMODE_1600x1200Y8;				
			case VM_YUV422:
				return FLYCAPTURE_VIDEOMODE_1600x1200YUV422;				
		}
	}	
	else if ( format.width == 1280 && format.height == 960 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return FLYCAPTURE_VIDEOMODE_1280x960RGB;								
			case VM_GREY16:			
				return FLYCAPTURE_VIDEOMODE_1280x960Y16;				
			case VM_GREY8:
				return FLYCAPTURE_VIDEOMODE_1280x960Y8;				
			case VM_YUV422:
				return FLYCAPTURE_VIDEOMODE_1280x960YUV422;							
				
		}
	}
	else if ( format.width == 1024 && format.height == 768 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return FLYCAPTURE_VIDEOMODE_1024x768RGB;								
			case VM_GREY16:			
				return FLYCAPTURE_VIDEOMODE_1024x768Y16;				
			case VM_GREY8:
				return FLYCAPTURE_VIDEOMODE_1024x768Y8;				
			case VM_YUV422:
				return FLYCAPTURE_VIDEOMODE_1024x768YUV422;											
		}
	}
	else if ( format.width == 800 && format.height == 600 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return FLYCAPTURE_VIDEOMODE_800x600RGB;								
			case VM_GREY16:			
				return FLYCAPTURE_VIDEOMODE_800x600Y16;				
			case VM_GREY8:
				return FLYCAPTURE_VIDEOMODE_800x600Y8;				
			case VM_YUV422:
				return FLYCAPTURE_VIDEOMODE_800x600YUV422;				
		}
	}	
	
	if ( format.width == 640 && format.height == 480 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case VM_RGB24:
				return FLYCAPTURE_VIDEOMODE_640x480RGB;				
			case VM_GREY16:
				return FLYCAPTURE_VIDEOMODE_640x480Y16;				
			case VM_GREY8:
				return FLYCAPTURE_VIDEOMODE_640x480Y8;				
			case VM_YUV422:
				return FLYCAPTURE_VIDEOMODE_640x480YUV422;				
			case VM_YUV411:
				return FLYCAPTURE_VIDEOMODE_640x480YUV411;			
		}
	}
	else if ( format.width == 320 && format.height == 240 )
	{
		return FLYCAPTURE_VIDEOMODE_320x240YUV422;
	}
	return FLYCAPTURE_VIDEOMODE_ANY;
}

bool PGRCamera::initCamera( unsigned long aSerialNumber, VMInputFormat *aFormat )
{	
	FlyCaptureError capError;
	CameraGUIError guiError;

	capError = flycaptureCreateContext( &context );
	if ( !checkCaptureError( capError, "flycaptureCreateContext()" ) )	
		return false;

	guiError = pgrcamguiCreateContext( &guiContext );	
	if ( !checkGuiError( guiError, "pgrcamguiCreateContext() Error creating GUI context." ) )
		return false;

	//format = *aFormat;

	serialNumber = aSerialNumber;
/*	// Show the camera selection dialog.		
	if ( aFormat == NULL || aFormat->showDlg )
	{
		int		iDialogStatus;
		//The user must select the camera
		guiError = ::pgrcamguiShowCameraSelectionModal(	guiContext, context, &serialNumber,	&iDialogStatus );
		if ( !checkGuiError( guiError, "pgrcamguiShowCameraSelectionModal() Error showing camera selection dialog." ) || serialNumber == 0 )
			return false;
	}*/
	
	//Check the bus index
	FlyCaptureInfoEx  arInfo[ 32 ];
	unsigned int	     uiSize = 32;
	FlyCaptureError error = flycaptureBusEnumerateCamerasEx( arInfo, &uiSize );
	if ( !checkCaptureError( error, "flycaptureBusEnumerateCamerasEx()" ) )	
		return false;
	if ( uiSize == 0 )
		return false;
	uiBusIndex = 0;
	bool found = false;
	while ( uiBusIndex < uiSize && !found)
	{
		FlyCaptureInfoEx* pinfo = &arInfo[ uiBusIndex ];
		if ( pinfo->SerialNumber == serialNumber )
		{
			found = true;
		}
		else
			uiBusIndex++;
	}
	
    if ( !found )		
		return false;

	FlyCaptureInfoEx* pinfo = &arInfo[ uiBusIndex ];
	//serialNumber = pinfo->SerialNumber;*/

	/*capError = flycaptureInitializeFromSerialNumber( context, serialNumber );
	if ( !checkCaptureError( capError, "flycaptureInitializeFromSerialNumber()" ) )
		return false;*/
	//serialNumber = aSerialNumber;
	/*initBuffers( *aFormat ); //¿?¿?¿?¿?¿?
	capError = flycaptureInitializePlus( context, uiBusIndex, BUFFERS_NUM, NULL );*/
	capError = flycaptureInitializeFromSerialNumber( context, serialNumber );
	if ( !checkCaptureError( capError, "flycaptureInitializePlus()" ) )
		return false;

	guiError = pgrcamguiInitializeSettingsDialog( guiContext, (GenericCameraContext) context );
	if ( !checkGuiError( guiError, "pgrcamguiInitializeSettingsDialog() Error creating settings dialog." ) )
		return false;

	//reset the camera to default factory settings
	setRegister( INITIALIZE, 0x80000000 );

	// Power-up the camera
	setRegister( CAMERA_POWER, 0x80000000 );

	int		iDialogStatus;
		//The user must select the camera
		guiError = ::pgrcamguiShowCameraSelectionModal(	guiContext, context, &serialNumber,	&iDialogStatus );
		if ( !checkGuiError( guiError, "pgrcamguiShowCameraSelectionModal() Error showing camera selection dialog." ) || serialNumber == 0 )
			return false;

	//Select videoMode
	if ( aFormat == NULL || aFormat->showDlg )
	{
		flycaptureGetCurrentVideoMode( context, &videoMode, &frameRate );
		if ( !checkCaptureError( capError, "flycaptureGetCurrentVideoMode()" ) )
			return false;	
	}
	else
		videoMode = buildVideoMode( *aFormat );
	

	//Select frame rate	
	if ( aFormat != NULL && !aFormat->showDlg )
	{
		setFrameRate( aFormat->fps );
	}
	//Check video mode
	bool valid = false;
	capError = flycaptureCheckVideoMode( context, videoMode, frameRate, &valid );
	if ( !checkCaptureError( capError, "flycaptureCheckVideoMode()" ) )
		return false;
	//Invalid video mode
	if ( !valid )
		return false;

	//Check Image Size and Sensor offset
	/*unsigned long roiSize;
	flycaptureGetCameraRegister( context, 0xa0c, &roiSize ); 	 
	width = roiSize >> 16; 
	//Offest de la imagen dentro del sensor
	height = roiSize & 0x0000FFFF;	
	unsigned long roiPos;	
	flycaptureGetCameraRegister( context, 0xa08, &roiPos ); 	 
	xOffset = roiPos >> 16;
	yOffset = roiPos & 0x0000FFFF;*/

	//Init camera
	if ( aFormat != NULL && aFormat->getPixelFormatIn() == VM_RAW8)
	{
		return false;
		//Init Custom image
		/*bool		  bAvailable;
		unsigned int	  uiMaxImageSizeCols;
		unsigned int	  uiMaxImageSizeRows;
		unsigned int	  uiUnitSizeHorz;
		unsigned int	  uiUnitSizeVert;
		unsigned int   uiPixelFormats; 
		capError = ::flycaptureQueryCustomImage(
			context,
			0, //mode
			&bAvailable,
			&uiMaxImageSizeCols,
			&uiMaxImageSizeRows,
			&uiUnitSizeHorz,
			&uiUnitSizeVert,
			&uiPixelFormats );
		if ( !checkCaptureError( capError, "flycaptureQueryCustomImage()" ) )
			return false;

		capError = flycaptureStartLockNextCustomImage(
			context, 
			0, //mode
			xOffset, //START_COL
			yOffset, //START_ROW
			aFormat->width,  //COLS
			aFormat->height, //ROWS
			50, //SPEED
			FLYCAPTURE_RAW8);
		if ( !checkCaptureError( capError, "flycaptureStartLockNextCustomImage()" ) )
			return false;*/
	}
	else
	{
		//Init the camera		
		capError = flycaptureStart( context, videoMode, frameRate );
		if ( !checkCaptureError( capError, "flycaptureStartLockNext()" ) )
			return false;
		started = true;
	}

	/*capError = flycaptureStop( context );
	 capError = flycaptureStart( 
      context, 
      FLYCAPTURE_VIDEOMODE_ANY,
      FLYCAPTURE_FRAMERATE_ANY );
	capError = flycaptureGrabImage2( context, &imageG );*/

	//Fill the format with initialized videoMode and framerate
	capError = flycaptureGetCurrentVideoMode( context, &videoMode, &frameRate );
	if ( !checkCaptureError( capError, "flycaptureGetCurrentVideoMode()" ) )
		return false;

	//Check the video mode and initialize the format	
	bool availableFormat = resolveFormat( videoMode, frameRate, format );	
	if ( !availableFormat )
		return false;
		
	ostringstream ss;
	ss << aSerialNumber;
	copyStringToChar( ss.str(), &identification.uniqueName );
	copyStringToChar( "PGR_CAMERA", &identification.identifier );
	copyStringToChar( pinfo->pszModelName, &identification.friendlyName );

	//Update the user's format
	if ( aFormat != NULL )	
		*aFormat = format;	

	return true;
}


inline char *PGRCamera::getFrame( bool wait )
{
	FlyCaptureError capError;
	if ( recording )
	{
		capError = flycaptureLockNext( context, &image );
		
		if ( capError == FLYCAPTURE_OK )
		{
			if ( lastRecordedFrame == -1 )
				lastRecordedFrame = image.uiSeqNum;
			else if ( lastRecordedFrame + 1 != image.uiSeqNum )
			{
				lostFrames = lostFrames + image.uiSeqNum - lastRecordedFrame +1;
				lastRecordedFrame = image.uiSeqNum;
			}
			else
				lastRecordedFrame++;
		}
		else
			return NULL;
		
		//DON'T CONVERT THE IMAGE AND RETURN NULL IF YOU LOST FRAMES
		if ( convertForTheRenderer )
		{
			capError = flycaptureConvertImage( context, &image.image, &imageConverted );
			if ( !checkCaptureError( capError, "flycaptureConvertImage()" ) )
				return NULL;
			pixelBuffer = (char*)imageConverted.pData;
		}
		else
			pixelBuffer = (char*)image.image.pData;
		//pixelBuffer = image.image.pData;
		
		//pixelBuffer = NULL;
		//return NULL;
		return pixelBuffer;
	}
	else
	{
		capError = flycaptureGrabImage2( context, &imageG );
		if ( capError != FLYCAPTURE_OK )
			return NULL;
		
		if ( convertForTheRenderer )
		{
			capError = flycaptureConvertImage( context, &imageG, &imageConverted );
			if ( !checkCaptureError( capError, "flycaptureConvertImage()" ) )
				return NULL;
			pixelBuffer = (char*)imageConverted.pData;
		}
		else
			pixelBuffer = (char*)imageG.pData;

	}
	return pixelBuffer;
}


void PGRCamera::showPropertyPage()
{
	CameraGUIError guiError;
	BOOL bShowing = FALSE;
	pgrcamguiGetSettingsWindowState( guiContext, &bShowing );
	//pgrcamguiShowInfoDlg(m_guicontext, g_context, NULL );
	guiError = pgrcamguiToggleSettingsWindowState( guiContext, GetDesktopWindow() );
	checkGuiError( guiError, "pgrcamguiToggleSettingsWindowState() Error showing the property page");
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
	return lostFrames;
}


bool PGRCamera::setRegister( unsigned long reg, unsigned long value )
{	
	FlyCaptureError capError = flycaptureSetCameraRegister( context, reg, value );
	if ( !checkCaptureError( capError, "flycaptureSetCameraRegister" ) )
		return false;
	return true;
}


bool PGRCamera::setRegisterBroadcast( unsigned long reg, unsigned long value )
{	
	FlyCaptureError capError = flycaptureSetCameraRegisterBroadcast( context, reg, value );
	if ( !checkCaptureError( capError, "flycaptureSetCameraRegister" ) )
		return false;
	return true;
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

VMPixelFormat PGRCamera::resolveColorCodingID( int ID )
{
	switch( ID )
	{
	case 0:
		return VM_GREY8;
	case 1:
		return VM_YUV411;
	case 2:
		return VM_YUV422;
	//case 3:
	//	return VM_YUV444;
	case 4:
		return VM_RGB24;
	case 5:
	case 6:
	case 7:
	case 8:
		return VM_GREY16;
	case 9:
		return VM_RAW8;
	//case 10:
	//	return VM_RAW16;
	default:
		return VM_UNKNOWN;
	}
}

bool PGRCamera::resolveFormat( FlyCaptureVideoMode videoMode, FlyCaptureFrameRate frameRate, VMInputFormat &rFormat )
{
	switch ( videoMode )
	{
		case FLYCAPTURE_VIDEOMODE_1600x1200YUV422:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1600x1200RGB:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1600x1200Y8:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1600x1200Y16:
		{
			rFormat.SetFormat( 1600, 1200, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1280x960YUV422:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1280x960RGB:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1280x960Y8:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1280x960Y16:
		{
			rFormat.SetFormat( 1280, 960, 30, VM_GREY16, VM_GREY16 );
			break;
		}			
		case FLYCAPTURE_VIDEOMODE_1024x768Y8:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1024x768Y16:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1024x768YUV422:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_1024x768RGB:
		{
			rFormat.SetFormat( 1024, 768, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_800x600Y8:
		{
			rFormat.SetFormat( 800, 600, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_800x600Y16:
		{
			rFormat.SetFormat( 800, 600, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_800x600YUV422:
		{
			rFormat.SetFormat( 800, 600, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_800x600RGB:
		{
			rFormat.SetFormat( 800, 600, 30, VM_RGB24, VM_RGB24 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_640x480RGB:
		{
			rFormat.SetFormat( 640, 480, 30, VM_RGB24, VM_RGB24 );			
			break;
		}
		case FLYCAPTURE_VIDEOMODE_640x480YUV422:
		{
			rFormat.SetFormat( 640, 480, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_640x480YUV411:
		{
			rFormat.SetFormat( 640, 480, 30, VM_YUV411, VM_YUV411 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_640x480Y16:
		{
			rFormat.SetFormat( 640, 480, 30, VM_GREY16, VM_GREY16 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_640x480Y8:
		{
			rFormat.SetFormat( 640, 480, 30, VM_GREY8, VM_GREY8 );
			break;
		}
		case FLYCAPTURE_VIDEOMODE_320x240YUV422:
		{
			rFormat.SetFormat( 320, 240, 30, VM_YUV422, VM_YUV422 );
			break;
		}
		/*case FLYCAPTURE_VIDEOMODE_160x120YUV444:
		{
			rFormat.SetFormat( 160, 120, 30, VM_YUV444, VM_YUV444 );
			break;
		}*/
		case FLYCAPTURE_VIDEOMODE_CUSTOM:
		{
			//Get the videomode register base
			unsigned long videoMode, value;
			flycaptureGetCameraRegister( context, CURRENT_VIDEO_MODE, &videoMode ); 
			videoMode = videoMode >> 29;
			if ( flycaptureGetCameraRegister( context, videoModeBase[videoMode] + IMAGE_SIZE, &value ) == FLYCAPTURE_OK )
			{
				int width = ( value >> 16 ) & 0x0000FFFF;
				int height = value & 0x0000FFFF;
				if ( flycaptureGetCameraRegister( context, videoModeBase[videoMode] + COLOR_CODING, &value ) == FLYCAPTURE_OK )
				{
					int colorCoding = ( value >> 24 ) & 0x00000FFF;
					rFormat.SetFormat( width, height, 30, resolveColorCodingID( colorCoding ), resolveColorCodingID( colorCoding ) );
				}
			}
			break;
		}
		default:
		{
			return false;		
		}
	}
	switch ( frameRate )
	{
		case FLYCAPTURE_FRAMERATE_1_875:
		{
			rFormat.fps = 1.875f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_3_75:
		{
			rFormat.fps = 3.75f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_7_5:
		{
			rFormat.fps = 7.5f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_15:
		{
			rFormat.fps = 15.0f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_30:
		{
			rFormat.fps = 30.0f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_60:
		{
			rFormat.fps = 60.0f;
			break;
		}
		case FLYCAPTURE_FRAMERATE_CUSTOM:
		{
			//previously initialized
			break;
		}
		default:
			return false;
	}
	return true;
}


FlyCaptureContext &PGRCamera::getContext()
{
	return context;
}


void PGRCamera::initializeConvertedImage()
{	
	/**if ( imageConverted.pData != NULL )
		delete [] imageConverted.pData;
	imageConverted.pData = new char[ static_cast<size_t>( formatRenderer.width * formatRenderer.height * formatRenderer.getBpp() ) ];
	//flycaptureConvertImage only supports BGR or BGRU
	if ( formatRenderer.pixelFormat == BGR24)
		imageConverted.pixelFormat = FLYCAPTURE_BGR;
	else if ( formatRenderer.pixelFormat == BGR32)
		imageConverted.pixelFormat = FLYCAPTURE_BGRU;	*/
}


void PGRCamera::getAvailableDevices(  VMInputIdentification **deviceList, int &numDevices )
{
	numDevices = 0;
	*deviceList = NULL;

	FlyCaptureInfoEx  arInfo[ 32 ];
	unsigned int	     uiSize = 32;

	FlyCaptureError error = flycaptureBusEnumerateCamerasEx( arInfo, &uiSize );
	//_HANDLE_ERROR( error, "flycaptureBusEnumerateCameras()" );
	VMInputIdentification device;
	numDevices = uiSize;
	*deviceList = new VMInputIdentification[uiSize];
	for( unsigned int uiBusIndex = 0; uiBusIndex < uiSize; uiBusIndex++ )
	{
		FlyCaptureInfoEx* pinfo = &arInfo[ uiBusIndex ];

		ostringstream ss;
		ss << pinfo->SerialNumber;
		copyStringToChar( ss.str(), &device.uniqueName );
		copyStringToChar( "PGR_CAMERA", &device.identifier );
		copyStringToChar( pinfo->pszModelName, &device.friendlyName );
		(*deviceList)[uiBusIndex] = device;		
	}
}

bool PGRCamera::getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit )
{

	unsigned long unit, unitPos, max;
	
	//Maximum image size
	flycaptureGetCameraRegister( context, videoModeBase[videoMode] + MAX_IMAGE_SIZE_INQ, &max );
	Hmax = max >> 16;
	Vmax = max & 0x0000FFFF;

	//Unit Size
	flycaptureGetCameraRegister( context, videoModeBase[videoMode] + UNIT_SIZE_INQ, &unit );
	Hunit = unit >> 16;
	Vunit = unit & 0x0000FFFF;

	//Unit position
	flycaptureGetCameraRegister( context, videoModeBase[videoMode] + UNIT_POSITION_INQ, &unitPos );
	HPosUnit = unitPos >> 16;
	VPosUnit = unitPos & 0x0000FFFF;
	return true;
}

bool PGRCamera::setImageROI( int x, int y, int width, int height )
{	
	/*if ( started )
		flycaptureStop( context );

	FlyCapturePixelFormat  format = FLYCAPTURE_MONO8;
	FlyCaptureError Error;
	if ( (Error = flycaptureStartCustomImage( context, 0, x + xOffset, y + yOffset, 
		width, height, 100, format ) ) != FLYCAPTURE_OK )
	{
		return false;
	}*/

	//Check MAX SIZe A00, A80...
	//UNIT_SIZE_INQ (004h) and UNIT_POSITION_INQ (04Ch) 
	//Hunit, Vunit 004
	//Hposunit, Vposunit 04c
	/*
	Hmax = Hunit * n = Hposunit*n3 (n, n3 are integers)
	Vmax = Vunit * m = Vposunit*m3 (m, m3 are integers)
	Left = Hposunit * n1
	Top = Vposunit * m1
	Width = Hunit * n2
	Height = Vunit * m2 (n1, n2, m1, m2 are integers)
	Left + Width <= Hmax
	Top + Height <= Vmax*/
	
	flycaptureStop( context );

	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit );

	// Put the camera into custom image format
	setRegister( CURRENT_VIDEO_FORMAT, 0xE0000000 );	
	unsigned long videoModeReg = videoMode << 29;
	setRegister( CURRENT_VIDEO_MODE, videoModeReg );

	// Specify position, dimensions & pixel depth:
	unsigned long position = 0; 
	position |= x + Hunit;
	position = ( position << 16 ) | y + Vunit;
	setRegister( videoModeBase[videoMode] + IMAGE_POSITION, position );
	
	unsigned long size = 0; 
	size |= width;
	size = ( size << 16 ) | height;
	setRegister( videoModeBase[videoMode] + IMAGE_SIZE, size);
	
	setRegister( videoModeBase[videoMode] + COLOR_CODING, buildColorCoding( format.getPixelFormatIn() ) << 24 );

	// Update the TOTAL_BYTES_HI_INQ, TOTAL_BYTES_LO_INQ, PACKET_PARA_INQ and  BYTE_PER_PACKET registers
	// VALUE_SETTING (0xA7C)
	setRegister( videoModeBase[videoMode] + VALUE_SETTING, 0x40000000 );   

	// Poll Setting_1 (wait for it to clear)
	unsigned long tempValue;
	FlyCaptureError error;
	do{		
		error = flycaptureGetCameraRegister( context, videoModeBase[videoMode] + VALUE_SETTING, &tempValue);
		checkCaptureError( error, "flycaptureGetCameraRegister()" );
	} while ((tempValue & 0x40000000) != 0);

   // Check ErrorFlag_1
   error = flycaptureGetCameraRegister( context, videoModeBase[videoMode] +  VALUE_SETTING, &tempValue);
   checkCaptureError( error, "flycaptureGetCameraRegister()" );

   if((tempValue & 0x00800000) != 0)
   {
      printf("Invalid custom image parameters specified.\n");
      return false;
   }

	// Choose max packet size: 
	// Read MaxBytePerPacket field from the PACKET_PARA_INQ register (A40h) 
	// Write the Max BytePerPacket size read above to the BytePerPacket field of the BYTE_PER_PACKET register (A44h) 

	error = flycaptureGetCameraRegister( context, videoModeBase[videoMode] + PACKET_PARA_INQ, &tempValue );
	checkCaptureError( error, "flycaptureGetCameraRegister()" );

	error = flycaptureSetCameraRegister( context, videoModeBase[videoMode] + BYTE_PER_PACKET, (tempValue & 0xFFFF) << 16);
	checkCaptureError( error, "flycaptureGetCameraRegister()" );
	
	// Set frame rate:
	// Read the FRAME_RATE register (0x83C) and write it back with bit1 & bit7 cleared and bit6 set.
	// Set the frame rate using the absolute value frame rate control (register 0x968)
	/*error = flycaptureGetCameraRegister( context, FRAME_RATE, &tempValue );
	checkCaptureError( error, "flycaptureGetCameraRegister()" );
	error = flycaptureSetCameraRegister( context, FRAME_RATE, ((tempValue | 0x20000000) | (tempValue & 0xBE000000)));
	checkCaptureError( error, "flycaptureGetCameraRegister()" );
	error = flycaptureSetCameraAbsPropertyEx(context, FLYCAPTURE_FRAME_RATE, false, true, false, 30.0);
	checkCaptureError( error, "flycaptureGetCameraRegister()" );*/

	error = flycaptureStart( context, FLYCAPTURE_VIDEOMODE_CUSTOM, FLYCAPTURE_FRAMERATE_CUSTOM );
	checkCaptureError( error, "flycaptureStart()" );

	return true;
}

bool PGRCamera::moveImageROI( int x, int y )
{
	unsigned long videoModeReg;
	flycaptureGetCameraRegister( context, CURRENT_VIDEO_MODE, &videoModeReg );
	int videoMode = videoModeReg >> 29;
	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit );

	unsigned long position = 0; 
	position |= x + Hunit;
	position = ( position << 16 ) | y + Vunit;
	
	return setRegister( videoModeBase[videoMode] + IMAGE_POSITION, position );	
}

void PGRCamera::printInfo()
{
	FlyCaptureError   error;
	FlyCaptureInfoEx info;
	error = flycaptureGetCameraInfo( context, &info );
    checkCaptureError( error, "flycaptureGetCameraInfo()" );
	printf( "Camera info:\n" );
	printf( "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "DCAM compliance: %1.2f\n"
        "Bus position: (%d,%d).\n",
		info.SerialNumber,
		info.pszModelName,
		info.pszVendorName,
		info.pszSensorInfo,
		info.iBusNum, info.iNodeNum );

}

bool PGRCamera::setGainControl( bool autoGain )
{
	bool onPush, onOff, autov;
	float value;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_GAIN, &onPush, &onOff, &autov, &value );
	if ( flycaptureSetCameraAbsPropertyEx( context, FLYCAPTURE_GAIN, onPush, onOff, autoGain, value ) != FLYCAPTURE_OK )
		return false;
	return true;
}

bool PGRCamera::getGainControl()
{
	bool onPush, onOff, autov;
	float value;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_GAIN, &onPush, &onOff, &autov, &value );
	return autov;		
}

bool PGRCamera::setExposureControl( bool autoExp )
{
	bool onPush, onOff, autov;
	float value;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_AUTO_EXPOSURE, &onPush, &onOff, &autov, &value );
	if ( flycaptureSetCameraAbsPropertyEx( context, FLYCAPTURE_AUTO_EXPOSURE, onPush, onOff, autoExp, value ) != FLYCAPTURE_OK )
		return false;
	return true;
}

bool PGRCamera::setSharpnessControlValue(float value)
{
	bool onPush, onOff, autov;
	float pvalue;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_SHARPNESS, &onPush, &onOff, &autov, &pvalue );
	if ( flycaptureSetCameraAbsPropertyEx( context, FLYCAPTURE_SHARPNESS, onPush, onOff, autov, value ) != FLYCAPTURE_OK )
		return false;
	return true;
}

bool PGRCamera::setSharpnessControl( bool autoSharp )
{
	bool onPush, onOff, autov;
	float value;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_SHARPNESS, &onPush, &onOff, &autov, &value );
	if ( flycaptureSetCameraAbsPropertyEx( context, FLYCAPTURE_SHARPNESS, onPush, onOff, autoSharp, value ) != FLYCAPTURE_OK )
		return false;
	return true;
}

bool PGRCamera::setShutterTime( float shutterTime )
{
	bool onPush, onOff, autov;
	float value;
	flycaptureGetCameraAbsPropertyEx( context, FLYCAPTURE_SHUTTER, &onPush, &onOff, &autov, &value );
	if ( flycaptureSetCameraAbsPropertyEx( context, FLYCAPTURE_SHUTTER, onPush, onOff, autov, shutterTime ) != FLYCAPTURE_OK )
		return false;
	return true;
}

bool PGRCamera::resetImageROI()
{
	unsigned long videoFormatReg;
	flycaptureGetCameraRegister( context, CURRENT_VIDEO_FORMAT, &videoFormatReg );
	if ( videoFormatReg != 0xE0000000 )
		return false;
	unsigned long videoModeReg;
	flycaptureGetCameraRegister( context, CURRENT_VIDEO_MODE, &videoModeReg );
	int videoMode = videoModeReg >> 29;
	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit );
	return setImageROI( 0, 0, Hmax - Hunit * 2, Vmax - Vunit * 2 );
}