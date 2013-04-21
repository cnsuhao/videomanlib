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

void uEyeCameraController::getShutterTime( float &shutterTime )
{
	assert( uEyeCamera!= NULL && "setShutterTime: Input not linked" );
	uEyeCamera->getExposure( shutterTime );
}

bool uEyeCameraController::setShutterTime( float shutterTime )
{
	assert( uEyeCamera!= NULL && "setShutterTime: Input not linked" );
	return uEyeCamera->setExposure( shutterTime );
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