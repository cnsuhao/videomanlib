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
class IuEyeCameraController : public VideoManPrivate::VideoManInputController
{
public:
	/** \brief Triger modes
	*/
	enum TriggerMode
	{
		Software, /** Software trigger */
		Falling,  /** Fallign edge trigger */
		Rising,   /** Rising edge trigger */		
	};

	/** \brief Strober modes
	*/
	enum StrobeMode
	{
		ConstantLow,  /** Constant low output */
		ConstantHigh, /** Constant high output*/
		LowActive,    /** Low active output */
		HighActive,   /** High active output */
	};
	

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

	virtual bool setExternalTrigger( bool enable, TriggerMode mode ) = 0;

	virtual bool setStrobeOutput( bool enable, IuEyeCameraController::StrobeMode mode ) = 0;

	virtual void forceTrigger() = 0;


};
