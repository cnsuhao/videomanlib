#ifndef UEYECAMERACONTROLLER_H
#define UEYECAMERACONTROLLER_H

#include "controllers/IuEyeCameraController.h"
#include "IDSuEye.h"

class uEyeCameraController :
	public  IuEyeCameraController
{
public:
	uEyeCameraController( const char *_identifier );
	virtual ~uEyeCameraController(void);

	bool setInput( VideoManPrivate::VideoInput *input );

	bool setImageROI( int x, int y, int width, int height );

	bool setImageROIpos( int x, int y );

	void getShutterTime( double &shutterTime );

	bool setShutterTime( double shutterTime );
	
	void getPixelClock( unsigned int &pixelClock );

	bool setPixelClock( unsigned int pixelClock );

	void getFrameRate( double &frameRate );

	bool setFrameRate( double frameRate );

	bool setMirrorUpDown( bool enable );
	
	bool setMirrorLeftRight( bool enable );

	bool setAutoGain( bool enable );

	bool setAutoShutter( bool enable );
			
protected:		
	IDSuEye *uEyeCamera;
};
#endif