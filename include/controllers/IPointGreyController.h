#pragma once

#include "VideoManInputController.h"

/** \brief Advanced controller for PointGrey cameras running with VMPointGrey2
\par Demo Code:
\code
	videoMan.getAvailableDevices( "PGR_CAMERA2", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format );
	...
	IPointGreyController *controller = (IPointGreyController.h*)videoMan.createController( inputID, "POINTGREY_CONTROLLER2" );
	if ( controller )			
	{
		controller->setGainControl( true );				
		controller->setShutterTime( 4 );
	}
	...
	videoMan.deleteController( &controller );	 
\endcode
*/
class IPointGreyController : public VideoManPrivate::VideoManInputController
{
public:
	IPointGreyController( const char *identifier ) : VideoManPrivate::VideoManInputController(identifier){}
	virtual ~IPointGreyController(){}	
	virtual bool setRegister( unsigned int reg, unsigned int value ) = 0;
	virtual void printCameraInfo() = 0;
	
	virtual bool setGainControl(bool autoGain) = 0;
	virtual bool getGainControl() = 0;	
	virtual bool setExposureControl( bool autoExp ) = 0;
	virtual bool setSharpnessControlValue(float value) = 0;
	virtual bool setSharpnessControl( bool autpoSharp ) = 0;
	virtual bool setShutterTime( float shutterTime ) = 0;
	virtual bool setFrameRate( float frameRate ) = 0;	
	virtual bool setTrigger( bool triggerOn, int source, int mode ) = 0;
	virtual bool fireSoftwareTrigger( bool broadcast ) = 0;
	virtual bool setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source ) = 0;
	
	virtual bool moveImageROI( int x, int y ) = 0;
	virtual bool setImageROI( int x, int y, int width, int height ) = 0;
	virtual bool getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit ) = 0;
	virtual bool resetImageROI() = 0;
	virtual void convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height ){};
	virtual bool getBayerFormat(size_t &btf)=0;
};
