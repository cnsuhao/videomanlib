#include <assert.h>
#ifdef WIN32
	#include <windows.h>
#endif
#include <iostream>
#include <sstream>
#include "IDSFalconFrameGrabber.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace VideoMan;

/*
Code based on the IdsDemo sample program included in the FALCON SDK
*/

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

void freeChar( char **src )
{
	if ( *src )
	{delete *src;
		*src = NULL;
	}
}

IDSFalconFrameGrabber::IDSFalconFrameGrabber(void)
{
	m_info.hThread = NULL;
	m_pcImageMem.resize( 20 );
	m_nImageMemID.resize( m_pcImageMem.size() );
	m_nSeqNumId.resize( m_pcImageMem.size() );
	m_lastSeqIndex = 0;
	m_lastNum = -1;
	m_numRepeatLastNum = 0;
	pixelBuffer = NULL;
}

IDSFalconFrameGrabber::~IDSFalconFrameGrabber(void)
{
    // wait till thread has ended
	#ifdef WIN32
	if ( m_info.hThread )
	{
		m_info.bKill = true;
		DWORD dwRet;		
		time_t seconds;
		time_t start = time(NULL);
		do
		{
			if ( !GetExitCodeThread( m_info.hThread, &dwRet ) )
				dwRet = 0;
			seconds = time (NULL);
		} while ( dwRet == STILL_ACTIVE && seconds - start < 5 );		
		CloseHandle( m_info.hThread );
		m_info.hThread = NULL;
	}
	#endif

	if (m_hBoard != NULL)
	{
		is_ClearSequence( m_hBoard );
		for ( int i = 0; i < (int)m_pcImageMem.size(); ++i )
		{
			// free image mem
			is_FreeImageMem( m_hBoard, m_pcImageMem[i], m_nImageMemID[i] );
			m_pcImageMem[i] = NULL;
		}
		// close board
		int ret = is_ExitBoard (m_hBoard);
	}
	m_hBoard = NULL;
}

int IDSFalconFrameGrabber::colorModeFromPixelFormat( VMPixelFormat pixelFormat )
{
	switch (pixelFormat)
	{
	case GREY8:
		return IS_SET_CM_Y8;
		break;
	/*case RGB15:
		return IS_SET_CM_RGB15;
		break;*/
/*	case RGB16:
		return IS_SET_CM_RGB16;
		break;*/
	case RGB24:
	case BGR24:
		return IS_SET_CM_RGB24;
		break;
	case RGB32:
	case BGR32:
		return IS_SET_CM_RGB32;
		break;
	default:
		return IS_SET_CM_RGB24;
		break;
	}
}

VMPixelFormat IDSFalconFrameGrabber::pixelFormatFromColorMode( int colorMode )
{
	switch (colorMode)
	{
	case IS_SET_CM_Y8:
		return GREY8;
	//case IS_SET_CM_RGB16:
	//	return GREY16;
	//	break;
	case IS_SET_CM_RGB24:
		return BGR24;
	case IS_SET_CM_RGB16:
		return YUV422;
	//case IS_SET_CM_RGB15:
	//	return RGB15;
	case IS_SET_CM_RGB32:
		return BGR32;
	default:
		return UNKNOWN;
	}
}

bool IDSFalconFrameGrabber::initBoard( const unsigned long &hids, VMInputFormat *aFormat )
{
	is_SetErrorReport( NULL, IS_DISABLE_ERR_REP );
	m_hBoard = (HIDS)hids;
	time_t seconds;
	time_t start = time(NULL);
	bool init = false;
	do
	{
		if ( is_InitBoard( &m_hBoard, NULL ) == IS_SUCCESS)
		{
			// get board type (Falcon or Eagle)
			BOARDINFO info;		
			if ( is_GetBoardInfo( m_hBoard, &info ) == IS_SUCCESS )
			{
				ostringstream ss;
				ss << info.ID << " " << (int)m_hBoard;
				copyStringToChar( ss.str(), &identification.friendlyName );												
				ostringstream ss2;
				ss2 << (int)m_hBoard;
				copyStringToChar( ss2.str(), &identification.uniqueName );
				init = true;
			}
			else
				is_ExitBoard (m_hBoard);
		}
		seconds = time (NULL);

		if ( !init )
			#ifdef WIN32
				Sleep( 100 );
			#endif
			#ifdef linux
				sleep( 100 );
			#endif

	} while ( !init && seconds - start < 20 );		
	if ( !init )
		return false;
	
	//Sleep( 2000 );
	int ret = is_SetVideoInput( m_hBoard, IS_SET_VIDEO_IN_1 );

	//Sleep( 2000 );

	int c = is_SetVideoMode (m_hBoard, IS_SET_VM_PAL ); //IS_SET_VM_NTSC 
	c = is_SetVideoMode (m_hBoard, IS_GET_VIDEO_MODE );
	if ( c == IS_NO_SUCCESS )
		return false;

	if ( aFormat != NULL )
	{
		colorMode = colorModeFromPixelFormat( aFormat->getPixelFormatIn() );
		VMPixelFormat pixelFormat = pixelFormatFromColorMode( colorMode );
		format.setPixelFormat( pixelFormat, pixelFormat );
	}
	else
	{
		colorMode = is_SetColorMode( m_hBoard, IS_GET_COLOR_MODE );
		VMPixelFormat pixelFormat = pixelFormatFromColorMode( colorMode );
		if ( pixelFormat == UNKNOWN )
		{
			is_ExitBoard (m_hBoard);
			return false;
		}
		format.setPixelFormat( pixelFormat, pixelFormat );
	}

	if ( colorMode == -1 || is_SetColorMode( m_hBoard, colorMode ) != IS_SUCCESS )
	{
        assert( false && "IDSFalconFrameGrabber::initBoard: Can't set color mode" );
		return false;
	}

	//is_StopLiveVideo (m_hBoard, IS_WAIT);

	is_SetDisplayMode( m_hBoard, IS_SET_DM_DIB );
	
	//is_SetCaptureMode( m_hBoard, IS_SET_CM_NONINTERLACED | IS_SET_CM_FRAME );//IS_SET_CM_ODD | IS_SET_CM_NONINTERLACED
	is_SetCaptureMode( m_hBoard, IS_SET_CM_FRAME );
	//is_SetDecimationMode( m_hBoard, IS_DECIMATION_DISTRIBUTED, 25 );
	

	

	//int pitch;
	//is_GetImageMemPitch( m_hBoard, &pitch );


//	is_SetScaler     (m_hBoard, IS_SET_SCALER_ON, (float) 0);
//	is_SetImageSize  (m_hBoard, 640, 240);

	format.width  = is_SetImageSize( m_hBoard, IS_GET_IMAGE_SIZE_X, 0 );
	format.height = is_SetImageSize( m_hBoard, IS_GET_IMAGE_SIZE_Y, 0 );	
	//is_SetImageSize( m_hBoard, format.width, format.height );

	// calculate size of image memory to allocate	
	if ( aFormat )
	{
		aFormat->width = format.width;
		aFormat->height = format.height;
	}
	long m_lMemSize = (long) format.width * (long) format.height * (long) format.depth / 8;

	//is_ClearSequence( m_hBoard );

	// allocate image memory
	for ( int i = 0; i < (int)m_pcImageMem.size(); ++i )
	{
		ret = is_AllocImageMem( m_hBoard, format.width, format.height, format.depth*format.nChannels, &m_pcImageMem[i], &m_nImageMemID[i] );
		if ( ret != IS_SUCCESS )
		{
			int error;
			char *errorString;
			is_GetError( m_hBoard, &error, &errorString );
			std::string errorS = errorString;
			cout << errorS << endl;
			assert( false && "IDSFalconFrameGrabber::initBoard: Can't allocate image memory" );
			
			return false;
		}

		ret = is_AddToSequence(	m_hBoard, m_pcImageMem[i], m_nImageMemID[i] );
		if( ret != IS_SUCCESS )
		{
			int error;
			char *errorString;
			is_GetError( m_hBoard, &error, &errorString );
			std::string errorS = errorString;
			cout << errorS << endl;
			is_FreeImageMem( m_hBoard, m_pcImageMem[i], m_nImageMemID[i] );
			
			return false;
		}
		m_nSeqNumId[i] = i + 1;
		is_SetRopEffect( m_hBoard, IS_SET_ROP_MIRROR_UPDOWN, 1, 0 );
	}


	// set this image memory active
	/*ret = is_SetImageMem (m_hBoard, m_pcImageMem, m_nImageMemID);
	if (ret != IS_SUCCESS)
	{
		assert( false && "IDSFalconFrameGrabber::initBoard: Can't set image memory" );
		return false;
	}

	// set the image size
	*/

//	ret = is_SetVideoInput( m_hBoard, IS_GET_VIDEO_IN );

	is_CaptureVideo( m_hBoard, IS_FORCE_VIDEO_START );//IS_DONT_WAIT
	//is_FreezeVideo ( m_hBoard, IS_DONT_WAIT);

	//is_SetBrightness( m_hBoard, 100 );

	// set flag that the is_SaveImage / is_LoadImage stores the path if the
	// file save / file open dialog box is used
	//is_BoardStatus( m_hBoard, IS_STORE_FILE_PATH, 1 );

	//BOOL m_boLive = is_CaptureVideo( m_hBoard, IS_GET_LIVE );
	//BOOL g_boLive = m_boLive;

	if ( aFormat != NULL )
		*aFormat = format;

	copyStringToChar( "IDS_FALCON_FRAME_GRABBER", &identification.identifier );

	pixelBuffer = NULL;

	return true;
}

char *IDSFalconFrameGrabber::getFrame( bool wait)
{
	//is_FreezeVideo( m_hBoard, IS_WAIT );
	if ( pixelBuffer == NULL )
	{
		int num;
		char *pcMem;
		char *pcMemLast = NULL;
		int ret = is_GetActSeqBuf( m_hBoard, &num, &pcMem, &pcMemLast );		
		//if ( ret == IS_NO_SUCCESS || num == 0 || num == m_lastNum )
		//	return NULL;
		m_lastNum = num;
		for( m_lastSeqIndex = 0; m_lastSeqIndex < (int)m_pcImageMem.size(); ++m_lastSeqIndex )
		{
			if( pcMemLast == m_pcImageMem[m_lastSeqIndex] )
			{
				pcMemLast = m_pcImageMem[m_lastSeqIndex];
				break;			
			}
		}
		//m_lastSeqIndex = num - 1;
		//m_lastSeqIndex = ( m_lastSeqIndex + 1 ) %2;
		if ( pcMemLast!= NULL )
		{
			ret = is_LockSeqBuf( m_hBoard, m_nSeqNumId[m_lastSeqIndex], m_pcImageMem[m_lastSeqIndex] );
			if( ret == IS_SUCCESS )
			{
				//cout << m_nSeqNumId[m_lastSeqIndex] << " " << num << endl;
				pixelBuffer = m_pcImageMem[m_lastSeqIndex];
				return pixelBuffer;
			}
		}
	}
	return NULL;
}

void IDSFalconFrameGrabber::releaseFrame()
{
	if ( pixelBuffer != NULL )
	{
		is_UnlockSeqBuf( m_hBoard, m_nImageMemID[m_lastSeqIndex], m_pcImageMem[m_lastSeqIndex] );		
		pixelBuffer = NULL;
	}
}

void IDSFalconFrameGrabber::play()
{

}

void IDSFalconFrameGrabber::pause()
{

}

void IDSFalconFrameGrabber::stop()
{

}

void IDSFalconFrameGrabber::showPropertyPage()
{
}

void IDSFalconFrameGrabber::getAvailableDevices( VMInputIdentification **deviceList, int &numDevices  )
{
	int number;
	is_GetNumberOfBoards( &number );
	std::vector<VMInputIdentification> devices;
	std::vector<HFALC> hfalcs;
	for ( int n = 0; n < number; ++n )
	{
		BOARDINFO info;		
		HFALC falc = 0;//(HFALC)( n + 1 );		
		if ( is_InitBoard( &falc, NULL ) != IS_SUCCESS )
			continue;
		if ( is_GetBoardInfo( falc, &info ) == IS_SUCCESS )
		{
			//int m_nBoardType = is_GetBoardType( (HIDS)( n + 1 ) );
			VMInputIdentification device;
			ostringstream ss;
			ss << info.ID << " " << (int)falc;			
			copyStringToChar( ss.str(), &device.friendlyName );			
			ostringstream ss2;
			if ( number == 1 )
				ss2 << 0;
			else
				ss2 << falc;					
			copyStringToChar( ss2.str(), &device.uniqueName );
			devices.push_back( device );
			hfalcs.push_back( falc );
		}
	}
	for ( int n = 0; n < (int)hfalcs.size(); ++n )
		is_ExitBoard( hfalcs[n] );
	*deviceList = new VMInputIdentification[devices.size()];
	numDevices = devices.size();
	for ( int n = 0; n < (int)devices.size(); ++n )
	{
		copyStringToChar( "IDS_FALCON_FRAME_GRABBER", &(*deviceList)[n].identifier );
		copyStringToChar( devices[n].friendlyName, &(*deviceList)[n].friendlyName );
		freeChar( &devices[n].friendlyName );
		copyStringToChar( devices[n].uniqueName, &(*deviceList)[n].uniqueName );
		freeChar( &devices[n].uniqueName );
	}
}

#ifdef linux
void *SwitchThread(void*)
{
}
#endif

#ifdef WIN32
ULONG __stdcall SwitchThread(LPVOID pv)
{
	IDSFalconFrameGrabber::IDS_SWITCH_THREAD *info = (IDSFalconFrameGrabber::IDS_SWITCH_THREAD*)pv;
	int ret = is_InitEvent( info->hf, info->hEvent, IS_SET_EVENT_SEQ );//IS_SET_EVENT_SEQ 
	ret = is_EnableEvent( info->hf, IS_SET_EVENT_SEQ );
	is_CaptureVideo( info->hf, IS_FORCE_VIDEO_START );//IS_DONT_WAIT
	int lastNum = -1;
	int numRepeatLastNum = 0;
	//char *pbuf = new char[800*800*4*8];
	while( !info->bKill )
	{
		if ( *info->pixelBuffer )
			continue;
		DWORD lReturn = WaitForSingleObject( info->hEvent, 100 );
		if ( lReturn != WAIT_OBJECT_0 )
		{
			is_StopLiveVideo( info->hf, IS_WAIT );
			is_CaptureVideo( info->hf, IS_FORCE_VIDEO_START );
			continue;
		}
		/*DWORD lReturn = WaitForMultipleObjects( 1, &info->hEvent, FALSE ,INFINITE );	
		int nEvIndex = lReturn - WAIT_OBJECT_0;
		if ( nEvIndex != WAIT_OBJECT_0 )
		{
			//is_StopLiveVideo( info->hf, IS_WAIT );
			//is_CaptureVideo( info->hf, IS_FORCE_VIDEO_START );
			continue;
		}*/

		
		int num;
		char *pcMem, *pcMemLast;
		is_GetActSeqBuf( info->hf, &num, &pcMem, &pcMemLast );
		int i;
		for( i = 0; i < (int)info->imageMem.size(); ++i )
		{
			if( pcMemLast == info->imageMem[i] )
				break;
		}
		//cout << num << endl;
		if ( lastNum == i )
			numRepeatLastNum++;
		else
			numRepeatLastNum = 0;
		lastNum = i;
		if ( numRepeatLastNum < 10 )
		{
		//is_GetActSeqBuf( info->hf, &num, &pcMem, &pcMemLast );
		//cout << "o "  << num << endl;
			//Sleep( 2 );
			
			is_LockSeqBuf( info->hf, info->imageMemID[i], info->imageMem[i] );
			//memcpy( pbuf, info->imageMem[i], info->nXSize * info->nYSize* 3 );
			*info->pixelBuffer = info->imageMem[i];
			*info->lastSeqIndex = i;
			(*info->callback)( info->imageMem[i], info->inputID, 0, info->frameCallbackData );
			//is_UnlockSeqBuf( info->hf, info->imageMemID[i], info->imageMem[i] );			
		}
	}
	is_DisableEvent( info->hf, IS_SET_EVENT_SEQ );
	is_ExitEvent( info->hf, IS_SET_EVENT_SEQ );
	ExitThread( 0 );
	return 0;
}
#endif

void IDSFalconFrameGrabber::setFrameCallback( getFrameCallback theCallback, void *data )
{
	m_info.hf = m_hBoard;
	m_info.callback = theCallback;
	m_info.imageMem = m_pcImageMem;
	m_info.imageMemID = m_nImageMemID;
	m_info.nXSize = format.width;
	m_info.nYSize = format.height;
	m_info.bKill = false;
	m_info.inputID = this->inputID;
	m_info.lastSeqIndex = &m_lastSeqIndex;
	m_info.pixelBuffer = &pixelBuffer;
	this->callback = theCallback;
	this->frameCallbackData = data;
	m_info.frameCallbackData = this->frameCallbackData;

	#ifdef linux
		int ret = pthread_create( &m_info.hThread, NULL, SwitchThread, (void*)&m_info );
		if ( ret )
		{
			m_info.hEvent = PTHREAD_COND_INITIALIZER;
			cerr << "IDSFalconFrameGrabber::setFrameCallback: Error creating thread" << endl;
		}
	#endif
	#ifdef WIN32
		HANDLE m_hEv = CreateEvent( NULL, FALSE, FALSE, NULL );
		if( m_hEv != NULL)
		{
			
			//HANDLE  m_hThreadEvent;
			//DWORD   m_dwThreadIDEvent;			
			//m_hThreadEvent = (HANDLE)_beginthreadex( NULL, 0, threadProcEvent, (void*)this, 0, (UINT*)&m_dwThreadIDEvent);
			m_info.hEvent = m_hEv;
			DWORD  lThreadID = 0;
			m_info.hThread = CreateThread (NULL,                   // no security
                                       0,                      // default stack size
                                       SwitchThread,           // address of thread function
                                       (void*)&m_info,   // pointer to info structure
                                       0,                      // start immediately
                                       &lThreadID);

		}
	#endif
}
