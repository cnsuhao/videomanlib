#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include "TISInput.h"

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

TISinput::TISinput()
{
	m_hGrabber = NULL;		
}

TISinput::~TISinput()
{
	if( m_hGrabber )
	{
		IC_CloseVideoCaptureDevice( m_hGrabber );
		IC_ReleaseGrabber( &m_hGrabber );
	}
}

void TISinput::getAvailableDevices(  VMInputIdentification **deviceList, int &numDevices )
{	
	numDevices = 0;
	*deviceList = NULL;

	numDevices = IC_GetDeviceCount();
	if ( numDevices == 0 )
		return;
		
	*deviceList = new VMInputIdentification[numDevices];
	for ( int i=0; i < numDevices; i++)
    {
		VMInputIdentification device;

        char *deviceName = IC_GetDevice( i );
		char *uniqueName = IC_GetUniqueNamefromList ( i );
		copyStringToChar( uniqueName, &device.uniqueName );
		copyStringToChar( "TIS_CAMERA", &device.identifier );
		copyStringToChar( deviceName, &device.friendlyName );
		(*deviceList)[i] = device;
	}
}
         

bool TISinput::initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *aformat )
{
	if ( !device.friendlyName && !device.uniqueName || ( aformat && aformat->showDlg ) )
		m_hGrabber = IC_ShowDeviceSelectionDialog(NULL);
	else
	{
		m_hGrabber = IC_CreateGrabber();
		if( m_hGrabber )
		{
			if(device.uniqueName && IC_OpenDevByUniqueName (m_hGrabber, device.uniqueName ) == IC_ERROR )
				return false;
			else if(device.friendlyName && IC_OpenVideoCaptureDevice (m_hGrabber, device.friendlyName ) == IC_ERROR )
				return false;
		}
		else
			return false;
	}

	char szFormatList[80][40];
	int iFormatCount;    
	iFormatCount = IC_ListVideoFormats( m_hGrabber, (char*)szFormatList,40 );
	if ( iFormatCount <= 0 )
		return false;

	if ( !aformat )
	{	
		//Set the first available format
		bool formatSet = false;
		for( int f = 0; f < min( iFormatCount, 80) && !formatSet; f++ )
		{
			istringstream ss( szFormatList[f] );	
			string colorFormat;
			ss >> colorFormat;
			if ( colorFormat == "Y800" )
			{
				IC_SetFormat( m_hGrabber, Y800 );		
				formatSet = true;
			}
			else if ( colorFormat == "RGB24" )
			{
				IC_SetFormat( m_hGrabber, RGB24 );			
				formatSet = true;
			}
			else if ( colorFormat == "RGB32" )
			{
				IC_SetFormat( m_hGrabber, RGB32 );	
				formatSet = true;
			}
		}
		if ( !formatSet )
			return false;
	}
	else if ( aformat && !aformat->showDlg )
	{
		switch( aformat->getPixelFormatOut() )
		{
		case VM_GREY8:
			{
				IC_SetFormat( m_hGrabber, Y800 );		
				break;
			}
		case VM_RGB24:
			{
				IC_SetFormat( m_hGrabber, RGB24 );		
				break;
			}
		case VM_RGB32:
			{
				IC_SetFormat( m_hGrabber, RGB32 );		
				break;
			}
		default: 
			return false;
		}
	}

	if ( IC_StartLive( m_hGrabber, 0 ) == IC_ERROR )
		return false;

	long lWidth, lHeight;
	int iBitsPerPixel;
	COLORFORMAT colorFormat;
	if ( IC_GetImageDescription ( m_hGrabber, &lWidth, &lHeight, &iBitsPerPixel, &colorFormat ) == IC_ERROR )
		return false;	
	format.width = lWidth;
	format.height = lHeight;
	switch( colorFormat )
	{
		case Y800:
		{
			format.setPixelFormat( VM_GREY8, VM_GREY8 );
			break;
		}
		case RGB24:
		{
			format.setPixelFormat( VM_GREY8, VM_RGB24);
			break;
		}
		case RGB32:
		{
			format.setPixelFormat( VM_GREY8, VM_RGB32 );
			break;
		}
		default:
			return false;
	}

	//Config initial properties
	IC_SetPropertySwitch ( m_hGrabber, "Exposure", "Auto", 1 );	
	 
	if ( aformat )
		*aformat = format;
	
	//Fill identification info
	char *deviceName = IC_GetDeviceName( m_hGrabber );
	int len = 1024;
	char *uniqueName = new char[len];
	if ( IC_GetUniqueName( m_hGrabber, uniqueName, len ) == IC_SUCCESS )			
		copyStringToChar( uniqueName, &identification.uniqueName );		
	delete uniqueName;
	if ( deviceName )
		copyStringToChar( deviceName, &identification.friendlyName );	
	copyStringToChar( "TIS_CAMERA", &identification.identifier );

	//IC_ShowPropertyDialog( m_hGrabber );   

	return true;
}


bool TISinput::linkController( VideoManInputController *acontroller )
{
	if ( acontroller != NULL && acontroller->setInput(this) )
	{
		controller = acontroller;
		return true;
	}
	return false;
}


char *TISinput::getFrame( bool wait)
{
	if ( IC_SnapImage( m_hGrabber, -1 ) == IC_SUCCESS )
	{
		pixelBuffer = (char*)IC_GetImagePtr( m_hGrabber ); 
		return pixelBuffer;
	}		
	return NULL;
}

void TISinput::releaseFrame( )
{
	pixelBuffer = NULL;
}