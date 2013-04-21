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

	void getShutterTime( float &shutterTime );

	bool setShutterTime( float shutterTime );	

	void getFrameRate( double &frameRate );

	bool setFrameRate( double frameRate );
			
protected:		
	IDSuEye *uEyeCamera;
};
#endif