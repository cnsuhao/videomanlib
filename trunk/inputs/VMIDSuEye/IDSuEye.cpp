#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "IDSuEye.h"
#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include <time.h>
#include <stdlib.h>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

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

IDSuEye::IDSuEye(void)
{
	m_hCam = NULL;
	m_pcImageMemory = NULL;
	lastPixelBuffer = NULL;
}

IDSuEye::~IDSuEye(void)
{
	if( m_hCam != 0 )
	{
		// Disable messages
		//is_EnableMessage( m_hCam, IS_FRAME, NULL );

		// Stop live video
		is_StopLiveVideo( m_hCam, IS_WAIT );

		// Free the allocated buffer
		if( m_pcImageMemory != NULL )
  			is_FreeImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
		m_pcImageMemory = NULL;

		is_DisableEvent( m_hCam, IS_SET_EVENT_FRAME );
		is_ExitEvent( m_hCam, IS_SET_EVENT_FRAME );

		// Close camera
		is_ExitCamera( m_hCam );
        m_hCam = NULL;
	}
}

int IDSuEye::colorModeFromPixelFormat( VMPixelFormat pixelFormat )
{
	switch (pixelFormat)
	{
	case VM_GREY8:
		return IS_CM_MONO8;
	case VM_GREY16:
		return IS_CM_MONO16;
	case VM_RAW8:
		return IS_CM_SENSOR_RAW8;
	case VM_RGB24:
		return IS_CM_RGB8_PACKED;
	case VM_BGR24:
		return IS_CM_BGR8_PACKED;
	case VM_RGB32:
		return IS_CM_RGBA8_PACKED;
	case VM_BGR32:
		return IS_CM_BGRA8_PACKED;
	case VM_YUV422:
		return IS_CM_UYVY_PACKED;
	default:
		return IS_CM_RGB8_PACKED;
	}
}

VMPixelFormat IDSuEye::pixelFormatFromColorMode( int colorMode )
{
	switch (colorMode)
	{
	case IS_CM_MONO8:
		return VM_GREY8;
	case IS_CM_MONO16:
		return VM_GREY16;
	case IS_CM_SENSOR_RAW8:
		return VM_RAW8;
	case IS_CM_RGB8_PACKED:
		return VM_RGB24;
	case IS_CM_BGR8_PACKED:
		return VM_BGR24;
	case IS_CM_RGBA8_PACKED:
		return VM_RGB32;
	case IS_CM_BGRA8_PACKED:
		return VM_BGR32;
	case IS_CM_UYVY_PACKED:
		return VM_YUV422;
	default:
		return VM_UNKNOWN;
	}
}

bool IDSuEye::initInput( const VMInputIdentification &device, VMInputFormat *aformat )
{
	int cameraId = 0;
	if ( device.uniqueName )
	{
		std::istringstream ss ( device.uniqueName );
		ss >> cameraId;
	}

	if ( cameraId >= 1 || cameraId <= 254 )
		m_hCam = (HIDS)cameraId; // open next camera
	else
		m_hCam = (HIDS)0; // open next camera
	int nRet = is_InitCamera( &m_hCam, NULL ); // init camera - no window handle required
	if ( nRet == IS_SUCCESS )
	{
		// retrieve original image size
		is_GetSensorInfo(m_hCam, &m_sInfo);
		m_nSizeX = m_sInfo.nMaxWidth;
		m_nSizeY = m_sInfo.nMaxHeight;
		copyStringToChar( m_sInfo.strSensorName, &identification.friendlyName );

		CAMINFO pInfo;
		if ( is_GetCameraInfo( m_hCam, &pInfo ) == IS_SUCCESS )
		{
			ostringstream ss;
			ss << (int)pInfo.Select;
			copyStringToChar( ss.str(), &identification.uniqueName );
		}
		else
		{
			ostringstream ss;
			if ( cameraId != 0 )
				ss << cameraId;
			else
				ss << m_hCam;
			copyStringToChar( ss.str(), &identification.uniqueName );
		}
		copyStringToChar( "IDS_uEye_CAMERA", &identification.identifier );

		// Added: mnieto
		// Enable Messages (to allow timestamps)
		/*is_EnableMessage(m_hCam,    IS_DEVICE_REMOVED,      GetSafeHwnd());
		is_EnableMessage(m_hCam,    IS_DEVICE_RECONNECTED,  GetSafeHwnd());
		is_EnableMessage(m_hCam,    IS_FRAME,               GetSafeHwnd());
        is_EnableMessage(m_hCam,    IS_TRIGGER,             GetSafeHwnd());*/
	}
	else
	{
		cerr << "No uEye camera could be opened !" << endl;
	    return false;
    }

	nRet = IS_NO_SUCCESS;
    if (m_hCam == NULL)
	{
		cerr << "Invalid HIDS " << m_hCam << endl;
		if ( device.uniqueName )
			cerr << "uniqueName " << string( device.uniqueName ) << endl;
        return false;
	}

	// Set display mode to DIB
    nRet = is_SetDisplayMode(m_hCam, IS_SET_DM_DIB);
	if ( !aformat || aformat->getPixelFormatIn() == VM_UNKNOWN )
	{
		//Get Image size
		IS_RECT rectAOI;
		nRet = is_AOI(m_hCam, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
		if (nRet == IS_SUCCESS)
		{
		  int x     = rectAOI.s32X;
		  int y     = rectAOI.s32Y;
		  m_nSizeX = rectAOI.s32Width;
		  m_nSizeY = rectAOI.s32Height;
		}
		else
		{
			cerr << "Error getting AOI" << endl;
			return false;
		}

		if (m_sInfo.nColorMode == IS_COLORMODE_BAYER)
		{
			m_nColorMode = IS_CM_RGB8_PACKED;
			m_nBitsPerPixel = 24;
			format.SetFormat( m_nSizeX, m_nSizeY, 30.0, VM_RGB24, VM_RGB24 );
		}
		else
		{
			m_nColorMode = IS_CM_MONO8;
			m_nBitsPerPixel = 8;
			format.SetFormat( m_nSizeX, m_nSizeY, 30.0, VM_GREY8, VM_GREY8 );
		}
	}
	else
	{
		// Get number of available formats and size of list
		UINT count;
		UINT bytesNeeded = sizeof(IMAGE_FORMAT_LIST);
		nRet = is_ImageFormat( m_hCam, IMGFRMT_CMD_GET_NUM_ENTRIES, &count, sizeof(count) );
		bytesNeeded += (count - 1) * sizeof(IMAGE_FORMAT_INFO);
		void* ptr = malloc(bytesNeeded);
		// Create and fill list
		IMAGE_FORMAT_LIST* pformatList = (IMAGE_FORMAT_LIST*) ptr;
		pformatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
		pformatList->nNumListElements = count;
		nRet = is_ImageFormat( m_hCam, IMGFRMT_CMD_GET_LIST, pformatList, bytesNeeded );

		//Set the desired format
		format.setPixelFormat( aformat->getPixelFormatIn(),aformat->getPixelFormatIn() );
		m_nColorMode = colorModeFromPixelFormat( aformat->getPixelFormatIn() );
		m_nSizeX = aformat->width;
		m_nSizeY = aformat->height;
		format.width = m_nSizeX;
		format.height = m_nSizeY;
		format.fps = aformat->fps;

		//Check if an image size matches the format
		bool encontrado = false;
		for ( int f = 0; !encontrado && f < (int)pformatList->nNumListElements; ++f )
		{
			if ( pformatList->FormatInfo[f].nWidth == aformat->width && pformatList->FormatInfo[f].nHeight == aformat->height )
			{
				encontrado = true;
				nRet = is_ImageFormat( m_hCam, IMGFRMT_CMD_SET_FORMAT, &pformatList->FormatInfo[f].nFormatID, sizeof(pformatList->FormatInfo[f].nFormatID) );
			}
		}
		if ( !encontrado )
		{
			cerr << "Proper image size not found " << aformat->width << " " << aformat->height << endl;
			return false;
		}
	}

//	is_SetImagePos( m_hCam, 0, 0 );
	// set the desired color mode
	if ( is_SetColorMode(m_hCam, m_nColorMode ) != IS_SUCCESS )
	{
		cerr << "Invalid color mode" << endl;
		return false;
	}
	format.width = m_nSizeX;
	format.height = m_nSizeY;
	int m_nColorMode = is_SetColorMode(m_hCam, IS_GET_COLOR_MODE );
	VMPixelFormat pixelFormat = pixelFormatFromColorMode( m_nColorMode );
	format.setPixelFormat( pixelFormat, pixelFormat );
	m_nBitsPerPixel = format.depth * format.nChannels;
	 // allocate an image memory.
	if (is_AllocImageMem(m_hCam, m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId ) != IS_SUCCESS)
	{
		cerr << "Memory allocation failed!" << endl;
		return false;
	}
	//is_AddToSequence( m_hCam, m_pcImageMemory[i], m_lMemoryId[i] );
	nRet = is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	if ( nRet != IS_SUCCESS )
	{
		cerr << "is_SetImageMem failed" << endl;
		return false;
	}

    // Enable Messages
    //is_InitEvent()
	//is_EnableEvent()
	//is_DisableEvent()
	//is_ExitEvent()
	//IS_SET_EVENT_FRAME
	//is_WaitEvent

	//Activate and initialise FRAME event
	#ifdef WIN32
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, "");
	if ( m_hEvent == NULL )
	{
		cerr << "CreateEvent failed" << endl;
		return false;
	}
    #endif

	nRet = is_EnableEvent( m_hCam, IS_SET_EVENT_FRAME);
	if ( nRet != IS_SUCCESS )
	{
		cerr << "is_EnableEvent failed" << endl;		
		return false;
	}

    #ifdef WIN32
	nRet = is_InitEvent( m_hCam, m_hEvent, IS_SET_EVENT_FRAME);
	if ( nRet != IS_SUCCESS )
	{
		cerr << "is_InitEvent failed" << endl;		
		return false;
	}
    #endif
	//Start image capture and wait 1000 ms for event to occur
	/*is_FreezeVideo (hCam, IS_DONT_WAIT);
	is_WaitEvent (hCam, IS_SET_EVENT_FRAME, 1000);
	*/

	//Get Pixel clock range
	UINT nRange[3];
	ZeroMemory(nRange, sizeof(nRange));
	nRet = is_PixelClock( m_hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nRange, sizeof(nRange) );
	//Get default Pixel clock
	UINT nPixelClockDefault;
	nRet = is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_GET_DEFAULT,
						(void*)&nPixelClockDefault, sizeof(nPixelClockDefault));
	//is_PixelClock( m_hCam, IS_PIXELCLOCK_CMD_SET, (void*)&nRange[0], sizeof(nRange[1]) );
	//Set default Pixel clock
	is_PixelClock( m_hCam, IS_PIXELCLOCK_CMD_SET, (void*)&nPixelClockDefault, sizeof(nPixelClockDefault) );


	// start live video
	nRet = is_CaptureVideo( m_hCam, IS_WAIT );
	if ( nRet != IS_SUCCESS )
	{
		cerr << "is_CaptureVideo failed" << endl;
		return false;
	}
	//is_FreezeVideo( m_hCam, IS_DONT_WAIT );

	if ( is_SetFrameRate(m_hCam, format.fps, &format.fps ) != IS_SUCCESS )
	{
		cerr << "Invalid frame rate" << endl;
		return false;
	}

	//Exposure = 1/FrameRate
	double exposure = 4;
	nRet = is_Exposure( m_hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposure, sizeof(exposure)  );
	if ( nRet != IS_SUCCESS )
	{
		cerr << "is_Exposure failed" << endl;
		return false;
	}
	//after start live video query framerate
	nRet = is_GetFramesPerSecond( m_hCam, &format.fps );
	if ( nRet != IS_SUCCESS || format.fps == 0 )
	{
		cerr << "is_GetFramesPerSecond failed" << endl;
		return false;
	}

	if ( aformat )
		*aformat = format;


	//is_GetImageHistogram()

    return true;
}

void IDSuEye::releaseFrame( )
{
	lastPixelBuffer = pixelBuffer;
	pixelBuffer = NULL;
}

char *IDSuEye::getFrame( bool wait)
{
    #ifdef WIN32
	if ( WaitForSingleObject( m_hEvent, 100 ) == WAIT_OBJECT_0 )
	{
		CloseHandle(m_hEvent);
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, "");
		is_InitEvent( m_hCam, m_hEvent, IS_SET_EVENT_FRAME);
		pixelBuffer = m_pcImageMemory;		
		m_valid_uInfo = (is_GetImageInfo( m_hCam, m_lMemoryId, &m_uInfo, sizeof(m_uInfo)) == IS_SUCCESS)?(true):(false);				
		return m_pcImageMemory;
	}
	#endif
	#ifdef linux
	if ( is_WaitEvent( m_hCam, IS_SET_EVENT_FRAME, 100) == IS_SUCCESS )
	{
	    pixelBuffer = m_pcImageMemory;
		return m_pcImageMemory;
    }
	#endif
	return NULL;
}

void IDSuEye::getAvailableDevices( VMInputIdentification **deviceList, int &numDevices  )
{
	numDevices = 0;
	*deviceList = NULL;
	vector <VMInputIdentification> tempList;
	//*deviceList = new VMInputIdentification[numDevices];
	//copyStringToChar( "IDS_uEye_CAMERA", &(*deviceList)->identifier );

	int nNumCam;
	if( is_GetNumberOfCameras( &nNumCam ) == IS_SUCCESS)
	{
		if( nNumCam >= 1 )
		{
			// Create new list with suitable size
			UEYE_CAMERA_LIST* pucl;
			pucl = (UEYE_CAMERA_LIST*) new char [sizeof (DWORD)+ nNumCam* sizeof (UEYE_CAMERA_INFO)];
			pucl->dwCount = nNumCam;
			//Retrieve camera info
			if (is_GetCameraList(pucl) == IS_SUCCESS)
			{
				int iCamera;
				for (iCamera = 0; iCamera < (int)pucl->dwCount; iCamera++ )
				{
					if ( !pucl->uci[iCamera].dwInUse )
					{
						//Test output of camera info on the screen
						//printf("Camera %i Id: %d", iCamera, pucl->uci[iCamera].dwCameraID);
						VMInputIdentification device;
						//istringstream ss( pucl->uci[iCamera].SerNo );
						//ss >> device.serialNumber;
						ostringstream ss;
						ss << pucl->uci[iCamera].dwCameraID;
						copyStringToChar( ss.str(), &device.uniqueName );
						copyStringToChar( pucl->uci[iCamera].Model, &device.friendlyName );
						copyStringToChar( "IDS_uEye_CAMERA", &device.identifier );
						tempList.push_back( device );
					}
				}
			}
			delete pucl;
		}
	}
	if ( tempList.size() > 0 )
	{
		numDevices = tempList.size();
		*deviceList = new VMInputIdentification[tempList.size()];
		for ( size_t d = 0; d < tempList.size(); ++d )
			(*deviceList)[d] = tempList[d];
	}
}

bool IDSuEye::setImageROI( int x, int y, int width, int height )
{
	is_StopLiveVideo( m_hCam, IS_WAIT );

	if( m_pcImageMemory != NULL )
		is_FreeImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	m_pcImageMemory = NULL;


	/*if ( is_SetImageSize( m_hCam, width, height ) != IS_SUCCESS )
	{
		cerr << "Invalid image size" << endl;
		return false;
	}
	if ( is_SetImagePos( m_hCam, x, y ) != IS_SUCCESS )
	{
		cerr << "Invalid image position" << endl;
		return false;
	}*/
	IS_RECT rectAOI;
	rectAOI.s32X     = x;
	rectAOI.s32Y     = y;
	rectAOI.s32Width = width;
	rectAOI.s32Height = height;
	if ( is_AOI( m_hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI)) != IS_SUCCESS )
	{
		cerr << "Invalid AOI" << endl;
		return false;
	}
	if (is_AllocImageMem(m_hCam, width, height, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId ) != IS_SUCCESS)
	{
		cerr << "Memory allocation failed!" << endl;
		return false;
	}
	is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	//is_AddToSequence( m_hCam, m_pcImageMemory[i], m_lMemoryId[i] );

    #ifdef WIN32
        CloseHandle(m_hEvent);
        m_hEvent = CreateEvent(NULL, TRUE, FALSE, "");
        is_InitEvent( m_hCam, m_hEvent, IS_SET_EVENT_FRAME);
    #endif
	if ( is_CaptureVideo( m_hCam, IS_WAIT ) != IS_SUCCESS )
		return false;
	return true;
}

bool IDSuEye::setImageROIpos( int x, int y )
{
	IS_RECT rectAOI;
	//is_AOI( m_hCam, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
	rectAOI.s32X     = x;
	rectAOI.s32Y     = y;
	IS_POINT_2D position;
	position.s32X = x;
	position.s32Y = y;
	if ( is_AOI( m_hCam, IS_AOI_IMAGE_SET_POS_FAST, (void*)&position, sizeof(position)) != IS_SUCCESS )
	{
		cerr << "Invalid AOI position" << endl;
		return false;
	}
	return true;
}

bool IDSuEye::linkController( VideoManInputController *acontroller )
{
	if ( acontroller->setInput( this ) )
	{
		controller = acontroller;
		return true;
	}
	return false;
}

void IDSuEye::getExposure( double &shutterTime )
{
	is_Exposure( m_hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, &shutterTime, sizeof(shutterTime)  );
}

bool IDSuEye::setExposure( double shutterTime )
{
	int nRet = is_Exposure( m_hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &shutterTime, sizeof(shutterTime)  );
	return ( nRet == IS_SUCCESS );
}

void IDSuEye::getPixelClock( unsigned int &pixelClock )
{
	is_PixelClock( m_hCam, IS_PIXELCLOCK_CMD_GET, &pixelClock, sizeof(pixelClock)  );

}

bool IDSuEye::setPixelClock( unsigned int pixelClock )
{
	int nRet = is_PixelClock( m_hCam, IS_PIXELCLOCK_CMD_SET,  &pixelClock, sizeof(pixelClock)  );
	return ( nRet == IS_SUCCESS );
}

void IDSuEye::getFrameRate( double &frameRate )
{
	is_GetFramesPerSecond( m_hCam, &frameRate );
}

bool IDSuEye::setFrameRate( double frameRate )
{
	int nRet = is_SetFrameRate( m_hCam, frameRate, &frameRate );
	return ( nRet == IS_SUCCESS );
}

void IDSuEye::getTimeStamp( char* buffer )
{	
	//UEYEIMAGEINFO ImageInfo;
    if (m_valid_uInfo)
    {
		stringstream oss;
		/*oss << m_uInfo.TimestampSystem.wDay << "." 
			<< m_uInfo.TimestampSystem.wMonth << "."
            << m_uInfo.TimestampSystem.wYear << ", "
			<< m_uInfo.TimestampSystem.wHour << ":"
            << m_uInfo.TimestampSystem.wMinute << ":"
            << m_uInfo.TimestampSystem.wSecond << ":"
            << m_uInfo.TimestampSystem.wMilliseconds;*/

		UINT64 u64TimestampDevice = m_uInfo.u64TimestampDevice;
		oss << u64TimestampDevice;

		strcpy( buffer, oss.str().c_str() );       
    }
}
void IDSuEye::getTimeStamp( unsigned long long* timeStamp )
{
	if( m_valid_uInfo)
		*timeStamp = m_uInfo.u64TimestampDevice;
}

bool IDSuEye::setMirrorUpDown( bool enable )
{
	if ( enable )
		return ( is_SetRopEffect( m_hCam, IS_SET_ROP_MIRROR_UPDOWN, 1, 0 ) == IS_SUCCESS );
	else
		return ( is_SetRopEffect( m_hCam, IS_SET_ROP_MIRROR_UPDOWN, 0, 0 ) == IS_SUCCESS );
}

bool IDSuEye::setMirrorLeftRight( bool enable )
{
	if ( enable )
		return ( is_SetRopEffect( m_hCam, IS_SET_ROP_MIRROR_LEFTRIGHT, 1, 0 ) == IS_SUCCESS );
	else
		return ( is_SetRopEffect( m_hCam, IS_SET_ROP_MIRROR_LEFTRIGHT, 0, 0 ) == IS_SUCCESS );		
}

bool IDSuEye::setAutoGain( bool enable )
{
	double dEnable = ( enable ? 1: 0 );
	return ( is_SetAutoParameter( m_hCam, IS_SET_ENABLE_AUTO_GAIN, &dEnable, 0) == IS_SUCCESS );
}

bool IDSuEye::setAutoShutter( bool enable )
{
	double dEnable = ( enable ? 1: 0 );
	return ( is_SetAutoParameter( m_hCam, IS_SET_ENABLE_AUTO_SHUTTER, &dEnable, 0) == IS_SUCCESS );
}

bool IDSuEye::setHardwareGamma( bool enable )
{
	return ( is_SetHardwareGamma( m_hCam, enable ? IS_SET_HW_GAMMA_ON : IS_SET_HW_GAMMA_OFF ) == IS_SUCCESS );
}

bool IDSuEye::getHardwareGamma()
{
	return ( is_SetHardwareGamma( m_hCam, IS_GET_HW_GAMMA ) == IS_SET_HW_GAMMA_ON );		
}
