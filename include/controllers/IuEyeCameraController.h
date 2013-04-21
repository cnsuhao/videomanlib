#pragma once
#include "VideoManInputController.h"

/** \brief If you have an IDS uEye camera you can access more features with this interface trough uEye SDK
\par Demo Code:
\code
	IuEyeCameraController *controller = (IuEyeCameraController*)videoMan.createController( "DSHOW_uEYE_CAMERA_CONTROLLER" );
	if ( controller )			
	{		
		...
		videoMan.deleteController( &controller ); //not required
	}
\endcode
*/
class IuEyeCameraController :public VideoManPrivate::VideoManInputController
{
public:
	IuEyeCameraController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IuEyeCameraController(void){};

	virtual bool setImageROI( int x, int y, int width, int height ) = 0;

	virtual bool setImageROIpos( int x, int y ) = 0;
	
	virtual void getShutterTime( float &shutterTime ) = 0;

	virtual bool setShutterTime( float shutterTime ) = 0;

	virtual void getFrameRate( double &frameRate ) = 0;

	virtual bool setFrameRate( double frameRate ) = 0;
};
