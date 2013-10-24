#pragma once
#include "VideoManInputController.h"

/** \brief Advanced controller for IDS uEye cameras running with VMIDSuEye
\par Demo Code:
\code
	videoMan.getAvailableDevices( "IDS_uEye_CAMERA", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format );
	...
	IuEyeCameraController *controller = (IuEyeCameraController.h*)videoMan.createController( inputID, "uEye_CAMERA_CONTROLLER" );
	if ( controller )			
	{
		controller->setFrameRate( 50 );				
		controller->setShutterTime( 4 );
	}
	...
	videoMan.deleteController( &controller );	 
\endcode
*/
class IuEyeCameraController :public VideoManPrivate::VideoManInputController
{
public:
	IuEyeCameraController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IuEyeCameraController(void){};

	virtual bool setImageROI( int x, int y, int width, int height ) = 0;

	virtual bool setImageROIpos( int x, int y ) = 0;
	
	virtual void getShutterTime( double &shutterTime ) = 0;

	virtual bool setShutterTime( double shutterTime ) = 0;

	virtual void getPixelClock( unsigned int &pixelClock ) = 0;

	virtual bool setPixelClock( unsigned int pixelClock ) = 0;

	virtual void getFrameRate( double &frameRate ) = 0;

	virtual bool setFrameRate( double frameRate ) = 0;

	virtual void getTimeStamp( char* buffer ) = 0;

	virtual void getTimeStamp( unsigned long long* timeStamp ) = 0;

	virtual bool setMirrorUpDown( bool enable ) = 0;
	
	virtual bool setMirrorLeftRight( bool enable ) = 0;

	virtual bool setAutoGain( bool enable ) = 0;

	virtual bool setAutoShutter( bool enable ) = 0;
	
	virtual bool setHardwareGamma( bool enable ) = 0;
	
	virtual bool getHardwareGamma() = 0;
};
