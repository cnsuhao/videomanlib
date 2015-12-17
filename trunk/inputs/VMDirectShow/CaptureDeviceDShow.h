#ifndef CAPTUREDEVICEDSHOW_H
#define CAPTUREDEVICEDSHOW_H

#include "VideoManInputFormat.h"
#include "VideoInputDShow.h"
#include "crossbar.h"

class CaptureDeviceDShow :
	public VideoManPrivate::VideoInput, VideoInputDShow
{
public:
	CaptureDeviceDShow(void);
	virtual ~CaptureDeviceDShow(void);

	bool initCamera( const char *friendlyName, const char *devicePath, VideoMan::VMInputFormat *aFormat );

	char *getFrame( bool wait = false);

	void releaseFrame();
	
	void showPropertyPage();

	void play();
	
	void pause();
	
	void stop();

	double getTimeStamp();

	static void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices  );	

	VideoManPrivate::VideoManInputController *getController();

	bool linkController( VideoManPrivate::VideoManInputController *controller );

	IBaseFilter *getVideoSourceFilter();

	std::string getFriendlyName();	

	int getInputChannelsCount();

	bool setInputChannel( int channel );

	bool supportVideoControl();
	bool getMode( long &mode );

	bool supportImageProperties();
	void setImageProperty( VideoProcAmpProperty imageProp, const long &value, const bool &flagAuto );
	bool getImageProperty( VideoProcAmpProperty imageProp, long &value, bool &flagAuto );
	bool getImagePropertyRange( VideoProcAmpProperty imageProp, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto );

	bool supportCameraControl();
	void setCameraControl( CameraControlProperty camControl, const long &value, const bool &flagAuto );
	bool getCameraControl( CameraControlProperty camControl, long &value, bool &flagAuto );
	bool getCameraControlRange( CameraControlProperty camControl, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto );

	bool supportFrameCallback();
	void setFrameCallback( VideoInput::getFrameCallback theCallback, void *data );
	
	//Capture Dialogs
	void showCaptureDialogDisplay();
	void showCaptureDialogSource();
	void showCaptureDialogFormat();

	HRESULT WINAPI SampleCB(double SampleTime, IMediaSample *pSample);
	HRESULT WINAPI BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength);

private:

	HRESULT prepareMedia( std::string &friendlyName, std::string &devicePath, VideoMan::VMInputFormat *aFormat );		
	BOOL CreateCrossbar();
	HANDLE hFPropThread;

	IAMVideoProcAmp *videoProperties;
	IAMVideoControl *videoControl;
	IAMCameraControl *cameraControl;
	IAMVfwCaptureDialogs *captureDlgs;
	CCrossbar *crossbar;
	IPin *outPinVideoSource;
	HANDLE ghSemaphore;
	HANDLE ghMutex;

	VideoManPrivate::VideoManInputController *controller;
	double lastTimeStamp;
};

#endif