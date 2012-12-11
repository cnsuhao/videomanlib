#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "IDSuEye.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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
	m_pcImageMemory[0] = NULL;
	m_pcImageMemory[1] = NULL;
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
		if( m_pcImageMemory[0] != NULL )
  			is_FreeImageMem( m_hCam, m_pcImageMemory[0], m_lMemoryId[0] );
		if( m_pcImageMemory[1] != NULL )
  			is_FreeImageMem( m_hCam, m_pcImageMemory[1], m_lMemoryId[1] );
        
		m_pcImageMemory[0] = NULL;
		m_pcImageMemory[1] = NULL;
		
		// Close camera
		is_ExitCamera( m_hCam );
        m_hCam = NULL;
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
		m_hCam = (HIDS)cameraId;						// open next camera
	else 
		m_hCam = (HIDS)0;						// open next camera
	int nRet = is_InitCamera( &m_hCam, NULL );		// init camera - no window handle required
	if (nRet == IS_SUCCESS)
	{
		// retrieve original image size
		is_GetSensorInfo(m_hCam, &m_sInfo);
		m_nSizeX = m_sInfo.nMaxWidth;
		m_nSizeY = m_sInfo.nMaxHeight;		
		copyStringToChar( m_sInfo.strSensorName, &identification.friendlyName );				
		ostringstream ss;
		ss << m_hCam;
		copyStringToChar( ss.str(), &identification.uniqueName );		
		copyStringToChar( "IDS_uEye_CAMERA", &identification.identifier );
	}
	else
	{
		cerr << "No uEye camera could be opened !" << endl;
	    return false;
    }

	nRet = IS_NO_SUCCESS;    
    if (m_hCam == NULL)
        return false;

	// Set display mode to DIB
    nRet = is_SetDisplayMode(m_hCam, IS_SET_DM_DIB);	
	if ( !aformat || aformat->getPixelFormatIn() == UNKNOWN )
	{
		if (m_sInfo.nColorMode == IS_COLORMODE_BAYER)
		{
			// setup the color depth to the current windows setting
			//is_GetColorDepth(m_hCam, &m_nBitsPerPixel, &m_nColorMode);
			m_nColorMode = IS_CM_RGB8_PACKED;
			m_nBitsPerPixel = 24;
			format.SetFormat( m_nSizeX, m_nSizeY, 30.0, RGB24, RGB24 );

		}
		else
		{
			m_nColorMode = IS_CM_MONO8;
			m_nBitsPerPixel = 8;
			format.SetFormat( m_nSizeX, m_nSizeY, 30.0, GREY8, GREY8 );
		}
	}
	else
	{
		//Set the desired format
		format.setPixelFormat( aformat->getPixelFormatIn(),aformat->getPixelFormatIn() );
		switch( format.getPixelFormatIn() )
		{
			// YUV422, YUV411, IYUV, , UNKNOWN
			case RGB24:
			case BGR24:
				{
					m_nColorMode = IS_CM_RGB8_PACKED;
					m_nBitsPerPixel = 24;
					break;
				}
			case BGR32:
			case RGB32:
				{
					m_nColorMode = IS_CM_RGBA8_PACKED;
					 m_nBitsPerPixel = 32;
					break;
				}
			case GREY8:
				{
					m_nColorMode = IS_CM_MONO8;
					 m_nBitsPerPixel = 8;
					break;
				}
			case GREY16:
				{
					m_nColorMode = IS_CM_MONO16;
					 m_nBitsPerPixel = 16;
					break;
				}
			case RAW8:
				{
					m_nColorMode = IS_CM_SENSOR_RAW8;
					 m_nBitsPerPixel = 8;
					break;
				}
		}
		m_nSizeX = aformat->width;
		m_nSizeY = aformat->height;
		format.width = m_nSizeX;
		format.height = m_nSizeY;
		format.fps = aformat->fps;

		if ( is_SetFrameRate(m_hCam, format.fps, &format.fps ) != IS_SUCCESS )
		{
			cerr << "Invalid frame rate" << endl;
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
	// set the image size to capture
	/*if ( is_SetImageSize(m_hCam, m_nSizeX, m_nSizeY ) != IS_SUCCESS )
	{
		cerr << "Invalid image size" << endl;
		return false;
	}*/	

	//Get the format	
	/*m_nSizeX = is_SetImageSize(m_hCam, IS_GET_IMAGE_SIZE_X, 0 );
	m_nSizeY = is_SetImageSize(m_hCam, IS_GET_IMAGE_SIZE_Y, 0 );	*/
	IS_RECT rectAOI; 
	nRet = is_AOI(m_hCam, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
	if (nRet == IS_SUCCESS)
	{
	  int x     = rectAOI.s32X;
	  int y     = rectAOI.s32Y;
	  m_nSizeX = rectAOI.s32Width;
	  m_nSizeY = rectAOI.s32Height;
	}
	
	
	format.width = m_nSizeX;
	format.height = m_nSizeY;
	int m_nColorMode = is_SetColorMode(m_hCam, IS_GET_COLOR_MODE );
	switch( m_nColorMode )
	{
		case IS_CM_RGB8_PACKED:		
			{
				format.setPixelFormat( BGR24, BGR24 );
				break;
			}
		case IS_CM_RGBA8_PACKED:		
			{				
				format.setPixelFormat( BGR32, BGR32 );
				break;
			}
		case IS_CM_MONO8:
			{
				format.setPixelFormat( GREY8, GREY8 );
				break;
			}
		case IS_CM_MONO16:
			{
				format.setPixelFormat( GREY16, GREY16 );
				break;
			}
		case IS_CM_SENSOR_RAW8:
			{
				format.setPixelFormat( RAW8, RAW8 );
				break;
			} 
	}
 

	m_nBitsPerPixel = format.depth * format.nChannels;
	 // allocate an image memory.
    if (is_AllocImageMem(m_hCam, m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory[0], &m_lMemoryId[0] ) != IS_SUCCESS)
    {
        cerr << "Memory allocation failed!" << endl;
		return false;
    }
	
	//is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	is_AddToSequence( m_hCam, m_pcImageMemory[0], m_lMemoryId[0] );

	if (is_AllocImageMem(m_hCam, m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory[1], &m_lMemoryId[1] ) != IS_SUCCESS)
    {
        cerr << "Memory allocation failed!" << endl;
		return false;
    }

	//is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	is_AddToSequence( m_hCam, m_pcImageMemory[1], m_lMemoryId[1] );
		
    // Enable Messages
    //is_InitEvent()
	//is_EnableEvent()
	//is_DisableEvent()
	//is_ExitEvent()
	//IS_SET_EVENT_FRAME
	//is_WaitEvent
	/*
	//Activate and initialise FRAME event
	is_EnableEvent (hCam, IS_SET_EVENT_FRAME);
	is_InitEvent (hCam, IS_SET_EVENT_FRAME);
	//Start image capture and wait 1000 ms for event to occur
	is_FreezeVideo (hCam, IS_DONT_WAIT);
	is_WaitEvent (hCam, IS_SET_EVENT_FRAME, 1000);
	*/

    // start live video
    is_CaptureVideo( m_hCam, IS_WAIT );
	//is_FreezeVideo( m_hCam, IS_WAIT );

	is_GetFramesPerSecond( m_hCam, &format.fps );	
	
	if ( aformat )
		*aformat = format;

	/*CAMINFO pInfo;
	if ( is_GetCameraInfo( m_hCam, &pInfo ) == IS_SUCCESS )
	{
		//copyStringToChar( pInfo., &identification.identifier );		
		//identification.serialNumber = pInfo.ID;
	}*/

	

	//is_GetImageHistogram()

    return true;
}

void IDSuEye::releaseFrame( )
{	
	is_UnlockSeqBuf( m_hCam, IS_IGNORE_PARAMETER, pixelBuffer );
	pixelBuffer = NULL;
}

char *IDSuEye::getFrame( bool wait)
{	
	INT nNum;
	char *pcMem;
	is_GetActSeqBuf(m_hCam, &nNum, &pcMem, &pixelBuffer);	
	is_LockSeqBuf( m_hCam, IS_IGNORE_PARAMETER, pixelBuffer );
	return pixelBuffer;
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
		
	// Free the allocated buffer
	if( m_pcImageMemory[0] != NULL )
		is_FreeImageMem( m_hCam, m_pcImageMemory[0], m_lMemoryId[0] );
	if( m_pcImageMemory[1] != NULL )
		is_FreeImageMem( m_hCam, m_pcImageMemory[1], m_lMemoryId[1] );
    
	m_pcImageMemory[0] = NULL;
	m_pcImageMemory[1] = NULL;

	
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
	if (is_AllocImageMem(m_hCam, width, height, m_nBitsPerPixel, &m_pcImageMemory[0], &m_lMemoryId[0] ) != IS_SUCCESS)
    {
        cerr << "Memory allocation failed!" << endl;
		return false;
    }
	
	//is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	is_AddToSequence( m_hCam, m_pcImageMemory[0], m_lMemoryId[0] );

	if (is_AllocImageMem(m_hCam, width, height, m_nBitsPerPixel, &m_pcImageMemory[1], &m_lMemoryId[1] ) != IS_SUCCESS)
    {
        cerr << "Memory allocation failed!" << endl;
		return false;
    }

	//is_SetImageMem( m_hCam, m_pcImageMemory, m_lMemoryId );
	is_AddToSequence( m_hCam, m_pcImageMemory[1], m_lMemoryId[1] );

	if ( is_CaptureVideo( m_hCam, IS_WAIT ) != IS_SUCCESS )
		return false;
	return true;
}

bool IDSuEye::setImageROIpos( int x, int y )
{
	/*if ( is_SetImagePos( m_hCam, x, y ) != IS_SUCCESS )
	{
		cerr << "Invalid image ROI position" << x << " " << y << endl;
		return false;
	}*/
	IS_RECT rectAOI; 
	rectAOI.s32X     = x;
	rectAOI.s32Y     = y;		 	
	if ( is_AOI( m_hCam, IS_AOI_IMAGE_SET_POS, (void*)&rectAOI, sizeof(rectAOI)) != IS_SUCCESS )
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