#pragma once
//#include <windows.h>
#include <vector>
#include "VideoManInputController.h"
#include "controllers/IPointGreyController.h"
#include "PGRCamera.h"

class PointGreyController :
	public IPointGreyController
{
public:
	PointGreyController(const char *identifier);
	virtual ~PointGreyController(void);
	
	void startRecording();
	void stopRecording();
	int  getNumberOfCameras();	

	bool setRegister( unsigned int reg, unsigned int value );

	bool setRegisterBroadcast( unsigned long reg, unsigned long value );

	void SynchronizeCameras();

	void UnlockBuffers();

	void printCameraInfo();

	bool setInput( VideoManPrivate::VideoInput *input );

	bool setGainControl(bool autoGain);
	
	bool getGainControl();	

	bool setExposureControl( bool autoExp );

	bool setSharpnessControlValue(float value);
	
	bool setSharpnessControl( bool autpoSharp );

	bool setShutterTime( float shutterTime );

	bool setImageROI( int x, int y, int width, int height );

	bool moveImageROI( int x, int y );

	bool getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit );

	bool resetImageROI();

	bool setTrigger( bool triggerOn, int source, int mode );

	bool setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source );
	
	bool fireSoftwareTrigger( bool broadcast );

	void convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height );

	bool getBayerFormat(size_t &btf);

	bool setFrameRate( float frameRate );	

private:
	std::vector< PGRCamera* > cameraList;

//	std::vector<FlyCaptureContext> contexts;

	
};
