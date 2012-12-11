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

FlyCapture2::VideoMode PGRCamera::buildVideoMode( VMInputFormat format )
{	
	if ( format.width == 1600 && format.height == 1200 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case RGB24:			
				return VIDEOMODE_1600x1200RGB;								
			case GREY16:			
				return VIDEOMODE_1600x1200Y16;				
			case GREY8:
				return VIDEOMODE_1600x1200Y8;				
			case YUV422:
				return VIDEOMODE_1600x1200YUV422;				
		}
	}	
	else if ( format.width == 1280 && format.height == 960 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case RGB24:
				return VIDEOMODE_1280x960RGB;								
			case GREY16:			
				return VIDEOMODE_1280x960Y16;				
			case GREY8:
				return VIDEOMODE_1280x960Y8;				
			case YUV422:
				return VIDEOMODE_1280x960YUV422;							
				
		}
	}
	else if ( format.width == 1024 && format.height == 768 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case RGB24:
				return VIDEOMODE_1024x768RGB;								
			case GREY16:			
				return VIDEOMODE_1024x768Y16;				
			case GREY8:
				return VIDEOMODE_1024x768Y8;				
			case YUV422:
				return VIDEOMODE_1024x768YUV422;											
		}
	}
	else if ( format.width == 800 && format.height == 600 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case RGB24:
				return VIDEOMODE_800x600RGB;								
			case GREY16:			
				return VIDEOMODE_800x600Y16;				
			case GREY8:
				return VIDEOMODE_800x600Y8;				
			case YUV422:
				return VIDEOMODE_800x600YUV422;				
		}
	}	
	else if ( format.width == 640 && format.height == 480 )
	{
		switch ( format.getPixelFormatIn() )
		{
			case RGB24:
				return VIDEOMODE_640x480RGB;				
			case GREY16:
				return VIDEOMODE_640x480Y16;				
			case GREY8:
				return VIDEOMODE_640x480Y8;				
			case YUV422:
				return VIDEOMODE_640x480YUV422;				
			case YUV411:
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
		;
		if( pgrCamerap->callback )
			(*pgrCamerap->callback)( pgrCamerap->pixelBuffer, pgrCamerap->inputID, pImage->GetTimeStamp().cycleSeconds,  pgrCamerap->frameCallbackData );			
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

	//FC2Config config;
	//config.grabMode = BUFFER_FRAMES;

	/*PGRGuid guid[64];
	unsigned int psize = 64;
	CameraSelectionDlg selectionDlg;
	bool ok;	
	selectionDlg.ShowModal( &ok, guid, &psize );
	if ( !ok || psize != 1 ) 
		return false;
	error = cam.Connect(&guid[0]);*/

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

	// Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }

	resetCamera();
	cam.WriteRegister( CAMERA_POWER, 0x80000000 );
	//PrintCameraInfo(&camInfo);
	 
//	config.numBuffers = 100;
//	cam.SetConfiguration( &config );
	
	// Show the camera configuration dialog.		
	if ( aFormat == NULL || aFormat->showDlg )
	{
		CameraControlDlg controlDlg;
		controlDlg.Connect( &cam );
		controlDlg.ShowModal( NULL );
		controlDlg.Hide();
		controlDlg.Disconnect();
		//int		iDialogStatus;
		//The user must select the camera
		//guiError = ::pgrcamguiShowCameraSelectionModal(	guiContext, context, &serialNumber,	&iDialogStatus );
		//if ( !checkGuiError( guiError, "pgrcamguiShowCameraSelectionModal() Error showing camera selection dialog." ) || serialNumber == 0 )
		//	return false;
	}
	else if ( aFormat != NULL )
	{
		videoMode = buildVideoMode( *aFormat );

		//Select frame rate	
		if ( aFormat != NULL && !aFormat->showDlg )
		{
			if ( aFormat->fps == 1.875 )
				frameRate = FRAMERATE_1_875;
			else if ( aFormat->fps == 3.75 )
				frameRate = FRAMERATE_3_75;
			else if ( aFormat->fps == 7.5 )
				frameRate = FRAMERATE_7_5;
			else if ( aFormat->fps == 15.0 )
				frameRate = FRAMERATE_15;
			else if ( aFormat->fps == 30.0 )
				frameRate = FRAMERATE_30;
			else if ( aFormat->fps == 60.0 )
				frameRate = FRAMERATE_60;
			else if ( aFormat->fps == 120.0 )
				frameRate = FRAMERATE_120;
			else if ( aFormat->fps == 240.0 )
				frameRate = FRAMERATE_240;
			else
				frameRate = FRAMERATE_FORMAT7;
		}
		error = cam.SetVideoModeAndFrameRate( videoMode, frameRate );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return false;
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
	error = cam.StartCapture( ); //Start Isochronous image capture
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        //return false;
    }

	VideoMode videoMode;
	FrameRate frameRate;	
	cam.GetVideoModeAndFrameRate( &videoMode, &frameRate );

	//Check the video mode and initialize the format	
	bool availableFormat = resolveFormat( videoMode, frameRate, format );	
	if ( !availableFormat )
		return false;
	
	//Change the user's format
	if ( aFormat != NULL )	
		*aFormat = format; 
	
	ostringstream ss;
	ss << aSerialNumber;
	copyStringToChar( ss.str(), &identification.uniqueName );
	copyStringToChar( "PGR_CAMERA2", &identification.identifier );
	copyStringToChar( camInfo.modelName, &identification.friendlyName );
//	act = 0;

	return true;
}

void PGRCamera::releaseFrame()
{
	rawImage.ReleaseBuffer();
	//if ( lastImage )
	//	lastImage->ReleaseBuffer();
	lastImage = NULL;
	pixelBuffer = NULL;
	/*Error error = convertedImage.Save("C:/kk2.pgm");
	if (error != PGRERROR_OK)
        PrintError( error );*/
	//error = convertedImageCopy.ReleaseBuffer();
	//if (error != PGRERROR_OK)
        //PrintError( error );
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
		case PIXEL_FORMAT_RAW8:
			return GREY8;
		case PIXEL_FORMAT_411YUV8:
			return YUV411;
		case PIXEL_FORMAT_422YUV8:
			return YUV422;
		//case PIXEL_FORMAT_444YUV8:
			//return YUV444;
		case PIXEL_FORMAT_MONO16:
		case PIXEL_FORMAT_S_MONO16:
		case PIXEL_FORMAT_RAW16:
			return GREY16;
		case PIXEL_FORMAT_RGB8:
			return RGB24;
		case PIXEL_FORMAT_BGR:
			return BGR24;
	}
	return UNKNOWN;
}


bool PGRCamera::resolveFormat( VideoMode videoMode, FrameRate frameRate, VMInputFormat &rFormat )
{
	switch ( videoMode )
	{
		case VIDEOMODE_1600x1200YUV422:
		{
			rFormat.SetFormat( 1600, 1200, 30, YUV422, YUV422 );
			break;
		}
		case VIDEOMODE_1600x1200RGB:
		{
			rFormat.SetFormat( 1600, 1200, 30, RGB24, RGB24 );
			break;
		}
		case VIDEOMODE_1600x1200Y8:
		{
			rFormat.SetFormat( 1600, 1200, 30, GREY8, GREY8 );
			break;
		}
		case VIDEOMODE_1600x1200Y16:
		{
			rFormat.SetFormat( 1600, 1200, 30, GREY16, GREY16 );
			break;
		}
		case VIDEOMODE_1280x960YUV422:
		{
			rFormat.SetFormat( 1280, 960, 30, YUV422, YUV422 );
			break;
		}
		case VIDEOMODE_1280x960RGB:
		{
			rFormat.SetFormat( 1280, 960, 30, RGB24, RGB24 );
			break;
		}
		case VIDEOMODE_1280x960Y8:
		{
			rFormat.SetFormat( 1280, 960, 30, GREY8, GREY8 );
			break;
		}
		case VIDEOMODE_1280x960Y16:
		{
			rFormat.SetFormat( 1280, 960, 30, GREY16, GREY16 );
			break;
		}			
		case VIDEOMODE_1024x768Y8:
		{
			rFormat.SetFormat( 1024, 768, 30, GREY8, GREY8 );
			break;
		}
		case VIDEOMODE_1024x768Y16:
		{
			rFormat.SetFormat( 1024, 768, 30, GREY16, GREY16 );
			break;
		}
		case VIDEOMODE_1024x768YUV422:
		{
			rFormat.SetFormat( 1024, 768, 30, YUV422, YUV422 );
			break;
		}
		case VIDEOMODE_1024x768RGB:
		{
			rFormat.SetFormat( 1024, 768, 30, RGB24, RGB24 );
			break;
		}
		case VIDEOMODE_800x600Y8:
		{
			rFormat.SetFormat( 800, 600, 30, GREY8, GREY8 );
			break;
		}
		case VIDEOMODE_800x600Y16:
		{
			rFormat.SetFormat( 800, 600, 30, GREY16, GREY16 );
			break;
		}
		case VIDEOMODE_800x600YUV422:
		{
			rFormat.SetFormat( 800, 600, 30, YUV422, YUV422 );
			break;
		}
		case VIDEOMODE_800x600RGB:
		{
			rFormat.SetFormat( 800, 600, 30, RGB24, RGB24 );
			break;
		}
		case VIDEOMODE_640x480RGB:
		{
			rFormat.SetFormat( 640, 480, 30, RGB24, RGB24 );			
			break;
		}
		case VIDEOMODE_640x480YUV422:
		{
			rFormat.SetFormat( 640, 480, 30, YUV422, YUV422 );
			break;
		}
		case VIDEOMODE_640x480YUV411:
		{
			rFormat.SetFormat( 640, 480, 30, YUV411, YUV411 );
			break;
		}
		case VIDEOMODE_640x480Y16:
		{
			rFormat.SetFormat( 640, 480, 30, GREY16, GREY16 );
			break;
		}
		case VIDEOMODE_640x480Y8:
		{
			rFormat.SetFormat( 640, 480, 30, GREY8, GREY8 );
			break;
		}
		case VIDEOMODE_320x240YUV422:
		{
			rFormat.SetFormat( 320, 240, 30, YUV422, YUV422 );
			break;
		}		
		case VIDEOMODE_FORMAT7:
		{	
			Format7ImageSettings pImageSettings;
			unsigned int pPacketSize;
			float pPercentage;
			Error error = cam.GetFormat7Configuration( &pImageSettings, &pPacketSize, &pPercentage );
			if ( error != PGRERROR_OK )
			{
				PrintError( error );
				return false;
			}			
			rFormat.SetFormat( pImageSettings.width, pImageSettings.height, 30, resolvePixelFormat( pImageSettings.pixelFormat ), resolvePixelFormat( pImageSettings.pixelFormat ) );
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
		case FRAMERATE_FORMAT7:
		{
			//previously initialized
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

bool PGRCamera::setImageROI( int x, int y, int width, int height, int videoMode )
{	
	cam.StopCapture();

	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit, videoMode );

	// Put the camera into custom image mode:
	cam.WriteRegister( CURRENT_VIDEO_FORMAT, 0xE0000000 );	
	unsigned long videoModeReg = videoMode << 29;
	cam.WriteRegister( CURRENT_VIDEO_MODE, videoModeReg );

	// Specify position, dimensions & pixel depth:
	unsigned long position = 0; 
	position |= x + Hunit;
	position = ( position << 16 ) | y + Vunit;
	cam.WriteRegister( videoModeBase[videoMode] + IMAGE_POSITION, position );
	
	unsigned long size = 0; 
	size |= width;
	size = ( size << 16 ) | height;
	cam.WriteRegister( videoModeBase[videoMode] + IMAGE_SIZE, size);

	cam.WriteRegister( videoModeBase[videoMode] + COLOR_CODING, buildColorCoding( format.getPixelFormatIn() ) << 24 );

	// Update the TOTAL_BYTES_HI_INQ, TOTAL_BYTES_LO_INQ, PACKET_PARA_INQ and  BYTE_PER_PACKET registers
	// VALUE_SETTING (0xA7C)
	cam.WriteRegister( videoModeBase[videoMode] + VALUE_SETTING, 0x40000000 );   

	// Poll Setting_1 (wait for it to clear)
	unsigned int tempValue;
	do{		
		cam.ReadRegister( videoModeBase[videoMode] + VALUE_SETTING, &tempValue );
	} while ((tempValue & 0x40000000) != 0);

   // Check ErrorFlag_1
   cam.ReadRegister( videoModeBase[videoMode] +  VALUE_SETTING, &tempValue );

   if((tempValue & 0x00800000) != 0)
   {
      printf("Invalid custom image parameters specified.\n");
      return false;
   }

	// Choose max packet size: 
	// Read MaxBytePerPacket field from the PACKET_PARA_INQ register (A40h) 
	// Write the Max BytePerPacket size read above to the BytePerPacket field of the BYTE_PER_PACKET register (A44h) 

	cam.ReadRegister( videoModeBase[videoMode] + PACKET_PARA_INQ, &tempValue );	
	cam.WriteRegister( videoModeBase[videoMode] + BYTE_PER_PACKET, (tempValue & 0xFFFF) << 16);
	
	// Set frame rate:
	// Read the FRAME_RATE register (0x83C) and write it back with bit1 & bit7 cleared and bit6 set.
	// Set the frame rate using the absolute value frame rate control (register 0x968)
	/*error = flycaptureGetCameraRegister( context, REG_FRAME_RATE, &tempValue );
	checkCaptureError( error, "flycaptureGetCameraRegister()" );
	error = flycaptureSetCameraRegister( context, REG_FRAME_RATE, ((tempValue | 0x20000000) | (tempValue & 0xBE000000)));
	checkCaptureError( error, "flycaptureGetCameraRegister()" );
	error = flycaptureSetCameraAbsPropertyEx(context, FLYCAPTURE_FRAME_RATE, false, true, false, 30.0);
	checkCaptureError( error, "flycaptureGetCameraRegister()" );*/

	cam.StartCapture();		
	return true;

	Format7ImageSettings imageSettings;
	imageSettings.width = width;
	imageSettings.height = height;
	imageSettings.offsetX = x + Hunit;
	imageSettings.offsetY = y + Vunit;
	imageSettings.pixelFormat = PIXEL_FORMAT_MONO8;

    // Validate the settings to make sure that they are valid
	bool valid;
    Format7PacketInfo fmt7PacketInfo;
    Error error = cam.ValidateFormat7Settings( &imageSettings, &valid, &fmt7PacketInfo );
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
	cam.StopCapture();
    // Set the settings to the camera
    /*error = cam.SetFormat7Configuration( &imageSettings, fmt7PacketInfo.recommendedBytesPerPacket );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }*/

	unsigned long yr = 0; 
	yr |= width;
	yr = ( yr << 16 ) | height;
	error = cam.WriteRegister( 0xa0c, yr );
	if ( error != PGRERROR_OK )
	{
		PrintError( error );
		return false;
	}

	error = cam.StartCapture();
	if ( error != PGRERROR_OK )
	{
		PrintError( error );
		return false;
	}
	return true;
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
	unsigned int videoModeReg;
	cam.ReadRegister( CURRENT_VIDEO_MODE, &videoModeReg );
	int videoMode = videoModeReg >> 29;
	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit, videoMode );

	unsigned long position = 0; 
	position |= x + Hunit;
	position = ( position << 16 ) | y + Vunit;	
	
	Error error = cam.WriteRegister( videoModeBase[videoMode] + IMAGE_POSITION, position );
	return ( error == PGRERROR_OK );
}

unsigned int PGRCamera::getRegisterValue(unsigned long _register )
{
	unsigned int value;
	cam.ReadRegister( _register, &value );	
	return value;
}

bool PGRCamera::getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit, int videoMode )
{
	if ( videoMode < 0 || videoMode > 5 )
	{
		cout <<"getROIUnits(): Invalid mode" << endl;
		return false;
	}

	unsigned long unit, unitPos, max;
	
	//Maximum image size
	max = getRegisterValue( videoModeBase[videoMode] + MAX_IMAGE_SIZE_INQ );
	Hmax = max >> 16;
	Vmax = max & 0x0000FFFF;

	//Unit Size
	unit = getRegisterValue( videoModeBase[videoMode] + UNIT_SIZE_INQ );
	Hunit = unit >> 16;
	Vunit = unit & 0x0000FFFF;

	//Unit position
	unitPos = getRegisterValue( videoModeBase[videoMode] + UNIT_POSITION_INQ );
	HPosUnit = unitPos >> 16;
	VPosUnit = unitPos & 0x0000FFFF;
	return true;
}

bool PGRCamera::resetImageROI()
{
	unsigned int videoFormatReg;
	cam.ReadRegister( CURRENT_VIDEO_FORMAT, &videoFormatReg );
	if ( videoFormatReg != 0xE0000000 )
		return false;
	unsigned int videoModeReg;
	cam.ReadRegister( CURRENT_VIDEO_MODE, &videoModeReg );
	int videoMode = videoModeReg >> 29;
	int Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit;
	getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit, videoMode );
	return setImageROI( 0, 0, Hmax - Hunit * 2, Vmax - Vunit * 2, videoMode );
}

int PGRCamera::buildColorCoding( VMPixelFormat pixelFormat )
{
	switch( pixelFormat )
	{
	case GREY8:
		return 0;
	case YUV411:
		return 1;
	case YUV422:
		return 2;
	//case 3:
	//	return YUV444;
	case RGB24:
		return 4;
	case GREY16:
		return 5;
	case RAW8:
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
	return ( cam.SetTriggerMode( &triger ) == PGRERROR_OK );
}

bool PGRCamera::fireSoftwareTrigger( bool broadcast )
{
	return ( cam.FireSoftwareTrigger( broadcast ) == PGRERROR_OK );
}

bool PGRCamera::setStrobeOutput()
{
	/*StrobeControl strobeControl;
	strobeControl.source = 2;
	cam.GetStrobe( &strobeControl );
	strobeControl.source = 2;
	strobeControl.delay = 0;
	strobeControl.duration = 6;
	strobeControl.onOff = true;	
	Error error = cam.SetStrobe( &strobeControl );
	PrintError( error );
	cam.GetStrobe( &strobeControl );
	StrobeInfo strobeInfo;
	strobeInfo.source = 2;
	error = cam.GetStrobeInfo( &strobeInfo );*/

	cam.WriteRegister( GPIO_STRPAT_CTRL, 0x80000100 ); //El periodo del patron es de 1 frame (stobe pattern)
	cam.WriteRegister( GPIO_CTRL_PIN_2, 0x80030000 ); //GPIO0 en modo strobe GPIO_MODE_3
	cam.WriteRegister( GPIO_XTRA_PIN_2, 0x00001800 ); //GPIO_MODE_3: Duration of the pulse
	cam.WriteRegister( GPIO_STRPAT_MASK_PIN_2, 0x8000FFFF ); //Patron del GPIO0: encendido siempre

	return true;
}

void PGRCamera::stop()
{
	Error error = cam.StopCapture();
    if (error != PGRERROR_OK)           
        PrintError( error );
}

bool PGRCamera::setFrameRate( float frameRate )
{
	Property prop;
	prop.type = FRAME_RATE;
	cam.GetProperty( &prop );
	prop.autoManualMode = false;
	prop.absControl = true;
	prop.onOff = true;
	prop.absValue = frameRate;
	return ( cam.SetProperty( &prop ) == PGRERROR_OK );	
}