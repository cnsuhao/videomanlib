#include "uEyeCameraController.h"
#include <assert.h>
#include <string>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

uEyeCameraController::uEyeCameraController( const char *_identifier ) : IuEyeCameraController( _identifier )
{
	uEyeCamera = NULL;
}

uEyeCameraController::~uEyeCameraController(void)
{
	
}

bool uEyeCameraController::setInput( VideoInput *input )
{
	string identifier = input->getIdentification().identifier;
	if ( identifier == "IDS_uEye_CAMERA" )
	{
		uEyeCamera = (IDSuEye*)input;
		if ( uEyeCamera )			
			return true;
	}
	return false;
}

bool uEyeCameraController::setImageROI( int x, int y, int width, int height )
{
	assert( uEyeCamera!= NULL && "setImageROI: Input not linked" );
	return uEyeCamera->setImageROI( x, y, width, height );
}

bool uEyeCameraController::setImageROIpos( int x, int y )
{
	assert( uEyeCamera!= NULL && "setImageROI: Input not linked" );
	return uEyeCamera->setImageROIpos( x, y );
}

void uEyeCameraController::getShutterTime( double &shutterTime )
{
	assert( uEyeCamera!= NULL && "setShutterTime: Input not linked" );
	uEyeCamera->getExposure( shutterTime );
}

bool uEyeCameraController::setShutterTime( double shutterTime )
{
	assert( uEyeCamera!= NULL && "setShutterTime: Input not linked" );
	return uEyeCamera->setExposure( shutterTime );
}

void uEyeCameraController::getPixelClock( unsigned int &pixelClock )
{
	assert( uEyeCamera!= NULL && "getPixelClock: Input not linked" );
	uEyeCamera->getPixelClock( pixelClock );
}

bool uEyeCameraController::setPixelClock( unsigned int pixelClock )
{
	assert( uEyeCamera!= NULL && "setPixelClock: Input not linked" );
	return uEyeCamera->setPixelClock( pixelClock );	
}

void uEyeCameraController::getTimeStamp( char* buffer )
{
	assert( uEyeCamera!=NULL && "getTimeStamp: Input not linked" );
	uEyeCamera->getTimeStamp( buffer );
}

void uEyeCameraController::getTimeStamp( unsigned long long* timeStamp )
{
	assert( uEyeCamera!=NULL && "getTimeStamp: Input not linked" );
	uEyeCamera->getTimeStamp( timeStamp );
}

void uEyeCameraController::getFrameRate( double &frameRate )
{
	assert( uEyeCamera!= NULL && "getFrameRate: Input not linked" );
	uEyeCamera->getFrameRate( frameRate );
}

bool uEyeCameraController::setFrameRate( double frameRate )
{
	assert( uEyeCamera!= NULL && "setFrameRate: Input not linked" );
	return uEyeCamera->setFrameRate( frameRate );
}

bool uEyeCameraController::setMirrorUpDown( bool enable )
{
	assert( uEyeCamera!= NULL && "setMirrorUpDown: Input not linked" );
	return uEyeCamera->setMirrorUpDown( enable );
}

bool uEyeCameraController::setMirrorLeftRight( bool enable )
{
	assert( uEyeCamera!= NULL && "setMirrorLeftRight: Input not linked" );
	return uEyeCamera->setMirrorLeftRight( enable );
}

bool uEyeCameraController::setAutoShutter( bool enable )
{
	assert( uEyeCamera!= NULL && "setAutoShutter: Input not linked" );
	return uEyeCamera->setAutoShutter( enable );
}

bool uEyeCameraController::setAutoGain( bool enable )
{
	assert( uEyeCamera!= NULL && "setAutoGain: Input not linked" );
	return uEyeCamera->setAutoGain( enable );
}


bool uEyeCameraController::setHardwareGamma( bool enable )
{
	assert( uEyeCamera!= NULL && "setHardwareGamma: Input not linked" );
	return uEyeCamera->setHardwareGamma( enable );

}
	
bool uEyeCameraController::getHardwareGamma()
{
	assert( uEyeCamera!= NULL && "getHardwareGamma: Input not linked" );
	return uEyeCamera->getHardwareGamma();
}