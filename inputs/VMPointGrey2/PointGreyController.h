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

	bool setFrameRate( float frameRate );

	bool moveImageROI( int x, int y );

	bool setImageROI( int x, int y, int width, int height );

	bool getROIUnits( int &Hmax, int &Vmax, int &imageHStepSize, int &imageVStepSize, int &offsetHStepSize, int &offsetVStepSize );

	bool resetImageROI();
	bool getBayerFormat(size_t &btf);

	bool setTrigger( bool triggerOn, int source, int mode );

	bool fireSoftwareTrigger( bool broadcast );

	void convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height );

	bool setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source );

private:
	std::vector< PGRCamera* > cameraList;
//	std::vector<FlyCaptureContext> contexts;
};